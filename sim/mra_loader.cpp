#include "mra_loader.h"
#include "file_search.h"
#include "third_party/miniz.h"
#include "third_party/pugixml.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

bool MRALoader::load(const std::string& mraPath, std::vector<uint8_t>& romData)
{
    romData.clear();
    m_lastError.clear();
    
    // Load and parse the MRA XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(mraPath.c_str());
    
    if (!result) {
        m_lastError = "Failed to parse MRA file: " + std::string(result.description());
        return false;
    }
    
    // Find the rom element with index="0" (or no index attribute)
    pugi::xml_node romNode;
    for (auto rom : doc.child("misterromdescription").children("rom")) {
        int index = rom.attribute("index").as_int(0);
        if (index == 0) {
            romNode = rom;
            break;
        }
    }
    
    if (!romNode) {
        m_lastError = "No ROM element with index 0 found";
        return false;
    }
    
    // Get the zip file path if specified
    std::string zipPath = romNode.attribute("zip").as_string("");
    if (!zipPath.empty()) {
        // Try to find the zip file in the same directory as the MRA
        std::filesystem::path mraDir = std::filesystem::path(mraPath).parent_path();
        zipPath = (mraDir / zipPath).string();
    }
    
    // Process each part in order
    for (auto child : romNode.children()) {
        std::string nodeName = child.name();
        
        if (nodeName == "part") {
            // Get part attributes
            std::string fileName = child.attribute("name").as_string("");
            std::string crcStr = child.attribute("crc").as_string("");
            int repeat = child.attribute("repeat").as_int(1);
            int offset = child.attribute("offset").as_int(0);
            int length = child.attribute("length").as_int(-1);
            std::string mapStr = child.attribute("map").as_string("");
            
            std::vector<uint8_t> partData;
            
            if (!fileName.empty()) {
                // Parse CRC if provided
                uint32_t crc32Value = 0;
                if (!crcStr.empty()) {
                    try {
                        crc32Value = std::stoul(crcStr, nullptr, 16);
                    } catch (...) {
                        // Invalid CRC, ignore it
                        crc32Value = 0;
                    }
                }
                
                // Load file from zip or filesystem
                if (!loadFileData(zipPath, fileName, partData, crc32Value)) {
                    m_lastError = "Failed to load file: " + fileName;
                    return false;
                }
                
                // Apply offset and length
                if (offset > 0 || length >= 0) {
                    if (offset >= partData.size()) {
                        m_lastError = "Offset exceeds file size for: " + fileName;
                        return false;
                    }
                    
                    size_t endPos = partData.size();
                    if (length >= 0) {
                        endPos = std::min(endPos, (size_t)(offset + length));
                    }
                    
                    partData = std::vector<uint8_t>(partData.begin() + offset, 
                                                   partData.begin() + endPos);
                }
                
                // Apply map if specified
                if (!mapStr.empty()) {
                    if (!applyMap(partData, mapStr)) {
                        m_lastError = "Failed to apply map: " + mapStr;
                        return false;
                    }
                }
            } else {
                // Parse hex data from text content
                std::string hexData = child.child_value();
                if (!hexData.empty()) {
                    if (!parseHexString(hexData, partData)) {
                        m_lastError = "Failed to parse hex data";
                        return false;
                    }
                }
            }
            
            // Verify CRC if specified
            if (!crcStr.empty() && !partData.empty()) {
                uint32_t expectedCrc = std::stoul(crcStr, nullptr, 16);
                uint32_t actualCrc = calculateCRC32(partData);
                if (expectedCrc != actualCrc) {
                    m_lastError = "CRC mismatch for " + fileName + 
                                 ": expected " + crcStr + 
                                 ", got " + std::to_string(actualCrc);
                    // Note: Just warning, not failing
                    printf("Warning: %s\n", m_lastError.c_str());
                }
            }
            
            // Apply repeat
            for (int r = 0; r < repeat; r++) {
                romData.insert(romData.end(), partData.begin(), partData.end());
            }
            
        } else if (nodeName == "interleave") {
            // Handle interleaved parts
            int outputWidth = child.attribute("output").as_int(8);
            if (outputWidth != 8 && outputWidth != 16 && outputWidth != 32) {
                m_lastError = "Invalid interleave output width: " + std::to_string(outputWidth);
                return false;
            }
            
            std::vector<std::vector<uint8_t>> interleaveParts;
            std::vector<std::string> maps;
            
            // Load all interleaved parts
            for (auto part : child.children("part")) {
                std::string fileName = part.attribute("name").as_string("");
                std::string mapStr = part.attribute("map").as_string("");
                std::string crcStr = part.attribute("crc").as_string("");
                
                // Parse CRC if provided
                uint32_t crc32Value = 0;
                if (!crcStr.empty()) {
                    try {
                        crc32Value = std::stoul(crcStr, nullptr, 16);
                    } catch (...) {
                        crc32Value = 0;
                    }
                }
                
                std::vector<uint8_t> partData;
                
                if (!fileName.empty()) {
                    // Load from file
                    if (!loadFileData(zipPath, fileName, partData, crc32Value)) {
                        m_lastError = "Failed to load interleave file: " + fileName;
                        return false;
                    }
                } else {
                    // Parse hex data from text content
                    std::string hexData = part.child_value();
                    if (!hexData.empty()) {
                        if (!parseHexString(hexData, partData)) {
                            m_lastError = "Failed to parse hex data in interleave";
                            return false;
                        }
                    }
                }
                
                // Verify CRC if specified
                if (!crcStr.empty()) {
                    uint32_t expectedCrc = std::stoul(crcStr, nullptr, 16);
                    uint32_t actualCrc = calculateCRC32(partData);
                    if (expectedCrc != actualCrc) {
                        printf("Warning: CRC mismatch for %s\n", fileName.c_str());
                    }
                }
                
                interleaveParts.push_back(partData);
                maps.push_back(mapStr);
            }
            
            // Perform interleaving: combine multiple parts into wider output
            if (interleaveParts.empty()) {
                m_lastError = "No parts in interleave section";
                return false;
            }
            
            size_t maxSize = 0;
            for (const auto& part : interleaveParts) {
                maxSize = std::max(maxSize, part.size());
            }
            
            int bytesPerUnit = outputWidth / 8;  // bytes per output unit
            
            // Create temporary storage for each output position
            std::vector<std::vector<uint8_t>> outputSlots(bytesPerUnit);
            
            // Process each part and place bytes according to map
            for (size_t partIdx = 0; partIdx < interleaveParts.size(); partIdx++) {
                const auto& part = interleaveParts[partIdx];
                const std::string& mapStr = maps[partIdx];
                
                // Convert map string to determine byte placement
                // For output="16": map="10" means high byte, map="01" means low byte
                std::vector<int> targetSlots;
                
                if (mapStr.empty()) {
                    // No map specified, place in order
                    targetSlots.push_back(partIdx % bytesPerUnit);
                } else {
                    // Parse map string: "01" means low byte (slot 0), "10" means high byte (slot 1)
                    // Each character represents a 4-bit nibble position
                    for (size_t i = 0; i < mapStr.length(); i++) {
                        char c = mapStr[i];
                        if (c == '1') {
                            // Bit set - this part goes to slot i
                            if (i < bytesPerUnit) {
                                targetSlots.push_back(i);
                            }
                        }
                    }
                }
                
                // Place all bytes from this part into target slots
                for (size_t byteIdx = 0; byteIdx < part.size(); byteIdx++) {
                    for (int slot : targetSlots) {
                        outputSlots[slot].push_back(part[byteIdx]);
                    }
                }
            }
            
            // Interleave the output: take one byte from each slot in order
            size_t maxSlotSize = 0;
            for (const auto& slot : outputSlots) {
                maxSlotSize = std::max(maxSlotSize, slot.size());
            }
            
            for (size_t unitIdx = 0; unitIdx < maxSlotSize; unitIdx++) {
                for (int slot = 0; slot < bytesPerUnit; slot++) {
                    if (unitIdx < outputSlots[slot].size()) {
                        romData.push_back(outputSlots[slot][unitIdx]);
                    } else {
                        romData.push_back(0);  // Pad with zeros
                    }
                }
            }
        }
    }
    
    return true;
}

