
#include "sim_core.h"
#include "F2.h"
#include "F2___024root.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "sim_sdram.h"
#include "sim_ddr.h"
#include "sim_video.h"

VerilatedContext *contextp;
F2 *top;
std::unique_ptr<VerilatedFstC> tfp;

SimSDRAM sdram(128 * 1024 * 1024);
SimDDR ddr_memory(16 * 1024 * 1024);
SimVideo video;

uint64_t total_ticks = 0;

bool trace_active = false;
char trace_filename[64];
int trace_depth = 1;

bool simulation_run = false;
bool simulation_step = false;
int simulation_step_size = 100000;
bool simulation_step_vblank = false;
uint64_t simulation_reset_until = 100;
bool system_pause = false;

bool simulation_wp_set = false;
int simulation_wp_addr = 0;

void sim_init() {
    contextp = new VerilatedContext;
    top = new F2{contextp};
    tfp = nullptr;
    strcpy(trace_filename, "sim.fst");
}

void sim_tick(int count)
{
    for( int i = 0; i < count; i++ )
    {
        total_ticks++;

        if (total_ticks < simulation_reset_until)
        {
            top->reset = 1;
        }
        else
        {
            top->reset = 0;
        }
        
        sdram.update_channel_64(top->sdr_cpu_addr, top->sdr_cpu_req, 1, 0, 0, &top->sdr_cpu_q, &top->sdr_cpu_ack);
        sdram.update_channel_32(top->sdr_scn0_addr, top->sdr_scn0_req, 1, 0, 0, &top->sdr_scn0_q, &top->sdr_scn0_ack);
        sdram.update_channel_64(top->sdr_scn_mux_addr, top->sdr_scn_mux_req, 1, 0, 0, &top->sdr_scn_mux_q, &top->sdr_scn_mux_ack);
        sdram.update_channel_16(top->sdr_audio_addr, top->sdr_audio_req, 1, 0, 0, &top->sdr_audio_q, &top->sdr_audio_ack);
        video.clock(top->ce_pixel != 0, top->hblank != 0, top->vblank != 0, top->red, top->green, top->blue);
        
        ddr_memory.clock(top->ddr_addr, top->ddr_wdata, top->ddr_rdata, top->ddr_read, top->ddr_write, top->ddr_busy, top->ddr_read_complete, top->ddr_burstcnt, top->ddr_byteenable);

        contextp->timeInc(1);
        top->clk = 0;

        top->eval();
        if (tfp) tfp->dump(contextp->time());

        contextp->timeInc(1);
        top->clk = 1;

        top->eval();
        if (tfp) tfp->dump(contextp->time());

        if (simulation_wp_set && top->rootp->F2__DOT__cpu_word_addr == simulation_wp_addr)
        {
            simulation_run = false;
            simulation_step = false;
            return;
        }
    }
}

void sim_tick_until(std::function<bool()> until)
{
    while(!until())
    {
        sim_tick(1);
    }
}

void sim_shutdown() {
    if (tfp)
    {
        tfp->close();
        tfp.reset();
    }

    top->final();

    delete top;
    delete contextp;
}
