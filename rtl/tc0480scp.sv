/* RAM ACCESS

_Line Start_
Stall
Stall
Stall
Stall
BG2 row select
BG3 row select
BG2 row zoom
BG3 row zoom
BG0 row scroll
BG1 row scroll
BG2 row scroll
BG3 row scroll
BG0 row scroll fine
BG1 row scroll fine
BG2 row scroll fine
BG3 row scroll fine


_Line_
CPU
FG0
BG0
BG0
FG0 Gfx
FG0 Gfx
BG1
BG1
CPU
FG0
BG2
BG2
FG0 Gfx
FG0 Gfx
BG3
BG3

*/

typedef enum [4:0]
{
    WAIT0 = 5'd0,
    WAIT1,
    WAIT2,
    WAIT3,
    BG2_ROW_SELECT,
    BG3_ROW_SELECT,
    BG2_ROW_ZOOM,
    BG3_ROW_ZOOM,
    BG0_ROW_SCROLL,
    BG1_ROW_SCROLL,
    BG2_ROW_SCROLL,
    BG3_ROW_SCROLL,
    BG0_ROW_SCROLL2,
    BG1_ROW_SCROLL2,
    BG2_ROW_SCROLL2,
    BG3_ROW_SCROLL2,

    CPU_ACCESS_0,
    FG0_ATTRIB_0,
    BG0_ATTRIB0,
    BG0_ATTRIB1,
    FG0_GFX0_0,
    FG0_GFX1_0,
    BG1_ATTRIB0,
    BG1_ATTRIB1,
    CPU_ACCESS_1,
    FG0_ATTRIB_1,
    BG2_ATTRIB0,
    BG2_ATTRIB1,
    FG0_GFX0_1,
    FG0_GFX1_1,
    BG3_ATTRIB0,
    BG3_ATTRIB1
} access_state_t;

module tc0480scp_shifter #(
    parameter PALETTE_WIDTH=8,
    parameter PIXEL_WIDTH=4,
    parameter DATA_WIDTH=8
    )
(
    input clk,
    input ce_pixel,
    input load,

    input [$clog2(DATA_WIDTH)-1:0] tap,
    input [(PIXEL_WIDTH * DATA_WIDTH) - 1:0] gfx_in,
    input [PALETTE_WIDTH - 1:0] palette_in,
    input flip_x,
    output [PIXEL_WIDTH + PALETTE_WIDTH - 1:0] dot_out
);

localparam DOT_WIDTH = (PALETTE_WIDTH + PIXEL_WIDTH);
localparam SHIFT_END = (DOT_WIDTH * DATA_WIDTH * 2) - 1;

reg [SHIFT_END:0] shift;

assign dot_out = shift[(SHIFT_END - (DOT_WIDTH * tap)) -: DOT_WIDTH];

always_ff @(posedge clk) begin
    if (ce_pixel) begin
        shift[SHIFT_END:DOT_WIDTH] <= shift[SHIFT_END-DOT_WIDTH:0];
        if (load) begin
            int i;
            if (flip_x) begin
                for( i = 0; i < DATA_WIDTH; i = i + 1 ) begin
                    shift[(DOT_WIDTH * ((DATA_WIDTH-1) - i)) +: DOT_WIDTH] <= { palette_in, gfx_in[(PIXEL_WIDTH * i) +: PIXEL_WIDTH] };
                end
            end else begin
                for( i = 0; i < DATA_WIDTH; i = i + 1 ) begin
                    shift[(DOT_WIDTH * i) +: DOT_WIDTH] <= { palette_in, gfx_in[(PIXEL_WIDTH * i) +: PIXEL_WIDTH] };
                end
            end
        end
    end
end

endmodule

module TC0480SCP #(parameter SS_IDX=-1) (
    input clk,
    input ce,

    input reset,

    // CPU interface
    input [17:0] VA,
    input [15:0] VDin,
    output reg [15:0] VDout,
    input LDSn,
    input UDSn,
    input CSn,
    input RW,
    output VDTACKn,

    // RAM interface
    output     [14:0] RA,
    input      [15:0] RADin,
    output     [15:0] RADout,
    output            RWALn,
    output            RWAHn,
    output            RADOEn,

    // ROM interface
    output reg [20:0] rom_address,
    input      [63:0] rom_data,
    output reg        rom_req,
    input             rom_ack,


    // Video interface
    output [15:0] SD,
    output HSYNn,
    output HBLNn,
    output VSYNn,
    output VBLNn,

    output HLDn,
    output VLDn,
    input OUHLDn, // FIXME - confirm inputs
    input OUVLDn,

    ssbus_if.slave ssbus
);

