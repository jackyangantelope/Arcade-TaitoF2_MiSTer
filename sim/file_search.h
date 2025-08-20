#ifndef FILE_SEARCH_H
#define FILE_SEARCH_H

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include "third_party/miniz.h"

/**
 * Class for searching and loading files from various paths.
 * Supports searching in directories and inside zip files.
 * Search is performed in the order paths were added.
 */
class FileSearch
{
  public:
    // Enum to track search path type
    enum class PathType
    {
        Directory,
        ZipFile
    };

    // Structure to track search paths in order
    struct SearchPath
    {
        std::string path;
        PathType type;
    };

    // Constructor
    FileSearch();

    // Destructor - clean up cached zip files
    ~FileSearch();

    /**
     * Add a path to the search list
     * @param path Path to a directory or zip file
     * @return true if path exists and was added
     */
    bool addSearchPath(const std::string &path);

    /**
     * Load a file into the provided buffer
     * @param filename Name of the file to locate
     * @param buffer Vector to store the file contents
     * @return true if file was found and loaded
     */
    bool loadFile(const std::string &filename, std::vector<uint8_t> &buffer);
    
    /**
     * Load a file by CRC32 from zip files in search paths
     * @param crc32 CRC32 value to search for
     * @param buffer Vector to store the file contents
     * @return true if file was found and loaded
     */
    bool loadFileByCRC(uint32_t crc32, std::vector<uint8_t> &buffer);
    
    /**
     * Find the full path of a file in search paths
     * @param filename Name of the file to locate
     * @return Full path to the file, or empty string if not found
     */
    std::string findFilePath(const std::string &filename);

    /**
     * Clear all search paths
     */
    void clearSearchPaths();
    
    /**
     * Save current search paths state
     * @return Saved state that can be restored later
     */
    std::vector<SearchPath> saveSearchPaths() const;
    
    /**
     * Restore search paths from saved state
     * @param savedPaths Previously saved search paths
     */
    void restoreSearchPaths(const std::vector<SearchPath> &savedPaths);
    
    /**
     * Get the number of search paths
     * @return Number of active search paths
     */
    size_t getSearchPathCount() const { return m_searchPaths.size(); }

  private:
    // Structure to hold opened zip archive information
    struct ZipInfo
    {
        mz_zip_archive archive;
        bool valid;

        ZipInfo() : valid(false)
        {
            mz_zip_zero_struct(&archive);
        }

        ~ZipInfo()
        {
            if (valid)
            {
                mz_zip_reader_end(&archive);
            }
        }
    };

    // List of search paths in the order they were added
    std::vector<SearchPath> m_searchPaths;

    // Map of zip file paths to their archive objects
    std::unordered_map<std::string, ZipInfo *> m_zipFiles;

    // Try to load file from a directory
    bool loadFromDirectory(const std::string &dirPath,
                           const std::string &filename,
                           std::vector<uint8_t> &buffer);

    // Try to load file from a zip archive
    bool loadFromZip(const std::string &zipPath, const std::string &filename,
                     std::vector<uint8_t> &buffer);
                     
    // Try to load file by CRC from a zip archive
    bool loadFromZipByCRC(const std::string &zipPath, uint32_t crc32,
                          std::vector<uint8_t> &buffer);
};

// Global FileSearch instance that can be used throughout the application
extern FileSearch g_fs;

#endif // FILE_SEARCH_H
