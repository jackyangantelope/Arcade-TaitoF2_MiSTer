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

module tc0480scp_shifter #(
    parameter TILE_WIDTH=8,
    parameter LENGTH=4
    )
(
    input                                         clk,
    input                                         ce,

    input                                         load,
    input [$clog2(LENGTH)-1:0]                    load_index,
    input [7:0]                                   load_color,
    input [(TILE_WIDTH*4)-1:0]                    load_data,
    input                                         load_flip,

    input [$clog2(LENGTH)+$clog2(TILE_WIDTH)-1:0] tap,
    output reg [11:0]                             dot_out
);

reg [7:0] color_buf[LENGTH];
reg [(TILE_WIDTH*4)-1:0] pixel_buf[LENGTH];

wire [$clog2(LENGTH)-1:0] tap_index = tap[$left(tap):$clog2(TILE_WIDTH)];
wire [$clog2(TILE_WIDTH)-1:0] tap_pixel = tap[$clog2(TILE_WIDTH)-1:0];

always_ff @(posedge clk) begin
    if (load) begin

        color_buf[load_index] <= load_color;
        if (load_flip) begin
            int i;
            for( i = 0; i < TILE_WIDTH; i = i + 1 ) begin
                pixel_buf[load_index][(4 * ((TILE_WIDTH-1) - i)) +: 4] <= load_data[(4 * i) +: 4];
            end
        end else begin
            pixel_buf[load_index] <= load_data;
        end
    end

    if (ce) begin
        dot_out <= { color_buf[tap_index], pixel_buf[tap_index][(4 * tap_pixel) +: 4] };
    end
end

endmodule


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

module tc0480scp_counter #(
    parameter TILE_BITS=3
    )
(
    input clk,
    input ce,

    input line_strobe,
    input frame_strobe,

    input [15:0] xbase,
    input [15:0] ybase,

    input [15:0] xofs,
    input [15:0] yofs,

    output [8:0] x,
    output [8:0] y,

    output reg [8-TILE_BITS:0] xtile
);

reg [8:0] xcnt;
reg [8:0] ycnt;
reg [TILE_BITS-1:0] tilecnt;

assign x = xcnt;
assign y = ycnt;

wire [8:0] xstart = xbase[8:0] - xofs[8:0];
wire [8:0] ystart = ybase[8:0] + yofs[8:0];

always_ff @(posedge clk) begin
    if (ce) begin
        xcnt <= xcnt + 9'd1;
        tilecnt <= tilecnt + 1;
        if (&tilecnt) begin
            xtile <= xtile + 1;
        end

        if (frame_strobe) begin
            tilecnt <= 0;
            xtile <= xstart[8:TILE_BITS] + 2;
            xcnt <= xstart[8:0];
            ycnt <= ystart;
        end

        if (line_strobe) begin
            tilecnt <= 0;
            xtile <= xstart[8:TILE_BITS] + 2;
            xcnt <= xstart[8:0];
            ycnt <= ycnt + 9'd1;
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
    output reg [22:0] rom_address,
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
    input OUHLDn,
    input OUVLDn,

    ssbus_if.slave ssbus
);


reg [15:0] ctrl[32];

wire [2:0] ctrl_prio     = ctrl[15][4:2];
wire       ctrl_wide     = ctrl[15][7];
wire       ctrl_flip     = ctrl[15][6];
wire       ctrl_sync     = ctrl[15][5];
wire       ctrl_bg3_zoom = ctrl[15][1];
wire       ctrl_bg2_zoom = ctrl[15][0];

wire [8:0] dispx, dispy;

wire [8:0] fg0_xcnt, fg0_ycnt;
wire [5:0] fg0_xtile;

reg [15:0] base_xofs;
reg [15:0] base_yofs;

reg line_strobe, frame_strobe;
tc0480scp_counter raw_counter(
    .clk,
    .ce,
    .line_strobe,
    .frame_strobe,
    .xbase(0),
    .ybase(0),
    .xofs(0),
    .yofs(0),
    .x(dispx),
    .y(dispy),
    .xtile()
);

tc0480scp_counter fg0_counter(
    .clk,
    .ce,
    .line_strobe,
    .frame_strobe,
    .xbase(base_xofs-30),
    .ybase(base_yofs-49),
    .xofs(ctrl[12]),
    .yofs(ctrl[13]),
    .x(fg0_xcnt),
    .y(fg0_ycnt),
    .xtile(fg0_xtile)
);


wire [8:0] bg_xcnt[4], bg_ycnt[4];
wire [4:0] bg_xtile[4];
reg [4:0] bg_xtile2[4];
wire [11:0] bg_dot[4];
reg [31:0] bg_attrib[4];
genvar bg_index;

reg [63:0] rom_data_reg;