reg dtack_n;
reg prev_cs_n;

reg ram_pending = 0;
reg ram_access = 0;
reg [15:0] ram_addr;

reg [15:0] ctrl[32];

reg [4:0] access_cycle;
logic [4:0] next_access_cycle;

assign VDTACKn = CSn ? 0 : dtack_n;
assign RA = ram_addr[15:1];

always_comb begin
    ram_addr = 16'd0;
    RWAHn = 1;
    RWALn = 1;
    RADOEn = 0;
    RADout = 16'd0;

    next_access_cycle = access_cycle + 5'd1;

    unique case (access_cycle)
        WAIT0,
        WAIT1,
        WAIT2,
        WAIT3: begin
        end

        BG2_ROW_SELECT: ram_addr = 16'd0;
        BG3_ROW_SELECT: ram_addr = 16'd0;

        BG2_ROW_ZOOM: ram_addr = 16'd0;
        BG3_ROW_ZOOM: ram_addr = 16'd0;

        BG0_ROW_SCROLL: ram_addr = 16'd0;
        BG1_ROW_SCROLL: ram_addr = 16'd0;
        BG2_ROW_SCROLL: ram_addr = 16'd0;
        BG3_ROW_SCROLL: ram_addr = 16'd0;

        BG0_ROW_SCROLL2: ram_addr = 16'd0;
        BG1_ROW_SCROLL2: ram_addr = 16'd0;
        BG2_ROW_SCROLL2: ram_addr = 16'd0;
        BG3_ROW_SCROLL2: ram_addr = 16'd0;

        CPU_ACCESS_0,
        CPU_ACCESS_1: begin
            ram_addr = VA[15:0];
            RADout = VDin;
            RWALn = ~ram_access | LDSn | RW;
            RWAHn = ~ram_access | UDSn | RW;
        end

        FG0_ATTRIB_0,
        FG0_ATTRIB_1: ram_addr = 16'd0;

        BG0_ATTRIB0: ram_addr = 16'd0;
        BG0_ATTRIB1: ram_addr = 16'd0;

        FG0_GFX0_0,
        FG0_GFX0_1: ram_addr = 16'd0;
        FG0_GFX1_0,
        FG0_GFX1_1: ram_addr = 16'd0;

        BG1_ATTRIB0: ram_addr = 16'd0;
        BG1_ATTRIB1: ram_addr = 16'd0;

        BG2_ATTRIB0: ram_addr = 16'd0;
        BG2_ATTRIB1: ram_addr = 16'd0;

        BG3_ATTRIB0: ram_addr = 16'd0;
        BG3_ATTRIB1: ram_addr = 16'd0;
    endcase
end

always @(posedge clk) begin
    bit [8:0] v;
    bit [5:0] h;

    if (reset) begin
        dtack_n <= 1;
        ram_pending <= 0;
        ram_access <= 0;
    end else if (ce) begin
        // CPu interface handling
        prev_cs_n <= CSn;
        if (~CSn & prev_cs_n) begin // CS edge
            if (VA[17]) begin // control access
                if (RW) begin
                    VDout <= ctrl[VA[5:1]];
                end else begin
                    if (~UDSn) ctrl[VA[5:1]][15:8] <= VDin[15:8];
                    if (~LDSn) ctrl[VA[5:1]][7:0]  <= VDin[7:0];
                end
                dtack_n <= 0;
            end else begin // ram access
                ram_pending <= 1;
            end
        end else if (CSn) begin
            dtack_n <= 1;
        end

        access_cycle <= next_access_cycle;

        case(access_cycle)
            CPU_ACCESS_0,
            CPU_ACCESS_1: begin
                if (ram_access) begin
                    ram_access <= 0;
                    ram_pending <= 0;
                    dtack_n <= 0;
                    VDout <= RADin;
                end
            end

            default: begin
            end
        endcase

        case(next_access_cycle)
            CPU_ACCESS_0,
            CPU_ACCESS_1: begin
                ram_access <= ram_pending;
            end

            default: begin
            end
        endcase
    end

    ssbus.setup(SS_IDX, 32, 1);
    if (ssbus.access(SS_IDX)) begin
        if (ssbus.write) begin
            ctrl[ssbus.addr[4:0]] <= ssbus.data[15:0];
            ssbus.write_ack(SS_IDX);
        end else if (ssbus.read) begin
            ssbus.read_response(SS_IDX, { 48'b0, ctrl[ssbus.addr[4:0]] });
        end
    end
end

endmodule