bool MRALoader::parseHexString(const std::string& hexStr, std::vector<uint8_t>& output)
{
    std::string cleaned;
    
    // Remove whitespace and convert to uppercase
    for (char c : hexStr) {
        if (!std::isspace(c)) {
            cleaned += std::toupper(c);
        }
    }
    
    // Must have even number of hex digits
    if (cleaned.length() % 2 != 0) {
        return false;
    }
    
    // Convert hex pairs to bytes
    for (size_t i = 0; i < cleaned.length(); i += 2) {
        std::string byteStr = cleaned.substr(i, 2);
        try {
            uint8_t byte = std::stoul(byteStr, nullptr, 16);
            output.push_back(byte);
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

bool MRALoader::loadFileData(const std::string& zipPath, const std::string& fileName, 
                             std::vector<uint8_t>& output, uint32_t crc32)
{
    if (!zipPath.empty() && std::filesystem::exists(zipPath)) {
        // Try to load from zip file
        mz_zip_archive zip;
        mz_zip_zero_struct(&zip);
        
        if (mz_zip_reader_init_file(&zip, zipPath.c_str(), 0)) {
            int fileIndex = -1;
            
            // First try to find by CRC if provided
            if (crc32 != 0) {
                fileIndex = searchByCRC(&zip, crc32);
            }
            
            // If CRC search failed or no CRC provided, search by name
            if (fileIndex < 0) {
                fileIndex = mz_zip_reader_locate_file(&zip, fileName.c_str(), nullptr, 0);
            }
            
            if (fileIndex >= 0) {
                mz_zip_archive_file_stat fileStat;
                if (mz_zip_reader_file_stat(&zip, fileIndex, &fileStat)) {
                    output.resize(fileStat.m_uncomp_size);
                    
                    if (mz_zip_reader_extract_to_mem(&zip, fileIndex, output.data(), 
                                                     output.size(), 0)) {
                        mz_zip_reader_end(&zip);
                        return true;
                    }
                }
            }
            
            mz_zip_reader_end(&zip);
        }
    }
    
    // Try to load from filesystem using global file search
    if (g_fs.loadFile(fileName, output)) {
        return true;
    }
    
    // Try direct file load as last resort
    std::ifstream file(fileName, std::ios::binary);
    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        output.resize(size);
        file.read(reinterpret_cast<char*>(output.data()), size);
        return file.good();
    }
    
    return false;
}

bool MRALoader::applyMap(std::vector<uint8_t>& data, const std::string& mapStr)
{
    // Map string like "0001" means take every 4th byte starting at offset 3
    // Map string like "01" means take odd bytes
    
    if (mapStr.empty()) {
        return true;
    }
    
    std::vector<uint8_t> result;
    size_t mapLen = mapStr.length();
    
    for (size_t i = 0; i < data.size(); i++) {
        char mapChar = mapStr[i % mapLen];
        if (mapChar == '1') {
            result.push_back(data[i]);
        }
    }
    
    data = std::move(result);
    return true;
}

uint32_t MRALoader::calculateCRC32(const std::vector<uint8_t>& data)
{
    return mz_crc32(MZ_CRC32_INIT, data.data(), data.size());
}

int MRALoader::searchByCRC(mz_zip_archive* zip, uint32_t crc32)
{
    for (unsigned int fileIndex = 0; fileIndex < zip->m_total_files; fileIndex++) {
        mz_zip_archive_file_stat fileStat;
        if (mz_zip_reader_file_stat(zip, fileIndex, &fileStat)) {
            if (fileStat.m_crc32 == crc32) {
                return fileIndex;
            }
        }
    }
    return -1;
}