wire [63:0] rom_data_deswizzle =
{
    rom_data_reg[( 9 * 4) +: 4],
    rom_data_reg[( 8 * 4) +: 4],
    rom_data_reg[(11 * 4) +: 4],
    rom_data_reg[(10 * 4) +: 4],
    rom_data_reg[(13 * 4) +: 4],
    rom_data_reg[(12 * 4) +: 4],
    rom_data_reg[(15 * 4) +: 4],
    rom_data_reg[(14 * 4) +: 4],

    rom_data_reg[( 1 * 4) +: 4],
    rom_data_reg[( 0 * 4) +: 4],
    rom_data_reg[( 3 * 4) +: 4],
    rom_data_reg[( 2 * 4) +: 4],
    rom_data_reg[( 5 * 4) +: 4],
    rom_data_reg[( 4 * 4) +: 4],
    rom_data_reg[( 7 * 4) +: 4],
    rom_data_reg[( 6 * 4) +: 4]
};

generate
for (bg_index = 0; bg_index < 4; bg_index = bg_index + 1) begin
    tc0480scp_counter #(.TILE_BITS(4)) bg_counter(
        .clk,
        .ce,
        .line_strobe,
        .frame_strobe,
        .xbase(base_xofs-(31 + (bg_index * 4))),
        .ybase(base_yofs-23),
        .xofs(ctrl[0+bg_index]),
        .yofs(ctrl[4+bg_index]),
        .x(bg_xcnt[bg_index]),
        .y(bg_ycnt[bg_index]),
        .xtile(bg_xtile[bg_index])
    );

    tc0480scp_shifter #(.TILE_WIDTH(16)) bg_shifter(
        .clk, .ce,

        .tap(bg_xcnt[bg_index][5:0]),
        .dot_out(bg_dot[bg_index]),

        .load(bg_load[bg_index]),
        .load_data(rom_data_deswizzle),
        .load_flip(bg_attrib[bg_index][30]),
        .load_color(bg_attrib[bg_index][23:16]),
        .load_index(bg_xtile2[bg_index][1:0])
    );
end
endgenerate


reg [15:0] fg0_attrib;
reg [31:0] fg0_gfx;

reg fg0_load;
wire [11:0] fg0_dot;

