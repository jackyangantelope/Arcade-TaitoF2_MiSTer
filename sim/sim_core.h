
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
class GfxCache;

class SimCore {
public:
    // Public members that external code needs access to
    F2* top;
    std::unique_ptr<SimVideo> video;
    std::unique_ptr<SimDDR> ddr_memory;
    std::unique_ptr<SimSDRAM> sdram;

    std::unique_ptr<GfxCache> gfx_480scp;
    std::unique_ptr<GfxCache> gfx_200obj;
    
    // Simulation state (made public for compatibility)
    uint64_t m_total_ticks;
    bool m_simulation_run;
    bool m_simulation_step;
    int m_simulation_step_size;
    bool m_simulation_step_vblank;
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
    bool SendIOCTLDataDDR(uint8_t index, uint32_t addr, const std::vector<uint8_t>& data);
    
    // Stats
    uint64_t GetTotalTicks() const { return m_total_ticks; }

    void SetGame(game_t game);
    game_t GetGame() const;
    const char *GetGameName() const;
    
private:
    // Verilator context and top module
    VerilatedContext* m_contextp;
    std::unique_ptr<VerilatedFstC> m_tfp;
    
    // IOCTL helper methods
    void WaitForIOCTLReady();
    
};

// Global instance
extern SimCore g_sim_core;



#endif // SIM_CORE_H
