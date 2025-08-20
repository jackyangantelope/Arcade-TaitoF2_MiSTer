
#ifndef SIM_CORE_H
#define SIM_CORE_H

#include <functional>
#include <memory>
#include <vector>
#include <cstdint>

#include "games.h"

class VerilatedContext;
class F2;
class VerilatedFstC;
class SimSDRAM;
class SimDDR;
class SimVideo;

class SimCore {
public:
    // Public members that external code needs access to
    SimVideo* video;
    F2* top;
    SimDDR* ddr_memory;
    SimSDRAM* sdram;
    
    // Simulation state (made public for compatibility)
    uint64_t m_total_ticks;
    bool m_simulation_run;
    bool m_simulation_step;
    int m_simulation_step_size;
    bool m_simulation_step_vblank;
    uint64_t m_simulation_reset_until;
    bool m_system_pause;
    bool m_simulation_wp_set;
    int m_simulation_wp_addr;
    bool m_trace_active;
    char m_trace_filename[64];
    int m_trace_depth;
    
    // Constructor/Destructor
    SimCore();
    ~SimCore();
    
    // Main simulation methods
    void Init();
    void Tick(int count = 1);
    void TickUntil(std::function<bool()> until);
    void Shutdown();
    
    // Trace control methods
    void StartTrace(const char* filename, int depth = 1);
    void StopTrace();
    bool IsTraceActive() const { return m_trace_active; }
    
    // IOCTL methods  
    bool SendIOCTLData(uint8_t index, const std::vector<uint8_t>& data);
    
    // Stats
    uint64_t GetTotalTicks() const { return m_total_ticks; }

    void SetGame(game_t game);
    game_t GetGame() const;
    const char *GetGameName() const;
    
private:
    // Verilator context and top module
    VerilatedContext* m_contextp;
    std::unique_ptr<VerilatedFstC> m_tfp;
    
    // Memory subsystems
    std::unique_ptr<SimSDRAM> m_sdram_impl;
    std::unique_ptr<SimDDR> m_ddr_memory_impl;
    std::unique_ptr<SimVideo> m_video_impl;
    
    // IOCTL helper methods
    void WaitForIOCTLReady();
    
};

// Global instance
extern SimCore g_sim_core;



#endif // SIM_CORE_H
