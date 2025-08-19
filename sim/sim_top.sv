import system_consts::*;

module sim_top(
    input             clk,
    input             reset,

    input  game_t     game,

    output            ce_pixel,
    output            hsync,
    output            hblank,
    output            vsync,
    output            vblank,
    output      [7:0] red,
    output      [7:0] green,
    output      [7:0] blue,

    input       [9:0] joystick_p1,
    input       [9:0] joystick_p2,
    input       [9:0] joystick_p3,
    input       [9:0] joystick_p4,
    input       [3:0] start,
    input       [3:0] coin,

    input             analog_inc,
    input             analog_abs,
    input       [7:0] analog_p1,
    input       [7:0] analog_p2,

    input       [7:0] dswa,
    input       [7:0] dswb,

    output reg [26:0] sdr_addr,
    input      [63:0] sdr_q,
    output reg [63:0] sdr_data,
    output reg        sdr_req,
    output reg        sdr_rw,
    output reg  [7:0] sdr_be,
    input             sdr_ack,

    // Memory stream interface
    output            ddr_acquire,
    output     [31:0] ddr_addr,
    output     [63:0] ddr_wdata,
    input      [63:0] ddr_rdata,
    output            ddr_read,
    output            ddr_write,
    output      [7:0] ddr_burstcnt,
    output      [7:0] ddr_byteenable,
    input             ddr_busy,
    input             ddr_read_complete,

    input      [12:0] obj_debug_idx,

    output     [15:0] audio_out,
    input       [1:0] audio_filter_en,

    input             ss_do_save,
    input             ss_do_restore,
    input       [1:0] ss_index,
    output      [3:0] ss_state_out,

    input      [23:0] bram_addr,
    input       [7:0] bram_data,
    input             bram_wr,

    input             sync_fix,

    input             pause
);

wire [26:0] sdr_cpu_addr, sdr_scn0_addr, sdr_scn_mux_addr, sdr_audio_addr;
wire sdr_cpu_req, sdr_scn0_req, sdr_scn_mux_req, sdr_audio_req;
reg  sdr_cpu_ack, sdr_scn0_ack, sdr_scn_mux_ack, sdr_audio_ack;
reg [63:0] sdr_cpu_q;
reg [31:0] sdr_scn0_q;
reg [63:0] sdr_scn_mux_q;
reg [15:0] sdr_audio_q;

// Instantiate the F2 module
F2 f2_inst(
    .clk(clk),
    .reset(reset),
    .game(game),
    
    .ce_pixel(ce_pixel),
    .hsync(hsync),
    .hblank(hblank),
    .vsync(vsync),
    .vblank(vblank),
    .red(red),
    .green(green),
    .blue(blue),
    
    .joystick_p1(joystick_p1),
    .joystick_p2(joystick_p2),
    .joystick_p3(joystick_p3),
    .joystick_p4(joystick_p4),
    .start(start),
    .coin(coin),
    
    .analog_inc(analog_inc),
    .analog_abs(analog_abs),
    .analog_p1(analog_p1),
    .analog_p2(analog_p2),
    
    .dswa(dswa),
    .dswb(dswb),
    
    .sdr_cpu_addr(sdr_cpu_addr),
    .sdr_cpu_q(sdr_cpu_q),
    .sdr_cpu_req(sdr_cpu_req),
    .sdr_cpu_ack(sdr_cpu_ack),
    
    .sdr_scn0_addr(sdr_scn0_addr),
    .sdr_scn0_q(sdr_scn0_q),
    .sdr_scn0_req(sdr_scn0_req),
    .sdr_scn0_ack(sdr_scn0_ack),
    
    .sdr_scn_mux_addr(sdr_scn_mux_addr),
    .sdr_scn_mux_q(sdr_scn_mux_q),
    .sdr_scn_mux_req(sdr_scn_mux_req),
    .sdr_scn_mux_ack(sdr_scn_mux_ack),
    
    .sdr_audio_addr(sdr_audio_addr),
    .sdr_audio_q(sdr_audio_q),
    .sdr_audio_req(sdr_audio_req),
    .sdr_audio_ack(sdr_audio_ack),
    
    .ddr_acquire(ddr_acquire),
    .ddr_addr(ddr_addr),
    .ddr_wdata(ddr_wdata),
    .ddr_rdata(ddr_rdata),
    .ddr_read(ddr_read),
    .ddr_write(ddr_write),
    .ddr_burstcnt(ddr_burstcnt),
    .ddr_byteenable(ddr_byteenable),
    .ddr_busy(ddr_busy),
    .ddr_read_complete(ddr_read_complete),
    
    .obj_debug_idx(obj_debug_idx),
    
    .audio_out(audio_out),
    .audio_filter_en(audio_filter_en),
    
    .ss_do_save(ss_do_save),
    .ss_do_restore(ss_do_restore),
    .ss_index(ss_index),
    .ss_state_out(ss_state_out),
    
    .bram_addr(bram_addr),
    .bram_data(bram_data),
    .bram_wr(bram_wr),
    
    .sync_fix(sync_fix),
    
    .pause(pause)
);

reg sdr_active;
reg [1:0] sdr_active_ch;

always_ff @(posedge clk) begin
    if (sdr_active) begin
        if (sdr_req == sdr_ack) begin
            case(sdr_active_ch)
            0: begin
                sdr_cpu_ack <= sdr_cpu_req;
                sdr_cpu_q <= sdr_q;
            end
            1: begin
                sdr_scn0_ack <= sdr_scn0_req;
                sdr_scn0_q <= sdr_q[31:0];
            end
            2: begin
                sdr_scn_mux_ack <= sdr_scn_mux_req;
                sdr_scn_mux_q <= sdr_q;
            end
            3: begin
                sdr_audio_ack <= sdr_audio_req;
                sdr_audio_q <= sdr_q[15:0];
            end
            endcase

            sdr_active <= 0;
        end
    end else begin
        if (sdr_scn0_req != sdr_scn0_ack) begin
            sdr_rw <= 1;
            sdr_addr <= sdr_scn0_addr;
            sdr_req <= ~sdr_req;
            sdr_active <= 1;
            sdr_active_ch <= 1;
        end else if (sdr_scn_mux_req != sdr_scn_mux_ack) begin
            sdr_rw <= 1;
            sdr_addr <= sdr_scn_mux_addr;
            sdr_req <= ~sdr_req;
            sdr_active <= 1;
            sdr_active_ch <= 2;
        end else if (sdr_audio_req != sdr_audio_ack) begin
            sdr_rw <= 1;
            sdr_addr <= sdr_audio_addr;
            sdr_req <= ~sdr_req;
            sdr_active <= 1;
            sdr_active_ch <= 3;
        end else if (sdr_cpu_req != sdr_cpu_ack) begin
            sdr_rw <= 1;
            sdr_addr <= sdr_cpu_addr;
            sdr_req <= ~sdr_req;
            sdr_active <= 1;
            sdr_active_ch <= 0;
        end
    end
end

endmodule