tc0480scp_shifter fg0_shifter(
    .clk, .ce,

    .tap(fg0_xcnt[4:0]),
    .dot_out(fg0_dot),

    .load(fg0_load),
    .load_data(fg0_gfx),
    .load_flip(fg0_attrib[14]),
    .load_color({2'b0, fg0_attrib[13:8]}),
    .load_index(fg0_xtile[1:0])
);

wire [1:0] bg_idx0 = (ctrl_prio[1:0] + 2'd0) ^ ~{2{ctrl_prio[2]}};
wire [1:0] bg_idx1 = (ctrl_prio[1:0] + 2'd1) ^ ~{2{ctrl_prio[2]}};
wire [1:0] bg_idx2 = (ctrl_prio[1:0] + 2'd2) ^ ~{2{ctrl_prio[2]}};
wire [1:0] bg_idx3 = (ctrl_prio[1:0] + 2'd3) ^ ~{2{ctrl_prio[2]}};

logic [15:0] bg_prio_dot[4];

always_comb begin
    bg_prio_dot[bg_idx0] = { 4'b0010, bg_dot[0] };
    bg_prio_dot[bg_idx1] = { 4'b0100, bg_dot[1] };
    bg_prio_dot[bg_idx2] = { 4'b0110, bg_dot[2] };
    bg_prio_dot[bg_idx3] = { 4'b1000, bg_dot[3] };
end

assign SD = |fg0_dot[3:0] ? { 4'b1010, fg0_dot } :
            |bg_prio_dot[0][3:0] ? bg_prio_dot[0] :
            |bg_prio_dot[1][3:0] ? bg_prio_dot[1] :
            |bg_prio_dot[2][3:0] ? bg_prio_dot[2] :
            bg_prio_dot[3];

reg dtack_n;
reg prev_cs_n;

reg ram_pending = 0;
reg ram_access = 0;
reg [15:0] ram_addr;


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


    if (access_cycle == BG3_ATTRIB1) begin
        next_access_cycle = CPU_ACCESS_0;
    end else begin
        next_access_cycle = access_cycle + 5'd1;
    end

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
        FG0_ATTRIB_1: begin
            ram_addr = 16'hc000 + { 3'b0, fg0_ycnt[8:3], fg0_xtile[5:0], 1'b0 };
        end

        BG0_ATTRIB0: ram_addr = 16'h0000 + { 4'b0, bg_ycnt[0][8:4], bg_xtile[0][4:0], 1'b0, 1'b0 };
        BG0_ATTRIB1: ram_addr = 16'h0000 + { 4'b0, bg_ycnt[0][8:4], bg_xtile[0][4:0], 1'b1, 1'b0 };
        BG1_ATTRIB0: ram_addr = 16'h1000 + { 4'b0, bg_ycnt[1][8:4], bg_xtile[1][4:0], 1'b0, 1'b0 };
        BG1_ATTRIB1: ram_addr = 16'h1000 + { 4'b0, bg_ycnt[1][8:4], bg_xtile[1][4:0], 1'b1, 1'b0 };
        BG2_ATTRIB0: ram_addr = 16'h2000 + { 4'b0, bg_ycnt[2][8:4], bg_xtile[2][4:0], 1'b0, 1'b0 };
        BG2_ATTRIB1: ram_addr = 16'h2000 + { 4'b0, bg_ycnt[2][8:4], bg_xtile[2][4:0], 1'b1, 1'b0 };
        BG3_ATTRIB0: ram_addr = 16'h3000 + { 4'b0, bg_ycnt[3][8:4], bg_xtile[3][4:0], 1'b0, 1'b0 };
        BG3_ATTRIB1: ram_addr = 16'h3000 + { 4'b0, bg_ycnt[3][8:4], bg_xtile[3][4:0], 1'b1, 1'b0 };


        FG0_GFX0_0,
        FG0_GFX0_1: ram_addr = 16'he000 + { 3'b0, fg0_attrib[7:0], fg0_ycnt[2:0] ^ {3{fg0_attrib[15]}}, 1'b0, 1'b0 };
        FG0_GFX1_0,
        FG0_GFX1_1: ram_addr = 16'he000 + { 3'b0, fg0_attrib[7:0], fg0_ycnt[2:0] ^ {3{fg0_attrib[15]}}, 1'b1, 1'b0 };


    endcase
end

reg prev_hld_n, prev_vld_n;
reg end_line;

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

        prev_hld_n <= OUHLDn;
        prev_vld_n <= OUVLDn;

        line_strobe <= 0;
        end_line <= prev_hld_n & ~OUHLDn;
        frame_strobe <= prev_vld_n & ~OUVLDn;
        fg0_load <= 0;

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

            BG0_ATTRIB0: bg_attrib[0][31:16] <= RADin;
            BG0_ATTRIB1: begin
                bg_attrib[0][15:0]  <= RADin;
                bg_req[0] <= ~bg_req[0];
                bg_xtile2[0] <= bg_xtile[0];
            end

            BG1_ATTRIB0: bg_attrib[1][31:16] <= RADin;
            BG1_ATTRIB1: begin
                bg_attrib[1][15:0]  <= RADin;
                bg_req[1] <= ~bg_req[1];
                bg_xtile2[1] <= bg_xtile[1];
            end

            BG2_ATTRIB0: bg_attrib[2][31:16] <= RADin;
            BG2_ATTRIB1: begin
                bg_attrib[2][15:0]  <= RADin;
                bg_req[2] <= ~bg_req[2];
                bg_xtile2[2] <= bg_xtile[2];
            end

            BG3_ATTRIB0: bg_attrib[3][31:16] <= RADin;
            BG3_ATTRIB1: begin
                bg_attrib[3][15:0]  <= RADin;
                bg_req[3] <= ~bg_req[3];
                bg_xtile2[3] <= bg_xtile[3];
            end

            FG0_ATTRIB_0,
            FG0_ATTRIB_1: fg0_attrib <= RADin;

            FG0_GFX0_0,
            FG0_GFX0_1: fg0_gfx[15:0] <= RADin;

            FG0_GFX1_0,
            FG0_GFX1_1: begin
                fg0_gfx[31:16] <= RADin;
                fg0_load <= 1;
            end

            BG3_ROW_SCROLL2: begin
                line_strobe <= 1;
            end

            default: begin
            end
        endcase

        if (end_line) begin
            access_cycle <= WAIT0;
        end else begin
            access_cycle <= next_access_cycle;
            case(next_access_cycle)
                CPU_ACCESS_0,
                CPU_ACCESS_1: begin
                    ram_access <= ram_pending;
                end

                default: begin
                end
            endcase
        end
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

reg req_active = 0;
reg [1:0] req_index;
reg bg_req[4];
reg bg_ack[4];
reg bg_load[4];

always_ff @(posedge clk) begin
    bg_load[0] <= 0;
    bg_load[1] <= 0;
    bg_load[2] <= 0;
    bg_load[3] <= 0;

    if (req_active) begin
        if (rom_req == rom_ack) begin
            bg_load[req_index] <= 1;
            rom_data_reg <= rom_data;
            bg_ack[req_index] <= bg_req[req_index];
            req_active <= 0;
        end
    end else begin
        int i;
        for( i = 0; i < 4; i = i + 1 ) begin
            if (bg_req[i] != bg_ack[i]) begin
                if (|bg_attrib[i][14:0]) begin
                    rom_address <= {bg_attrib[i][15:0], bg_ycnt[i][3:0] ^ {4{bg_attrib[i][31]}}, 3'b0 };
                    rom_req <= ~rom_req;
                    req_active <= 1;
                    req_index <= 2'(i);
                end else begin
                    bg_ack[i] <= bg_req[i];
                    bg_load[i] <= 1;
                    rom_data_reg <= 64'd0;
                end
            end
        end
    end
end

endmodule

