#ifndef SIM_IOCTL_H
#define SIM_IOCTL_H

#include <string>
#include <vector>
#include <cstdint>

class F2;

/**
 * Simulator IOCTL interface for loading MRA files
 * Simulates the MiSTer framework's ioctl behavior
 */
class SimIOCTL
{
public:
    SimIOCTL(F2* core);
    ~SimIOCTL();
    
    /**
     * Load an MRA file and send data via ioctl interface
     * @param mraPath Path to the .mra file
     * @param romSearchPaths Optional paths to search for ROM files
     * @return true if successful
     */
    bool loadMRA(const std::string& mraPath, const std::vector<std::string>& romSearchPaths = {});
    
    /**
     * Simulate ioctl download process by sending data to core
     * @param index ioctl index (0 for main ROM)
     * @param data ROM data to send
     * @return true if successful
     */
    bool sendIOCTLData(uint8_t index, const std::vector<uint8_t>& data);
    
    /**
     * Get the last error message
     */
    const std::string& getLastError() const { return m_lastError; }
    
private:
    F2* m_core;
    std::string m_lastError;
    
    /**
     * Clock the simulator while waiting for ioctl_wait to deassert
     */
    void waitForIOCTLReady();
    
    /**
     * Single clock cycle for the simulator
     */
    void clock();
};

#endif // SIM_IOCTL_H