#include "file_search.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

// Define the global FileSearch instance
FileSearch g_fs;

FileSearch::FileSearch()
{
    // Constructor - nothing to initialize
}

FileSearch::~FileSearch()
{
    // Clean up any open zip archives
    for (auto &[path, zipInfo] : m_zipFiles)
    {
        delete zipInfo;
    }
    m_zipFiles.clear();
}

bool FileSearch::addSearchPath(const std::string &path)
{
    if (!fs::exists(path))
    {
        std::cerr << "Search path does not exist: " << path << std::endl;
        return false;
    }

    if (fs::is_directory(path))
    {
        // Add directory to the search list
        SearchPath searchPath;
        searchPath.path = path;
        searchPath.type = PathType::Directory;
        m_searchPaths.push_back(searchPath);
        std::cout << "Added directory to search path: " << path << std::endl;
        return true;
    }
    else if (fs::is_regular_file(path))
    {
        // Check if it's a zip file by extension
        if (path.size() > 4 && path.substr(path.size() - 4) == ".zip")
        {
            // Create and initialize the zip archive
            ZipInfo *zipInfo = new ZipInfo();

            // Try to open the zip file
            if (!mz_zip_reader_init_file(&zipInfo->archive, path.c_str(), 0))
            {
                std::cerr << "Failed to open zip file: " << path << std::endl;
                delete zipInfo;
                return false;
            }

            zipInfo->valid = true;
            m_zipFiles[path] = zipInfo;

            // Add to the ordered search list
            SearchPath searchPath;
            searchPath.path = path;
            searchPath.type = PathType::ZipFile;
            m_searchPaths.push_back(searchPath);

            std::cout << "Added zip file to search path: " << path << std::endl;
            return true;
        }
        else
        {
            std::cerr << "Path is a file but not a zip: " << path << std::endl;
            return false;
        }
    }

    std::cerr << "Path is neither a directory nor a file: " << path
              << std::endl;
    return false;
}

void FileSearch::clearSearchPaths()
{
    m_searchPaths.clear();

    // Clean up and clear zip files
    for (auto &[path, zipInfo] : m_zipFiles)
    {
        delete zipInfo;
    }
    m_zipFiles.clear();
}

bool FileSearch::loadFile(const std::string &filename,
                          std::vector<uint8_t> &buffer)
{
    // Search all paths in the order they were added
    for (const auto &searchPath : m_searchPaths)
    {
        if (searchPath.type == PathType::Directory)
        {
            if (loadFromDirectory(searchPath.path, filename, buffer))
            {
                return true;
            }
        }
        else if (searchPath.type == PathType::ZipFile)
        {
            if (loadFromZip(searchPath.path, filename, buffer))
            {
                return true;
            }
        }
    }

    // File not found in any search path
    return false;
}

bool FileSearch::loadFromDirectory(const std::string &dirPath,
                                   const std::string &filename,
                                   std::vector<uint8_t> &buffer)
{
    fs::path filePath = fs::path(dirPath) / filename;

    // Check if file exists
    if (!fs::exists(filePath))
    {
        return false;
    }

    // Open the file
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Resize buffer and read file
    buffer.resize(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        std::cerr << "Failed to read file: " << filePath << std::endl;
        return false;
    }

    std::cout << "Loaded file from directory: " << filePath << std::endl;
    return true;
}

bool FileSearch::loadFromZip(const std::string &zipPath,
                             const std::string &filename,
                             std::vector<uint8_t> &buffer)
{
    ZipInfo *zipInfo = m_zipFiles[zipPath];
    if (!zipInfo || !zipInfo->valid)
    {
        return false;
    }

    // Find the file inside the zip archive
    int file_index = mz_zip_reader_locate_file(&zipInfo->archive,
                                               filename.c_str(), nullptr, 0);
    if (file_index < 0)
    {
        return false;
    }

    // Get file info
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&zipInfo->archive, file_index, &file_stat))
    {
        std::cerr << "Failed to get file info from zip: " << zipPath << " -> "
                  << filename << std::endl;
        return false;
    }

    // Resize buffer to fit the file
    buffer.resize(file_stat.m_uncomp_size);

    // Extract file
    if (!mz_zip_reader_extract_to_mem(&zipInfo->archive, file_index,
                                      buffer.data(), buffer.size(), 0))
    {
        std::cerr << "Failed to extract file from zip: " << zipPath << " -> "
                  << filename << std::endl;
        return false;
    }

    std::cout << "Loaded file from zip: " << zipPath << " -> " << filename
              << std::endl;
    return true;
}

bool FileSearch::loadFileByCRC(uint32_t crc32, std::vector<uint8_t> &buffer)
{
    // Search through all zip files in search paths
    for (const auto &searchPath : m_searchPaths)
    {
        if (searchPath.type == PathType::ZipFile)
        {
            if (loadFromZipByCRC(searchPath.path, crc32, buffer))
            {
                return true;
            }
        }
    }
    
    return false;
}

bool FileSearch::loadFromZipByCRC(const std::string &zipPath, uint32_t crc32,
                                  std::vector<uint8_t> &buffer)
{
    ZipInfo *zipInfo = m_zipFiles[zipPath];
    if (!zipInfo || !zipInfo->valid)
    {
        return false;
    }

    // Search for file by CRC32
    for (unsigned int fileIndex = 0; fileIndex < zipInfo->archive.m_total_files; fileIndex++)
    {
        mz_zip_archive_file_stat fileStat;
        if (mz_zip_reader_file_stat(&zipInfo->archive, fileIndex, &fileStat))
        {
            if (fileStat.m_crc32 == crc32)
            {
                // Found file with matching CRC
                buffer.resize(fileStat.m_uncomp_size);
                
                if (mz_zip_reader_extract_to_mem(&zipInfo->archive, fileIndex,
                                                 buffer.data(), buffer.size(), 0))
                {
                    return true;
                }
                else
                {
                    buffer.clear();
                    return false;
                }
            }
        }
    }

    return false;
}

std::string FileSearch::findFilePath(const std::string &filename)
{
    // Search through all search paths
    for (const auto &searchPath : m_searchPaths)
    {
        if (searchPath.type == PathType::Directory)
        {
            std::filesystem::path fullPath = std::filesystem::path(searchPath.path) / filename;
            if (std::filesystem::exists(fullPath))
            {
                return fullPath.string();
            }
        }
        else if (searchPath.type == PathType::ZipFile)
        {
            // For ZIP files, return the ZIP path if the file exists inside it
            ZipInfo *zipInfo = m_zipFiles[searchPath.path];
            if (zipInfo && zipInfo->valid)
            {
                int fileIndex = mz_zip_reader_locate_file(&zipInfo->archive, filename.c_str(), nullptr, 0);
                if (fileIndex >= 0)
                {
                    return searchPath.path; // Return the ZIP file path
                }
            }
        }
    }
    
    return ""; // File not found
}

std::vector<FileSearch::SearchPath> FileSearch::saveSearchPaths() const
{
    return m_searchPaths;
}

void FileSearch::restoreSearchPaths(const std::vector<SearchPath> &savedPaths)
{
    // Clear current paths and zip cache
    clearSearchPaths();
    
    // Restore saved paths
    for (const auto &path : savedPaths)
    {
        addSearchPath(path.path);
    }
}