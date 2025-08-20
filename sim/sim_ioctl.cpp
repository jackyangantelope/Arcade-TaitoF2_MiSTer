#include "sim_ioctl.h"
#include "mra_loader.h"
#include "file_search.h"
#include "F2.h"
#include "F2___024root.h"
#include "sim_sdram.h"
#include "sim_ddr.h"
#include "sim_core.h"
#include <iostream>

extern SimSDRAM sdram;
extern SimDDR ddr_memory;

SimIOCTL::SimIOCTL(F2* core) : m_core(core)
{
}

SimIOCTL::~SimIOCTL()
{
}

bool SimIOCTL::loadMRA(const std::string& mraPath, const std::vector<std::string>& romSearchPaths)
{
    m_lastError.clear();
    
    // Add ROM search paths
    for (const auto& path : romSearchPaths) {
        g_fs.addSearchPath(path);
    }
    
    // Add the directory containing the MRA file as a search path
    size_t lastSlash = mraPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        g_fs.addSearchPath(mraPath.substr(0, lastSlash));
    }
    
    // Load the MRA file
    MRALoader loader;
    std::vector<uint8_t> romData;
    
    if (!loader.load(mraPath, romData)) {
        m_lastError = "Failed to load MRA: " + loader.getLastError();
        return false;
    }
    
    std::cout << "Loaded MRA: " << mraPath << std::endl;
    std::cout << "ROM data size: " << romData.size() << " bytes" << std::endl;
    
    // Send ROM data via ioctl interface (index 0)
    return sendIOCTLData(0, romData);
}

bool SimIOCTL::sendIOCTLData(uint8_t index, const std::vector<uint8_t>& data)
{
    if (!m_core) {
        m_lastError = "Core not initialized";
        return false;
    }
    
    std::cout << "Starting ioctl download (index=" << (int)index << ", size=" << data.size() << ")" << std::endl;
    
    // Start download sequence
    m_core->reset = 1;
    m_core->ioctl_download = 1;
    m_core->ioctl_index = index;
    m_core->ioctl_wr = 0;
    m_core->ioctl_addr = 0;
    m_core->ioctl_dout = 0;
    
    // Clock to let the core see download start
    clock();
    
    // Send each byte
    for (size_t i = 0; i < data.size(); i++) {
        // Set up data and address
        m_core->ioctl_addr = i;
        m_core->ioctl_dout = data[i];
        m_core->ioctl_wr = 1;
        
        // Clock and wait for ready
        clock();
        waitForIOCTLReady();
        
        // Deassert write
        m_core->ioctl_wr = 0;
        clock();
        
        // Progress indicator every 64KB
        if ((i & 0xFFFF) == 0) {
            std::cout << "  Sent " << i << "/" << data.size() << " bytes" << std::endl;
        }
    }
    
    // End download sequence
    m_core->ioctl_download = 0;
    m_core->reset = 0;
    clock();
    
    std::cout << "ioctl download complete" << std::endl;
    return true;
}

void SimIOCTL::waitForIOCTLReady()
{
    int timeout = 1000; // Prevent infinite loops
    
    while (m_core->ioctl_wait && timeout > 0) {
        clock();
        timeout--;
    }
    
    if (timeout == 0) {
        std::cerr << "Warning: ioctl_wait timeout" << std::endl;
    }
}

void SimIOCTL::clock()
{
    // Use the proper sim_tick function to advance the simulation
    sim_tick(1);
}
