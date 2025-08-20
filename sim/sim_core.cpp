
#include "sim_core.h"
#include "sim_hierarchy.h"
#include "F2.h"
#include "F2___024root.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "sim_sdram.h"
#include "sim_ddr.h"
#include "sim_video.h"
#include <cstring>
#include <cstdio>

// Global instance
SimCore g_sim_core;

// SimCore implementation
SimCore::SimCore() 
    : video(nullptr), top(nullptr), ddr_memory(nullptr), sdram(nullptr),
      m_contextp(nullptr), m_total_ticks(0), m_trace_active(false), m_trace_depth(1),
      m_simulation_run(false), m_simulation_step(false), m_simulation_step_size(100000),
      m_simulation_step_vblank(false), m_simulation_reset_until(15000000),
      m_system_pause(false), m_simulation_wp_set(false), m_simulation_wp_addr(0)
{
    strcpy(m_trace_filename, "sim.fst");
}

SimCore::~SimCore()
{
    Shutdown();
}

void SimCore::Init()
{
    m_contextp = new VerilatedContext;
    top = new F2{m_contextp};
    m_tfp = nullptr;
    
    // Create memory subsystems
    m_sdram_impl = std::make_unique<SimSDRAM>(128 * 1024 * 1024);
    m_ddr_memory_impl = std::make_unique<SimDDR>(16 * 1024 * 1024);
    m_video_impl = std::make_unique<SimVideo>();
    
    // Set public pointers
    sdram = m_sdram_impl.get();
    ddr_memory = m_ddr_memory_impl.get();
    video = m_video_impl.get();
}

void SimCore::Tick(int count)
{
    for (int i = 0; i < count; i++)
    {
        m_total_ticks++;

        if (m_total_ticks < m_simulation_reset_until)
        {
            top->reset = 1;
        }
        else
        {
            top->reset = 0;
        }

        sdram->update_channel_64(0, 8, top->sdr_addr, top->sdr_req, top->sdr_rw,
                                top->sdr_be, top->sdr_data, &top->sdr_q, &top->sdr_ack);
        video->clock(top->ce_pixel != 0, top->hblank != 0, top->vblank != 0,
                    top->red, top->green, top->blue);

        ddr_memory->clock(top->ddr_addr, top->ddr_wdata, top->ddr_rdata,
                         top->ddr_read, top->ddr_write, top->ddr_busy,
                         top->ddr_read_complete, top->ddr_burstcnt,
                         top->ddr_byteenable);

        m_contextp->timeInc(1);
        top->clk = 0;

        top->eval();
        if (m_tfp)
            m_tfp->dump(m_contextp->time());

        m_contextp->timeInc(1);
        top->clk = 1;

        top->eval();
        if (m_tfp)
            m_tfp->dump(m_contextp->time());

        if (m_simulation_wp_set &&
            top->rootp->F2_SIGNAL(cpu_word_addr) == m_simulation_wp_addr)
        {
            m_simulation_run = false;
            m_simulation_step = false;
            return;
        }
    }
}

void SimCore::TickUntil(std::function<bool()> until)
{
    while (!until())
    {
        Tick(1);
    }
}

void SimCore::Shutdown()
{
    if (m_tfp)
    {
        m_tfp->close();
        m_tfp.reset();
    }

    if (top)
    {
        top->final();
        delete top;
        top = nullptr;
    }

    if (m_contextp)
    {
        delete m_contextp;
        m_contextp = nullptr;
    }
    
    // Reset public pointers
    sdram = nullptr;
    ddr_memory = nullptr;
    video = nullptr;
    
    // Reset member objects
    m_sdram_impl.reset();
    m_ddr_memory_impl.reset();
    m_video_impl.reset();
}

void SimCore::StartTrace(const char* filename, int depth)
{
    if (!m_contextp || !top) return;
    
    if (m_tfp)
    {
        m_tfp->close();
        m_tfp.reset();
    }
    
    strcpy(m_trace_filename, filename);
    m_trace_depth = depth;
    
    m_tfp = std::make_unique<VerilatedFstC>();
    top->trace(m_tfp.get(), m_trace_depth);
    m_tfp->open(m_trace_filename);
    m_trace_active = true;
}

void SimCore::StopTrace()
{
    if (m_tfp)
    {
        m_tfp->close();
        m_tfp.reset();
    }
    m_trace_active = false;
}

bool SimCore::SendIOCTLData(uint8_t index, const std::vector<uint8_t>& data)
{
    if (!top) {
        return false;
    }
    
    printf("Starting ioctl download (index=%d, size=%zu)\n", (int)index, data.size());
    
    // Start download sequence
    top->reset = 1;
    top->ioctl_download = 1;
    top->ioctl_index = index;
    top->ioctl_wr = 0;
    top->ioctl_addr = 0;
    top->ioctl_dout = 0;
    
    // Clock to let the core see download start
    Tick(1);
    
    // Send each byte
    for (size_t i = 0; i < data.size(); i++) {
        // Set up data and address
        top->ioctl_addr = i;
        top->ioctl_dout = data[i];
        top->ioctl_wr = 1;
        
        // Clock and wait for ready
        Tick(1);
        WaitForIOCTLReady();
        
        // Deassert write
        top->ioctl_wr = 0;
        Tick(1);
        
        // Progress indicator every 64KB
        if ((i & 0xFFFF) == 0) {
            printf("  Sent %zu/%zu bytes\n", i, data.size());
        }
    }
    
    // End download sequence
    top->ioctl_download = 0;
    top->reset = 0;
    Tick(1);
    
    printf("ioctl download complete\n");
    return true;
}

void SimCore::WaitForIOCTLReady()
{
    int timeout = 1000; // Prevent infinite loops
    
    while (top->ioctl_wait && timeout > 0) {
        Tick(1);
        timeout--;
    }
    
    if (timeout == 0) {
        printf("Warning: ioctl_wait timeout\n");
    }
}


