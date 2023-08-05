// Copyright 2022 ETH Zurich and University of Bologna and Polytechnique Montreal.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: MohammadHossein AskariHemmat <m.h.askari.hemmat@gmail.com>
// Description:
// Ara's FPGA based SoC, containing:
//       - ara_soc:
//          - ara
//          - L2 Cache
//          - cva6/ariane
//      - peripherals:
//          - uart (wip)
//          - jtag (wip)

module xilinx_ara_soc import axi_pkg::*; import ara_pkg::*; #(
    // Number of parallel vector lanes.
    parameter  int           unsigned NrLanes      = 2,
    // Support for floating-point data types
    parameter  fpu_support_e          FPUSupport   = FPUSupportHalfSingleDouble,
    // External support for vfrec7, vfrsqrt7
    parameter  fpext_support_e        FPExtSupport = FPExtSupportEnable,
    // Support for fixed-point data types
    parameter  fixpt_support_e        FixPtSupport = FixedPointEnable,
    // AXI Interface
    parameter  int           unsigned AxiDataWidth = 32*NrLanes,
    parameter  int           unsigned AxiAddrWidth = 64,
    parameter  int           unsigned AxiUserWidth = 1,
    parameter  int           unsigned AxiIdWidth   = 5,
    // Main memory
    parameter  int           unsigned L2NumWords   = 2**15,
    // Dependant parameters. DO NOT CHANGE!
    localparam type                   axi_data_t   = logic [AxiDataWidth-1:0],
    localparam type                   axi_strb_t   = logic [AxiDataWidth/8-1:0],
    localparam type                   axi_addr_t   = logic [AxiAddrWidth-1:0],
    localparam type                   axi_user_t   = logic [AxiUserWidth-1:0],
    localparam type                   axi_id_t     = logic [AxiIdWidth-1:0]
  ) (
    // Scan chain
    // UART
    input  logic       rx_i,
    output logic       tx_o,

    // ddr3
    input  logic         sys_clk_n,
    input  logic         sys_clk_p,
    input  logic         cpu_resetn  ,
    inout  wire  [31:0]  ddr3_dq     ,
    inout  wire  [ 3:0]  ddr3_dqs_n  ,
    inout  wire  [ 3:0]  ddr3_dqs_p  ,
    output logic [14:0]  ddr3_addr   ,
    output logic [ 2:0]  ddr3_ba     ,
    output logic         ddr3_ras_n  ,
    output logic         ddr3_cas_n  ,
    output logic         ddr3_we_n   ,
    output logic         ddr3_reset_n,
    output logic [ 0:0]  ddr3_ck_p   ,
    output logic [ 0:0]  ddr3_ck_n   ,
    output logic [ 0:0]  ddr3_cke    ,
    output logic [ 0:0]  ddr3_cs_n   ,
    output logic [ 3:0]  ddr3_dm     ,
    output logic [ 0:0]  ddr3_odt    ,

    // SPI
    output logic        spi_mosi    ,
    input  logic        spi_miso    ,
    output logic        spi_ss      ,
    output logic        spi_clk_o   
    );
  /****************************
   *  CLK and RST Generation  *
   ****************************/
  logic test_en = 1'b0;
  logic ddr3_clock_out, clk_i, locked;
  logic ddr3_sync_rst, rst_ni, rst_i;
  xlnx_clk_gen wiz (
    // Clock out ports
    .clk_out1(clk_i),     // output clk_i 100MHz
    // Status and control signals
    .resetn(cpu_resetn), // input resetn
    .locked(locked),       // output locked
   // Clock in ports
    .clk_in1(ddr3_clock_out)    // input clk 200MHz
  );
  rstgen i_rstgen_main (
    .clk_i        ( clk_i                     ),
    .rst_ni       ( locked & (~ddr3_sync_rst) ),
    .test_mode_i  ( test_en                   ),
    .rst_no       ( rst_ni                    ),
    .init_no      (                           ) // keep open
  );
  assign rst_i = ~rst_ni;

  /*************
   *  Signals  *
   *************/

  /* CLINT */
  logic rtc; // real time clock used by CLINT to increase `mtime` register

  /* UART */
  logic        uart_penable;
  logic        uart_pwrite;
  logic [31:0] uart_paddr;
  logic        uart_psel;
  logic [31:0] uart_pwdata;
  logic [31:0] uart_prdata;
  logic        uart_pready;
  logic        uart_pslverr;

  /* Memory Map */
  localparam logic[63:0] ROMLength      = 64'h10000;
  localparam logic[63:0] CLINTLength    = 64'hC0000;
  localparam logic[63:0] PLICLength     = 64'h3FF_FFFF;
  localparam logic[63:0] UARTLength     = 64'h1000;
  // localparam logic[63:0] TimerLength    = 64'h1000;
  localparam logic[63:0] SPILength      = 64'h800000;
  localparam logic[63:0] DRAMLength     = 64'h40000000; // 1GByte of DDR (split between two chips on Genesys2)

  typedef enum int unsigned {
    ROM = 0,
    CLINT  = 1,
    PLIC  = 2,
    UART = 3,    // INT0
    SPI = 4,     // INT1
    DRAM = 5 
  } axi_slaves_e;
  localparam NrAXIMasters = 1; // Actually masters, but slaves on the crossbar
  localparam NrAXISlaves = DRAM + 1;

  typedef enum logic [63:0] {
    ROMBase      = 64'h0001_0000,
    CLINTBase    = 64'h0200_0000,
    PLICBase     = 64'h0C00_0000,
    UARTBase     = 64'h1000_0000,
    //TimerBase    = 64'h1800_0000,
    SPIBase      = 64'h2000_0000,
    DRAMBase     = 64'h8000_0000
  } soc_bus_start_e;

  /* Ara System */
  // Ariane's AXI port data width
  localparam AxiNarrowDataWidth = 64;
  localparam AxiNarrowStrbWidth = AxiNarrowDataWidth / 8;
  // Ara's AXI port data width
  localparam AxiWideDataWidth   = AxiDataWidth;
  localparam AXiWideStrbWidth   = AxiWideDataWidth / 8;

  localparam AxiSocIdWidth  = AxiIdWidth - $clog2(NrAXIMasters);
  localparam AxiCoreIdWidth = AxiSocIdWidth - 1;

  // Internal types
  typedef logic [AxiNarrowDataWidth-1:0] axi_narrow_data_t;
  typedef logic [AxiNarrowStrbWidth-1:0] axi_narrow_strb_t;
  typedef logic [AxiSocIdWidth-1:0] axi_soc_id_t;
  typedef logic [AxiCoreIdWidth-1:0] axi_core_id_t;

  // AXI Typedefs
  // Here I swap the axi id width of system bus and soc bus. Previous settings is wrong, I think.
  `AXI_TYPEDEF_ALL(system, axi_addr_t, axi_soc_id_t, axi_data_t, axi_strb_t, axi_user_t)
  `AXI_TYPEDEF_ALL(ara_axi, axi_addr_t, axi_core_id_t, axi_data_t, axi_strb_t, axi_user_t)
  `AXI_TYPEDEF_ALL(ariane_axi, axi_addr_t, axi_core_id_t, axi_narrow_data_t, axi_narrow_strb_t,
    axi_user_t)
  `AXI_TYPEDEF_ALL(soc_narrow, axi_addr_t, axi_id_t, axi_narrow_data_t, axi_narrow_strb_t,
    axi_user_t)
  `AXI_TYPEDEF_ALL(soc_wide, axi_addr_t, axi_id_t, axi_data_t, axi_strb_t, axi_user_t)
  `AXI_LITE_TYPEDEF_ALL(soc_narrow_lite, axi_addr_t, axi_narrow_data_t, axi_narrow_strb_t)

  // Ara system interface
  system_req_t  system_axi_req;
  system_resp_t system_axi_resp;

  // Ariane config, TODO
  localparam ariane_pkg::ariane_cfg_t ArianeAraConfig = '{
    RASDepth             : 2,
    BTBEntries           : 32,
    BHTEntries           : 128,
    // idempotent region
    NrNonIdempotentRules : 2,
    NonIdempotentAddrBase: {64'b0, 64'b0},
    NonIdempotentLength  : {64'b0, 64'b0},
    NrExecuteRegionRules : 2,
    //                      DRAM,       Boot ROM,   Debug Module
    ExecuteRegionAddrBase: {DRAMBase, ROMBase},
    ExecuteRegionLength  : {DRAMLength, ROMLength},
    // cached region
    NrCachedRegionRules  : 1,
    CachedRegionAddrBase : {DRAMBase},
    CachedRegionLength   : {DRAMLength},
    //  cache config
    Axi64BitCompliant    : 1'b1,
    SwapEndianess        : 1'b0,
    // debug
    DmBaseAddress        : 64'h0,
    NrPMPEntries         : 0
  };

  /* AXI Crossbar */
  axi_pkg::xbar_rule_64_t [NrAXISlaves-1:0] routing_rules;
  assign routing_rules = '{
    '{idx: ROM, start_addr: ROMBase, end_addr: ROMBase + ROMLength},
    '{idx: CLINT, start_addr: CLINTBase, end_addr: CLINTBase + CLINTLength},
    '{idx: PLIC, start_addr: PLICBase, end_addr: PLICBase + PLICLength},
    '{idx: UART, start_addr: UARTBase, end_addr: UARTBase + UARTLength},
    '{idx: SPI, start_addr: SPIBase, end_addr: SPIBase + SPILength},
    '{idx: DRAM, start_addr: DRAMBase, end_addr: DRAMBase + DRAMLength}
  };

  localparam axi_pkg::xbar_cfg_t XBarCfg = '{
    NoSlvPorts        : NrAXIMasters,
    NoMstPorts        : NrAXISlaves,
    MaxMstTrans       : 4,
    MaxSlvTrans       : 4,
    FallThrough       : 1'b0,
    LatencyMode       : axi_pkg::CUT_MST_PORTS,
    AxiIdWidthSlvPorts: AxiSocIdWidth,
    AxiIdUsedSlvPorts : AxiSocIdWidth,
    UniqueIds         : 1'b0,
    AxiAddrWidth      : AxiAddrWidth,
    AxiDataWidth      : AxiWideDataWidth,
    NoAddrRules       : NrAXISlaves
  };

  /* Peripheral bus */
  soc_wide_req_t    [NrAXISlaves-1:0] periph_wide_axi_req;
  soc_wide_resp_t   [NrAXISlaves-1:0] periph_wide_axi_resp;
  soc_narrow_req_t  [NrAXISlaves-1:0] periph_narrow_axi_req;
  soc_narrow_resp_t [NrAXISlaves-1:0] periph_narrow_axi_resp;

  /* Ariane irq */
  logic [1:0] ariane_irq;
  logic       ariane_ipi;
  logic       ariane_timer_irq;
  logic       ariane_debug_req = '0;

  /* PLIC */
  // PLIC use 32bits data and addr
  localparam int unsigned PLICAddrWidth = 32;
  localparam int unsigned PLICDataWidth = 32;
  // M-Mode Hart, S-Mode Hart
  localparam int unsigned NumTargets = 2;
  // Uart, SPI, reserved
  localparam int unsigned NumSources = 4;
  localparam int unsigned MaxPriority = 7;

  REG_BUS #(
        .ADDR_WIDTH ( PLICAddrWidth ),
        .DATA_WIDTH ( PLICDataWidth )
  ) reg_bus (clk_i);

  logic [NumSources-1:0]     irq_sources;
  logic                      plic_penable;
  logic                      plic_pwrite;
  logic [PLICAddrWidth-1:0]  plic_paddr;
  logic                      plic_psel;
  logic [PLICDataWidth-1:0]  plic_pwdata;
  logic [PLICDataWidth-1:0]  plic_prdata;
  logic                      plic_pready;
  logic                      plic_pslverr;

  reg_intf::reg_intf_resp_d32 plic_resp;
  reg_intf::reg_intf_req_a32_d32 plic_req;

  /* ROM */
  logic                          rom_req;
  logic [AxiAddrWidth-1:0]       rom_addr;
  logic [AxiNarrowDataWidth-1:0] rom_rdata;

  /* DDR3 */
  logic [AxiIdWidth-1:0]           ddr3_axi_awid;
  logic [AxiAddrWidth-1:0]         ddr3_axi_awaddr;
  logic [7:0]                      ddr3_axi_awlen;
  logic [2:0]                      ddr3_axi_awsize;
  logic [1:0]                      ddr3_axi_awburst;
  logic [0:0]                      ddr3_axi_awlock;
  logic [3:0]                      ddr3_axi_awcache;
  logic [2:0]                      ddr3_axi_awprot;
  logic [3:0]                      ddr3_axi_awregion;
  logic [3:0]                      ddr3_axi_awqos;
  logic                            ddr3_axi_awvalid;
  logic                            ddr3_axi_awready;
  logic [AxiNarrowDataWidth-1:0]   ddr3_axi_wdata;
  logic [AxiNarrowDataWidth/8-1:0] ddr3_axi_wstrb;
  logic                            ddr3_axi_wlast;
  logic                            ddr3_axi_wvalid;
  logic                            ddr3_axi_wready;
  logic [AxiIdWidth-1:0]           ddr3_axi_bid;
  logic [1:0]                      ddr3_axi_bresp;
  logic                            ddr3_axi_bvalid;
  logic                            ddr3_axi_bready;
  logic [AxiIdWidth-1:0]           ddr3_axi_arid;
  logic [AxiAddrWidth-1:0]         ddr3_axi_araddr;
  logic [7:0]                      ddr3_axi_arlen;
  logic [2:0]                      ddr3_axi_arsize;
  logic [1:0]                      ddr3_axi_arburst;
  logic [0:0]                      ddr3_axi_arlock;
  logic [3:0]                      ddr3_axi_arcache;
  logic [2:0]                      ddr3_axi_arprot;
  logic [3:0]                      ddr3_axi_arregion;
  logic [3:0]                      ddr3_axi_arqos;
  logic                            ddr3_axi_arvalid;
  logic                            ddr3_axi_arready;
  logic [AxiIdWidth-1:0]           ddr3_axi_rid;
  logic [AxiNarrowDataWidth-1:0]   ddr3_axi_rdata;
  logic [1:0]                      ddr3_axi_rresp;
  logic                            ddr3_axi_rlast;
  logic                            ddr3_axi_rvalid;
  logic                            ddr3_axi_rready;

  //////////////////////
  //   Ara System     //
  //////////////////////

  ara_system_with_intr #(
    .NrLanes           (NrLanes              ),
    .FPUSupport        (FPUSupport           ),
    .FPExtSupport      (FPExtSupport         ),
    .FixPtSupport      (FixPtSupport         ),
    .ArianeCfg         (ArianeAraConfig      ),
    .AxiAddrWidth      (AxiAddrWidth         ),
    .AxiIdWidth        (AxiCoreIdWidth       ),
    .AxiNarrowDataWidth(AxiNarrowDataWidth   ),
    .AxiWideDataWidth  (AxiWideDataWidth     ),
    .ara_axi_ar_t      (ara_axi_ar_chan_t    ),
    .ara_axi_aw_t      (ara_axi_aw_chan_t    ),
    .ara_axi_b_t       (ara_axi_b_chan_t     ),
    .ara_axi_r_t       (ara_axi_r_chan_t     ),
    .ara_axi_w_t       (ara_axi_w_chan_t     ),
    .ara_axi_req_t     (ara_axi_req_t        ),
    .ara_axi_resp_t    (ara_axi_resp_t       ),
    .ariane_axi_ar_t   (ariane_axi_ar_chan_t ),
    .ariane_axi_aw_t   (ariane_axi_aw_chan_t ),
    .ariane_axi_b_t    (ariane_axi_b_chan_t  ),
    .ariane_axi_r_t    (ariane_axi_r_chan_t  ),
    .ariane_axi_w_t    (ariane_axi_w_chan_t  ),
    .ariane_axi_req_t  (ariane_axi_req_t     ),
    .ariane_axi_resp_t (ariane_axi_resp_t    ),
    .system_axi_ar_t   (system_ar_chan_t     ),
    .system_axi_aw_t   (system_aw_chan_t     ),
    .system_axi_b_t    (system_b_chan_t      ),
    .system_axi_r_t    (system_r_chan_t      ),
    .system_axi_w_t    (system_w_chan_t      ),
    .system_axi_req_t  (system_req_t         ),
    .system_axi_resp_t (system_resp_t        ))
  i_system (
    .clk_i             (clk_i              ),
    .rst_ni            (rst_ni             ),
    .boot_addr_i       (ROMBase            ), // start fetching from ROM
    .hart_id_i         (hart_id            ),
    .scan_enable_i     (1'b0               ),
    .scan_data_i       (1'b0               ),
    .scan_data_o       (/* Unconnected */  ),
    .axi_req_o         (system_axi_req     ),
    .axi_resp_i        (system_axi_resp    ),
    .ariane_irq_i      (ariane_irq         ),
    .ariane_ipi_i      (ariane_ipi         ),
    .ariane_time_irq_i (ariane_timer_irq   ),
    .ariane_debug_req_i(ariane_debug_req   )
  );

  ////////////////
  //  Crossbar  //
  ////////////////

  axi_xbar #(
    .Cfg          (XBarCfg                ),
    .slv_aw_chan_t(system_aw_chan_t       ),
    .mst_aw_chan_t(soc_wide_aw_chan_t     ),
    .w_chan_t     (system_w_chan_t        ),
    .slv_b_chan_t (system_b_chan_t        ),
    .mst_b_chan_t (soc_wide_b_chan_t      ),
    .slv_ar_chan_t(system_ar_chan_t       ),
    .mst_ar_chan_t(soc_wide_ar_chan_t     ),
    .slv_r_chan_t (system_r_chan_t        ),
    .mst_r_chan_t (soc_wide_r_chan_t      ),
    .slv_req_t    (system_req_t           ),
    .slv_resp_t   (system_resp_t          ),
    .mst_req_t    (soc_wide_req_t         ),
    .mst_resp_t   (soc_wide_resp_t        ),
    .rule_t       (axi_pkg::xbar_rule_64_t)
  ) i_soc_xbar (
    .clk_i                (clk_i               ),
    .rst_ni               (rst_ni              ),
    .test_i               (1'b0                ),
    .slv_ports_req_i      (system_axi_req      ),
    .slv_ports_resp_o     (system_axi_resp     ),
    .mst_ports_req_o      (periph_wide_axi_req ),
    .mst_ports_resp_i     (periph_wide_axi_resp),
    .addr_map_i           (routing_rules       ),
    .en_default_mst_port_i('0                  ),
    .default_mst_port_i   ('0                  )
  );

  //////////////////////
  //  Peripherals     //
  //////////////////////

  //////////////////////
  //       UART       //
  //////////////////////

  axi2apb_64_32 #(
    .AXI4_ADDRESS_WIDTH(AxiAddrWidth      ),
    .AXI4_RDATA_WIDTH  (AxiNarrowDataWidth),
    .AXI4_WDATA_WIDTH  (AxiNarrowDataWidth),
    .AXI4_ID_WIDTH     (AxiIdWidth        ),
    .AXI4_USER_WIDTH   (AxiUserWidth      ),
    .BUFF_DEPTH_SLAVE  (2                 ),
    .APB_ADDR_WIDTH    (32                )
  ) i_axi2apb_64_32_uart (
    .ACLK      (clk_i                                ),
    .ARESETn   (rst_ni                               ),
    .test_en_i (test_en                              ),
    .AWID_i    (periph_narrow_axi_req[UART].aw.id    ),
    .AWADDR_i  (periph_narrow_axi_req[UART].aw.addr  ),
    .AWLEN_i   (periph_narrow_axi_req[UART].aw.len   ),
    .AWSIZE_i  (periph_narrow_axi_req[UART].aw.size  ),
    .AWBURST_i (periph_narrow_axi_req[UART].aw.burst ),
    .AWLOCK_i  (periph_narrow_axi_req[UART].aw.lock  ),
    .AWCACHE_i (periph_narrow_axi_req[UART].aw.cache ),
    .AWPROT_i  (periph_narrow_axi_req[UART].aw.prot  ),
    .AWREGION_i(periph_narrow_axi_req[UART].aw.region),
    .AWUSER_i  (periph_narrow_axi_req[UART].aw.user  ),
    .AWQOS_i   (periph_narrow_axi_req[UART].aw.qos   ),
    .AWVALID_i (periph_narrow_axi_req[UART].aw_valid ),
    .AWREADY_o (periph_narrow_axi_resp[UART].aw_ready),
    .WDATA_i   (periph_narrow_axi_req[UART].w.data   ),
    .WSTRB_i   (periph_narrow_axi_req[UART].w.strb   ),
    .WLAST_i   (periph_narrow_axi_req[UART].w.last   ),
    .WUSER_i   (periph_narrow_axi_req[UART].w.user   ),
    .WVALID_i  (periph_narrow_axi_req[UART].w_valid  ),
    .WREADY_o  (periph_narrow_axi_resp[UART].w_ready ),
    .BID_o     (periph_narrow_axi_resp[UART].b.id    ),
    .BRESP_o   (periph_narrow_axi_resp[UART].b.resp  ),
    .BVALID_o  (periph_narrow_axi_resp[UART].b_valid ),
    .BUSER_o   (periph_narrow_axi_resp[UART].b.user  ),
    .BREADY_i  (periph_narrow_axi_req[UART].b_ready  ),
    .ARID_i    (periph_narrow_axi_req[UART].ar.id    ),
    .ARADDR_i  (periph_narrow_axi_req[UART].ar.addr  ),
    .ARLEN_i   (periph_narrow_axi_req[UART].ar.len   ),
    .ARSIZE_i  (periph_narrow_axi_req[UART].ar.size  ),
    .ARBURST_i (periph_narrow_axi_req[UART].ar.burst ),
    .ARLOCK_i  (periph_narrow_axi_req[UART].ar.lock  ),
    .ARCACHE_i (periph_narrow_axi_req[UART].ar.cache ),
    .ARPROT_i  (periph_narrow_axi_req[UART].ar.prot  ),
    .ARREGION_i(periph_narrow_axi_req[UART].ar.region),
    .ARUSER_i  (periph_narrow_axi_req[UART].ar.user  ),
    .ARQOS_i   (periph_narrow_axi_req[UART].ar.qos   ),
    .ARVALID_i (periph_narrow_axi_req[UART].ar_valid ),
    .ARREADY_o (periph_narrow_axi_resp[UART].ar_ready),
    .RID_o     (periph_narrow_axi_resp[UART].r.id    ),
    .RDATA_o   (periph_narrow_axi_resp[UART].r.data  ),
    .RRESP_o   (periph_narrow_axi_resp[UART].r.resp  ),
    .RLAST_o   (periph_narrow_axi_resp[UART].r.last  ),
    .RUSER_o   (periph_narrow_axi_resp[UART].r.user  ),
    .RVALID_o  (periph_narrow_axi_resp[UART].r_valid ),
    .RREADY_i  (periph_narrow_axi_req[UART].r_ready  ),
    .PENABLE   (uart_penable                         ),
    .PWRITE    (uart_pwrite                          ),
    .PADDR     (uart_paddr                           ),
    .PSEL      (uart_psel                            ),
    .PWDATA    (uart_pwdata                          ),
    .PRDATA    (uart_prdata                          ),
    .PREADY    (uart_pready                          ),
    .PSLVERR   (uart_pslverr                         )
  );

  axi_dw_converter #(
    .AxiSlvPortDataWidth(AxiWideDataWidth     ),
    .AxiMstPortDataWidth(AxiNarrowDataWidth   ),
    .AxiAddrWidth       (AxiAddrWidth         ),
    .AxiIdWidth         (AxiIdWidth           ),
    .AxiMaxReads        (2                    ),
    .ar_chan_t          (soc_wide_ar_chan_t   ),
    .mst_r_chan_t       (soc_narrow_r_chan_t  ),
    .slv_r_chan_t       (soc_wide_r_chan_t    ),
    .aw_chan_t          (soc_narrow_aw_chan_t ),
    .b_chan_t           (soc_wide_b_chan_t    ),
    .mst_w_chan_t       (soc_narrow_w_chan_t  ),
    .slv_w_chan_t       (soc_wide_w_chan_t    ),
    .axi_mst_req_t      (soc_narrow_req_t     ),
    .axi_mst_resp_t     (soc_narrow_resp_t    ),
    .axi_slv_req_t      (soc_wide_req_t       ),
    .axi_slv_resp_t     (soc_wide_resp_t      )
  ) i_axi_slave_uart_dwc (
    .clk_i     (clk_i                       ),
    .rst_ni    (rst_ni                      ),
    .slv_req_i (periph_wide_axi_req[UART]   ),
    .slv_resp_o(periph_wide_axi_resp[UART]  ),
    .mst_req_o (periph_narrow_axi_req[UART] ),
    .mst_resp_i(periph_narrow_axi_resp[UART])
  );

  apb_uart i_apb_uart (
    .CLK     ( clk_i           ),
    .RSTN    ( rst_ni          ),
    .PSEL    ( uart_psel       ),
    .PENABLE ( uart_penable    ),
    .PWRITE  ( uart_pwrite     ),
    .PADDR   ( uart_paddr[4:2] ),
    .PWDATA  ( uart_pwdata     ),
    .PRDATA  ( uart_prdata     ),
    .PREADY  ( uart_pready     ),
    .PSLVERR ( uart_pslverr    ),
    .INT     ( irq_sources[0]  ),
    .OUT1N   (                 ),
    .OUT2N   (                 ),
    .RTSN    (                 ),
    .DTRN    (                 ),
    .CTSN    ( 1'b0            ),
    .DSRN    ( 1'b0            ),
    .DCDN    ( 1'b0            ),
    .RIN     ( 1'b0            ),
    .SIN     ( rx_i            ),
    .SOUT    ( tx_o            )
);

  //////////////////////
  //       CLINT      //
  //////////////////////

  // divide clock by two
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      rtc <= 0;
    end else begin
      rtc <= rtc ^ 1'b1;
    end
  end
  
  clint #(
    .AXI_ADDR_WIDTH ( AxiAddrWidth      ),
    .AXI_DATA_WIDTH ( AxiNarrowDataWidth),
    .AXI_ID_WIDTH   ( AxiIdWidth        ),
    .NR_CORES       ( 1                 ),
    .axi_req_t      ( soc_narrow_req_t  ),
    .axi_resp_t     ( soc_narrow_resp_t )
  ) i_clint (
    .clk_i       ( clk_i                        ),
    .rst_ni      ( rst_ni                       ),
    .testmode_i  ( test_en                      ),
    .axi_req_i   ( periph_narrow_axi_req[CLINT] ),
    .axi_resp_o  ( periph_narrow_axi_resp[CLINT]),
    .rtc_i       ( rtc                          ),
    .timer_irq_o ( ariane_timer_irq             ),
    .ipi_o       ( ariane_ipi                   )
  );

  axi_dw_converter #(
    .AxiSlvPortDataWidth(AxiWideDataWidth     ),
    .AxiMstPortDataWidth(AxiNarrowDataWidth   ),
    .AxiAddrWidth       (AxiAddrWidth         ),
    .AxiIdWidth         (AxiIdWidth           ),
    .AxiMaxReads        (2                    ),
    .ar_chan_t          (soc_wide_ar_chan_t   ),
    .mst_r_chan_t       (soc_narrow_r_chan_t  ),
    .slv_r_chan_t       (soc_wide_r_chan_t    ),
    .aw_chan_t          (soc_narrow_aw_chan_t ),
    .b_chan_t           (soc_wide_b_chan_t    ),
    .mst_w_chan_t       (soc_narrow_w_chan_t  ),
    .slv_w_chan_t       (soc_wide_w_chan_t    ),
    .axi_mst_req_t      (soc_narrow_req_t     ),
    .axi_mst_resp_t     (soc_narrow_resp_t    ),
    .axi_slv_req_t      (soc_wide_req_t       ),
    .axi_slv_resp_t     (soc_wide_resp_t      )
  ) i_axi_slave_clint_dwc (
    .clk_i     (clk_i                       ),
    .rst_ni    (rst_ni                      ),
    .slv_req_i (periph_wide_axi_req[CLINT]   ),
    .slv_resp_o(periph_wide_axi_resp[CLINT]  ),
    .mst_req_o (periph_narrow_axi_req[CLINT] ),
    .mst_resp_i(periph_narrow_axi_resp[CLINT])
  );

  //////////////////////
  //       SPI        //
  //////////////////////

  logic [31:0] s_axi_spi_awaddr;
  logic [7:0]  s_axi_spi_awlen;
  logic [2:0]  s_axi_spi_awsize;
  logic [1:0]  s_axi_spi_awburst;
  logic [0:0]  s_axi_spi_awlock;
  logic [3:0]  s_axi_spi_awcache;
  logic [2:0]  s_axi_spi_awprot;
  logic [3:0]  s_axi_spi_awregion;
  logic [3:0]  s_axi_spi_awqos;
  logic        s_axi_spi_awvalid;
  logic        s_axi_spi_awready;
  logic [31:0] s_axi_spi_wdata;
  logic [3:0]  s_axi_spi_wstrb;
  logic        s_axi_spi_wlast;
  logic        s_axi_spi_wvalid;
  logic        s_axi_spi_wready;
  logic [1:0]  s_axi_spi_bresp;
  logic        s_axi_spi_bvalid;
  logic        s_axi_spi_bready;
  logic [31:0] s_axi_spi_araddr;
  logic [7:0]  s_axi_spi_arlen;
  logic [2:0]  s_axi_spi_arsize;
  logic [1:0]  s_axi_spi_arburst;
  logic [0:0]  s_axi_spi_arlock;
  logic [3:0]  s_axi_spi_arcache;
  logic [2:0]  s_axi_spi_arprot;
  logic [3:0]  s_axi_spi_arregion;
  logic [3:0]  s_axi_spi_arqos;
  logic        s_axi_spi_arvalid;
  logic        s_axi_spi_arready;
  logic [31:0] s_axi_spi_rdata;
  logic [1:0]  s_axi_spi_rresp;
  logic        s_axi_spi_rlast;
  logic        s_axi_spi_rvalid;
  logic        s_axi_spi_rread;

  // we need 32xNrLane(currently NrLane=2) to 32bit converter
  // each time we change NrLane, IP config need to be changed accordingly? TODO
  xlnx_axi_dwidth_converter i_xlnx_axi_dwidth_converter_spi (
      .s_axi_aclk     ( clk_i              ),
      .s_axi_aresetn  ( rst_ni             ),
      .s_axi_awid     ( periph_wide_axi_req[SPI].aw.id        ),
      .s_axi_awaddr   ( periph_wide_axi_req[SPI].aw.addr[31:0]  ),
      .s_axi_awlen    ( periph_wide_axi_req[SPI].aw.len         ),
      .s_axi_awsize   ( periph_wide_axi_req[SPI].aw.size        ),
      .s_axi_awburst  ( periph_wide_axi_req[SPI].aw.burst       ),
      .s_axi_awlock   ( periph_wide_axi_req[SPI].aw.lock        ),
      .s_axi_awcache  ( periph_wide_axi_req[SPI].aw.cache       ),
      .s_axi_awprot   ( periph_wide_axi_req[SPI].aw.prot        ),
      .s_axi_awregion ( periph_wide_axi_req[SPI].aw.region      ),
      .s_axi_awqos    ( periph_wide_axi_req[SPI].aw.qos         ),
      .s_axi_awvalid  ( periph_wide_axi_req[SPI].aw_valid       ),
      .s_axi_awready  ( periph_wide_axi_resp[SPI].aw_ready      ),
      .s_axi_wdata    ( periph_wide_axi_req[SPI].w.data         ),
      .s_axi_wstrb    ( periph_wide_axi_req[SPI].w.strb         ),
      .s_axi_wlast    ( periph_wide_axi_req[SPI].w.last         ),
      .s_axi_wvalid   ( periph_wide_axi_req[SPI].w_valid        ),
      .s_axi_wready   ( periph_wide_axi_resp[SPI].w_ready       ),
      .s_axi_bid      ( periph_wide_axi_resp[SPI].b.id          ),
      .s_axi_bresp    ( periph_wide_axi_resp[SPI].b.resp        ),
      .s_axi_bvalid   ( periph_wide_axi_resp[SPI].b_valid       ),
      .s_axi_bready   ( periph_wide_axi_req[SPI].b_ready        ),
      .s_axi_arid     ( periph_wide_axi_req[SPI].ar.id          ),
      .s_axi_araddr   ( periph_wide_axi_req[SPI].ar.addr[31:0]  ),
      .s_axi_arlen    ( periph_wide_axi_req[SPI].ar.len         ),
      .s_axi_arsize   ( periph_wide_axi_req[SPI].ar.size        ),
      .s_axi_arburst  ( periph_wide_axi_req[SPI].ar.burst       ),
      .s_axi_arlock   ( periph_wide_axi_req[SPI].ar.lock        ),
      .s_axi_arcache  ( periph_wide_axi_req[SPI].ar.cache       ),
      .s_axi_arprot   ( periph_wide_axi_req[SPI].ar.prot        ),
      .s_axi_arregion ( periph_wide_axi_req[SPI].ar.region      ),
      .s_axi_arqos    ( periph_wide_axi_req[SPI].ar.qos         ),
      .s_axi_arvalid  ( periph_wide_axi_req[SPI].ar_valid       ),
      .s_axi_arready  ( periph_wide_axi_resp[SPI].ar_ready      ),
      .s_axi_rid      ( periph_wide_axi_resp[SPI].r.id          ),
      .s_axi_rdata    ( periph_wide_axi_resp[SPI].r.data        ),
      .s_axi_rresp    ( periph_wide_axi_resp[SPI].r.resp        ),
      .s_axi_rlast    ( periph_wide_axi_resp[SPI].r.last        ),
      .s_axi_rvalid   ( periph_wide_axi_resp[SPI].r_valid       ),
      .s_axi_rready   ( periph_wide_axi_req[SPI].r_ready        ),
      .m_axi_awaddr   ( s_axi_spi_awaddr   ),
      .m_axi_awlen    ( s_axi_spi_awlen    ),
      .m_axi_awsize   ( s_axi_spi_awsize   ),
      .m_axi_awburst  ( s_axi_spi_awburst  ),
      .m_axi_awlock   ( s_axi_spi_awlock   ),
      .m_axi_awcache  ( s_axi_spi_awcache  ),
      .m_axi_awprot   ( s_axi_spi_awprot   ),
      .m_axi_awregion ( s_axi_spi_awregion ),
      .m_axi_awqos    ( s_axi_spi_awqos    ),
      .m_axi_awvalid  ( s_axi_spi_awvalid  ),
      .m_axi_awready  ( s_axi_spi_awready  ),
      .m_axi_wdata    ( s_axi_spi_wdata    ),
      .m_axi_wstrb    ( s_axi_spi_wstrb    ),
      .m_axi_wlast    ( s_axi_spi_wlast    ),
      .m_axi_wvalid   ( s_axi_spi_wvalid   ),
      .m_axi_wready   ( s_axi_spi_wready   ),
      .m_axi_bresp    ( s_axi_spi_bresp    ),
      .m_axi_bvalid   ( s_axi_spi_bvalid   ),
      .m_axi_bready   ( s_axi_spi_bready   ),
      .m_axi_araddr   ( s_axi_spi_araddr   ),
      .m_axi_arlen    ( s_axi_spi_arlen    ),
      .m_axi_arsize   ( s_axi_spi_arsize   ),
      .m_axi_arburst  ( s_axi_spi_arburst  ),
      .m_axi_arlock   ( s_axi_spi_arlock   ),
      .m_axi_arcache  ( s_axi_spi_arcache  ),
      .m_axi_arprot   ( s_axi_spi_arprot   ),
      .m_axi_arregion ( s_axi_spi_arregion ),
      .m_axi_arqos    ( s_axi_spi_arqos    ),
      .m_axi_arvalid  ( s_axi_spi_arvalid  ),
      .m_axi_arready  ( s_axi_spi_arready  ),
      .m_axi_rdata    ( s_axi_spi_rdata    ),
      .m_axi_rresp    ( s_axi_spi_rresp    ),
      .m_axi_rlast    ( s_axi_spi_rlast    ),
      .m_axi_rvalid   ( s_axi_spi_rvalid   ),
      .m_axi_rready   ( s_axi_spi_rready   )
  );
  // Currently we use system clk with 100MHz frequency, and set sck frequency ratio to 8:1,
  // Which means frequency of spi sck signal is 12.5Mhz. Each time we change system clk frequency
  // by reconfiguring clock wizard ip, please change sck ratio config of axi-quad-spi ip accordingly. 
  xlnx_axi_quad_spi i_xlnx_axi_quad_spi (
      .ext_spi_clk    ( clk_i                  ),   // sck freq = ext_spi_clk / sck_ratio
      .s_axi4_aclk    ( clk_i                  ),
      .s_axi4_aresetn ( rst_ni                 ),
      .s_axi4_awaddr  ( s_axi_spi_awaddr[23:0] ),
      .s_axi4_awlen   ( s_axi_spi_awlen        ),
      .s_axi4_awsize  ( s_axi_spi_awsize       ),
      .s_axi4_awburst ( s_axi_spi_awburst      ),
      .s_axi4_awlock  ( s_axi_spi_awlock       ),
      .s_axi4_awcache ( s_axi_spi_awcache      ),
      .s_axi4_awprot  ( s_axi_spi_awprot       ),
      .s_axi4_awvalid ( s_axi_spi_awvalid      ),
      .s_axi4_awready ( s_axi_spi_awready      ),
      .s_axi4_wdata   ( s_axi_spi_wdata        ),
      .s_axi4_wstrb   ( s_axi_spi_wstrb        ),
      .s_axi4_wlast   ( s_axi_spi_wlast        ),
      .s_axi4_wvalid  ( s_axi_spi_wvalid       ),
      .s_axi4_wready  ( s_axi_spi_wready       ),
      .s_axi4_bresp   ( s_axi_spi_bresp        ),
      .s_axi4_bvalid  ( s_axi_spi_bvalid       ),
      .s_axi4_bready  ( s_axi_spi_bready       ),
      .s_axi4_araddr  ( s_axi_spi_araddr[23:0] ),
      .s_axi4_arlen   ( s_axi_spi_arlen        ),
      .s_axi4_arsize  ( s_axi_spi_arsize       ),
      .s_axi4_arburst ( s_axi_spi_arburst      ),
      .s_axi4_arlock  ( s_axi_spi_arlock       ),
      .s_axi4_arcache ( s_axi_spi_arcache      ),
      .s_axi4_arprot  ( s_axi_spi_arprot       ),
      .s_axi4_arvalid ( s_axi_spi_arvalid      ),
      .s_axi4_arready ( s_axi_spi_arready      ),
      .s_axi4_rdata   ( s_axi_spi_rdata        ),
      .s_axi4_rresp   ( s_axi_spi_rresp        ),
      .s_axi4_rlast   ( s_axi_spi_rlast        ),
      .s_axi4_rvalid  ( s_axi_spi_rvalid       ),
      .s_axi4_rready  ( s_axi_spi_rready       ),
      .io0_i          ( '0                     ),
      .io0_o          ( spi_mosi               ),
      .io0_t          (                        ),
      .io1_i          ( spi_miso               ),
      .io1_o          (                        ),
      .io1_t          (                        ),
      .ss_i           ( '0                     ),
      .ss_o           ( spi_ss                 ),
      .ss_t           (                        ),
      .sck_o          ( spi_clk_o              ),
      .sck_i          ( '0                     ),
      .sck_t          (                        ),
      .ip2intc_irpt   ( irq_sources[1]         )
  );

  //////////////////////
  //       PLIC       //
  //////////////////////

  axi2apb_64_32 #(
      .AXI4_ADDRESS_WIDTH ( AxiAddrWidth  ),
      .AXI4_RDATA_WIDTH   ( AxiNarrowDataWidth  ),
      .AXI4_WDATA_WIDTH   ( AxiNarrowDataWidth  ),
      .AXI4_ID_WIDTH      ( AxiIdWidth    ),
      .AXI4_USER_WIDTH    ( AxiUserWidth  ),
      .BUFF_DEPTH_SLAVE   ( 2             ),
      .APB_ADDR_WIDTH     ( 32            )
  ) i_axi2apb_64_32_plic (
    .ACLK      ( clk_i          ),
    .ARESETn   ( rst_ni         ),
    .test_en_i ( test_en        ),
    .AWID_i    ( periph_narrow_axi_req[PLIC].aw.id     ),
    .AWADDR_i  ( periph_narrow_axi_req[PLIC].aw.addr   ),
    .AWLEN_i   ( periph_narrow_axi_req[PLIC].aw.len    ),
    .AWSIZE_i  ( periph_narrow_axi_req[PLIC].aw.size   ),
    .AWBURST_i ( periph_narrow_axi_req[PLIC].aw.burst  ),
    .AWLOCK_i  ( periph_narrow_axi_req[PLIC].aw.lock   ),
    .AWCACHE_i ( periph_narrow_axi_req[PLIC].aw.cache  ),
    .AWPROT_i  ( periph_narrow_axi_req[PLIC].aw.prot   ),
    .AWREGION_i( periph_narrow_axi_req[PLIC].aw.region ),
    .AWUSER_i  ( periph_narrow_axi_req[PLIC].aw.user   ),
    .AWQOS_i   ( periph_narrow_axi_req[PLIC].aw.qos    ),
    .AWVALID_i ( periph_narrow_axi_req[PLIC].aw_valid  ),
    .AWREADY_o ( periph_narrow_axi_resp[PLIC].aw_ready ),
    .WDATA_i   ( periph_narrow_axi_req[PLIC].w.data    ),
    .WSTRB_i   ( periph_narrow_axi_req[PLIC].w.strb    ),
    .WLAST_i   ( periph_narrow_axi_req[PLIC].w.last    ),
    .WUSER_i   ( periph_narrow_axi_req[PLIC].w.user    ),
    .WVALID_i  ( periph_narrow_axi_req[PLIC].w_valid   ),
    .WREADY_o  ( periph_narrow_axi_resp[PLIC].w_ready  ),
    .BID_o     ( periph_narrow_axi_resp[PLIC].b.id     ),
    .BRESP_o   ( periph_narrow_axi_resp[PLIC].b.resp   ),
    .BVALID_o  ( periph_narrow_axi_resp[PLIC].b_valid  ),
    .BUSER_o   ( periph_narrow_axi_resp[PLIC].b.user   ),
    .BREADY_i  ( periph_narrow_axi_req[PLIC].b_ready   ),
    .ARID_i    ( periph_narrow_axi_req[PLIC].ar.id     ),
    .ARADDR_i  ( periph_narrow_axi_req[PLIC].ar.addr   ),
    .ARLEN_i   ( periph_narrow_axi_req[PLIC].ar.len    ),
    .ARSIZE_i  ( periph_narrow_axi_req[PLIC].ar.size   ),
    .ARBURST_i ( periph_narrow_axi_req[PLIC].ar.burst  ),
    .ARLOCK_i  ( periph_narrow_axi_req[PLIC].ar.lock   ),
    .ARCACHE_i ( periph_narrow_axi_req[PLIC].ar.cache  ),
    .ARPROT_i  ( periph_narrow_axi_req[PLIC].ar.prot   ),
    .ARREGION_i( periph_narrow_axi_req[PLIC].ar.region ),
    .ARUSER_i  ( periph_narrow_axi_req[PLIC].ar.user   ),
    .ARQOS_i   ( periph_narrow_axi_req[PLIC].ar.qos    ),
    .ARVALID_i ( periph_narrow_axi_req[PLIC].ar_valid  ),
    .ARREADY_o ( periph_narrow_axi_resp[PLIC].ar_ready ),
    .RID_o     ( periph_narrow_axi_resp[PLIC].r.id     ),
    .RDATA_o   ( periph_narrow_axi_resp[PLIC].r.data   ),
    .RRESP_o   ( periph_narrow_axi_resp[PLIC].r.resp   ),
    .RLAST_o   ( periph_narrow_axi_resp[PLIC].r.last   ),
    .RUSER_o   ( periph_narrow_axi_resp[PLIC].r.user   ),
    .RVALID_o  ( periph_narrow_axi_resp[PLIC].r_valid  ),
    .RREADY_i  ( periph_narrow_axi_req[PLIC].r_ready   ),
    .PENABLE   ( plic_penable   ),
    .PWRITE    ( plic_pwrite    ),
    .PADDR     ( plic_paddr     ),
    .PSEL      ( plic_psel      ),
    .PWDATA    ( plic_pwdata    ),
    .PRDATA    ( plic_prdata    ),
    .PREADY    ( plic_pready    ),
    .PSLVERR   ( plic_pslverr   )
  );

  axi_dw_converter #(
    .AxiSlvPortDataWidth(AxiWideDataWidth     ),
    .AxiMstPortDataWidth(AxiNarrowDataWidth   ),
    .AxiAddrWidth       (AxiAddrWidth         ),
    .AxiIdWidth         (AxiIdWidth           ),
    .AxiMaxReads        (2                    ),
    .ar_chan_t          (soc_wide_ar_chan_t   ),
    .mst_r_chan_t       (soc_narrow_r_chan_t  ),
    .slv_r_chan_t       (soc_wide_r_chan_t    ),
    .aw_chan_t          (soc_narrow_aw_chan_t ),
    .b_chan_t           (soc_wide_b_chan_t    ),
    .mst_w_chan_t       (soc_narrow_w_chan_t  ),
    .slv_w_chan_t       (soc_wide_w_chan_t    ),
    .axi_mst_req_t      (soc_narrow_req_t     ),
    .axi_mst_resp_t     (soc_narrow_resp_t    ),
    .axi_slv_req_t      (soc_wide_req_t       ),
    .axi_slv_resp_t     (soc_wide_resp_t      )
  ) i_axi_slave_plic_dwc (
    .clk_i     (clk_i                       ),
    .rst_ni    (rst_ni                      ),
    .slv_req_i (periph_wide_axi_req[PLIC]   ),
    .slv_resp_o(periph_wide_axi_resp[PLIC]  ),
    .mst_req_o (periph_narrow_axi_req[PLIC] ),
    .mst_resp_i(periph_narrow_axi_resp[PLIC])
  );

  apb_to_reg i_apb_to_reg (
      .clk_i     ( clk_i        ),
      .rst_ni    ( rst_ni       ),
      .penable_i ( plic_penable ),
      .pwrite_i  ( plic_pwrite  ),
      .paddr_i   ( plic_paddr   ),
      .psel_i    ( plic_psel    ),
      .pwdata_i  ( plic_pwdata  ),
      .prdata_o  ( plic_prdata  ),
      .pready_o  ( plic_pready  ),
      .pslverr_o ( plic_pslverr ),
      .reg_o     ( reg_bus      )
  );

  assign plic_req.addr  = reg_bus.addr;
  assign plic_req.write = reg_bus.write;
  assign plic_req.wdata = reg_bus.wdata;
  assign plic_req.wstrb = reg_bus.wstrb;
  assign plic_req.valid = reg_bus.valid;

  assign reg_bus.rdata = plic_resp.rdata;
  assign reg_bus.error = plic_resp.error;
  assign reg_bus.ready = plic_resp.ready;
  
  assign irq_sources[3:2] = 2'b00; 

  plic_top #(
    .N_SOURCE    ( NumSources  ),
    .N_TARGET    ( NumTargets  ),
    .MAX_PRIO    ( MaxPriority )
  ) i_plic (
    .clk_i,
    .rst_ni,
    .req_i         ( plic_req    ),
    .resp_o        ( plic_resp   ),
    .le_i          ( '0          ), // 0:level 1:edge
    .irq_sources_i ( irq_sources ),
    .eip_targets_o ( ariane_irq  )
  );

  //////////////////////
  //       ROM        //
  //////////////////////

  axi_dw_converter #(
    .AxiSlvPortDataWidth(AxiWideDataWidth     ),
    .AxiMstPortDataWidth(AxiNarrowDataWidth   ),
    .AxiAddrWidth       (AxiAddrWidth         ),
    .AxiIdWidth         (AxiIdWidth           ),
    .AxiMaxReads        (2                    ),
    .ar_chan_t          (soc_wide_ar_chan_t   ),
    .mst_r_chan_t       (soc_narrow_r_chan_t  ),
    .slv_r_chan_t       (soc_wide_r_chan_t    ),
    .aw_chan_t          (soc_narrow_aw_chan_t ),
    .b_chan_t           (soc_wide_b_chan_t    ),
    .mst_w_chan_t       (soc_narrow_w_chan_t  ),
    .slv_w_chan_t       (soc_wide_w_chan_t    ),
    .axi_mst_req_t      (soc_narrow_req_t     ),
    .axi_mst_resp_t     (soc_narrow_resp_t    ),
    .axi_slv_req_t      (soc_wide_req_t       ),
    .axi_slv_resp_t     (soc_wide_resp_t      )
  ) i_axi_slave_rom_dwc (
    .clk_i     (clk_i                       ),
    .rst_ni    (rst_ni                      ),
    .slv_req_i (periph_wide_axi_req[ROM]   ),
    .slv_resp_o(periph_wide_axi_resp[ROM]  ),
    .mst_req_o (periph_narrow_axi_req[ROM] ),
    .mst_resp_i(periph_narrow_axi_resp[ROM])
  );

  AXI_BUS #(
    .AXI_ADDR_WIDTH ( AxiAddrWidth       ),
    .AXI_DATA_WIDTH ( AxiNarrowDataWidth ),
    .AXI_ID_WIDTH   ( AxiIdWidth         ),
    .AXI_USER_WIDTH ( AxiUserWidth       )
  ) mem_bus();

  `AXI_ASSIGN_FROM_REQ(mem_bus, periph_narrow_axi_req[ROM]);
  `AXI_ASSIGN_TO_RESP(periph_narrow_axi_resp[ROM], mem_bus);

  axi2mem #(
    .AXI_ID_WIDTH   ( AxiIdWidth         ),
    .AXI_ADDR_WIDTH ( AxiAddrWidth       ),
    .AXI_DATA_WIDTH ( AxiNarrowDataWidth ),
    .AXI_USER_WIDTH ( AxiUserWidth       )
  ) i_axi2rom (
    .clk_i  ( clk_i       ),
    .rst_ni ( rst_ni      ),
    .slave  ( mem_bus     ),
    .req_o  ( rom_req     ),
    .we_o   (             ),
    .addr_o ( rom_addr    ),
    .be_o   (             ),
    .data_o (             ),
    .data_i ( rom_rdata   )
  );

  bootrom i_bootrom (
    .clk_i   ( clk_i     ),
    .req_i   ( rom_req   ),
    .addr_i  ( rom_addr  ),
    .rdata_o ( rom_rdata )
  );

  //////////////////////
  //       fan        //
  //////////////////////
/*
  fan_ctrl i_fan_ctrl (
    .clk_i         ( clk        ),
    .rst_ni        ( ndmreset_n ),
    .pwm_setting_i ( '1         ),
    .fan_pwm_o     ( fan_pwm    )
  );
*/
  //////////////////////
  //       DDR3       //
  //////////////////////

  axi_dw_converter #(
    .AxiSlvPortDataWidth(AxiWideDataWidth     ),
    .AxiMstPortDataWidth(AxiNarrowDataWidth   ),
    .AxiAddrWidth       (AxiAddrWidth         ),
    .AxiIdWidth         (AxiIdWidth           ),
    .AxiMaxReads        (2                    ),
    .ar_chan_t          (soc_wide_ar_chan_t   ),
    .mst_r_chan_t       (soc_narrow_r_chan_t  ),
    .slv_r_chan_t       (soc_wide_r_chan_t    ),
    .aw_chan_t          (soc_narrow_aw_chan_t ),
    .b_chan_t           (soc_wide_b_chan_t    ),
    .mst_w_chan_t       (soc_narrow_w_chan_t  ),
    .slv_w_chan_t       (soc_wide_w_chan_t    ),
    .axi_mst_req_t      (soc_narrow_req_t     ),
    .axi_mst_resp_t     (soc_narrow_resp_t    ),
    .axi_slv_req_t      (soc_wide_req_t       ),
    .axi_slv_resp_t     (soc_wide_resp_t      )
  ) i_axi_slave_dram_dwc (
    .clk_i     (clk_i                       ),
    .rst_ni    (rst_ni                      ),
    .slv_req_i (periph_wide_axi_req[DRAM]   ),
    .slv_resp_o(periph_wide_axi_resp[DRAM]  ),
    .mst_req_o (periph_narrow_axi_req[DRAM] ),
    .mst_resp_i(periph_narrow_axi_resp[DRAM])
  );

  AXI_BUS #(
    .AXI_ADDR_WIDTH ( AxiAddrWidth       ),
    .AXI_DATA_WIDTH ( AxiNarrowDataWidth ),
    .AXI_ID_WIDTH   ( AxiIdWidth         ),
    .AXI_USER_WIDTH ( AxiUserWidth       )
  ) riscv_atop();

  AXI_BUS #(
    .AXI_ADDR_WIDTH ( AxiAddrWidth       ),
    .AXI_DATA_WIDTH ( AxiNarrowDataWidth ),
    .AXI_ID_WIDTH   ( AxiIdWidth         ),
    .AXI_USER_WIDTH ( AxiUserWidth       )
  ) ddr3();

  `AXI_ASSIGN_FROM_REQ(riscv_atop, periph_narrow_axi_req[DRAM]);
  `AXI_ASSIGN_TO_RESP(periph_narrow_axi_resp[DRAM], riscv_atop);

  axi_riscv_atomics_wrap #(
    .AXI_ADDR_WIDTH     ( AxiAddrWidth       ),
    .AXI_DATA_WIDTH     ( AxiNarrowDataWidth ),
    .AXI_ID_WIDTH       ( AxiIdWidth         ),
    .AXI_USER_WIDTH     ( AxiUserWidth       ),
    .AXI_MAX_WRITE_TXNS ( 1                  ),
    .RISCV_WORD_WIDTH   ( 64                 )
  ) i_axi_riscv_atomics (
    .clk_i  ( clk_i                    ),
    .rst_ni ( rst_ni                   ),
    .slv    ( riscv_atop               ),
    .mst    ( ddr3                     )
  );

  xlnx_axi_clock_converter i_xlnx_axi_clock_converter_ddr (
    .s_axi_aclk     ( clk_i            ),
    .s_axi_aresetn  ( rst_ni           ),
    .s_axi_awid     ( ddr3.aw_id       ),
    .s_axi_awaddr   ( ddr3.aw_addr     ),
    .s_axi_awlen    ( ddr3.aw_len      ),
    .s_axi_awsize   ( ddr3.aw_size     ),
    .s_axi_awburst  ( ddr3.aw_burst    ),
    .s_axi_awlock   ( ddr3.aw_lock     ),
    .s_axi_awcache  ( ddr3.aw_cache    ),
    .s_axi_awprot   ( ddr3.aw_prot     ),
    .s_axi_awregion ( ddr3.aw_region   ),
    .s_axi_awqos    ( ddr3.aw_qos      ),
    .s_axi_awvalid  ( ddr3.aw_valid    ),
    .s_axi_awready  ( ddr3.aw_ready    ),
    .s_axi_wdata    ( ddr3.w_data      ),
    .s_axi_wstrb    ( ddr3.w_strb      ),
    .s_axi_wlast    ( ddr3.w_last      ),
    .s_axi_wvalid   ( ddr3.w_valid     ),
    .s_axi_wready   ( ddr3.w_ready     ),
    .s_axi_bid      ( ddr3.b_id        ),
    .s_axi_bresp    ( ddr3.b_resp      ),
    .s_axi_bvalid   ( ddr3.b_valid     ),
    .s_axi_bready   ( ddr3.b_ready     ),
    .s_axi_arid     ( ddr3.ar_id       ),
    .s_axi_araddr   ( ddr3.ar_addr     ),
    .s_axi_arlen    ( ddr3.ar_len      ),
    .s_axi_arsize   ( ddr3.ar_size     ),
    .s_axi_arburst  ( ddr3.ar_burst    ),
    .s_axi_arlock   ( ddr3.ar_lock     ),
    .s_axi_arcache  ( ddr3.ar_cache    ),
    .s_axi_arprot   ( ddr3.ar_prot     ),
    .s_axi_arregion ( ddr3.ar_region   ),
    .s_axi_arqos    ( ddr3.ar_qos      ),
    .s_axi_arvalid  ( ddr3.ar_valid    ),
    .s_axi_arready  ( ddr3.ar_ready    ),
    .s_axi_rid      ( ddr3.r_id        ),
    .s_axi_rdata    ( ddr3.r_data      ),
    .s_axi_rresp    ( ddr3.r_resp      ),
    .s_axi_rlast    ( ddr3.r_last      ),
    .s_axi_rvalid   ( ddr3.r_valid     ),
    .s_axi_rready   ( ddr3.r_ready     ),
    // to size converter
    .m_axi_aclk     ( ddr3_clock_out   ),
    .m_axi_aresetn  ( rst_ni           ),
    .m_axi_awid     ( ddr3_axi_awid    ),
    .m_axi_awaddr   ( ddr3_axi_awaddr  ),
    .m_axi_awlen    ( ddr3_axi_awlen   ),
    .m_axi_awsize   ( ddr3_axi_awsize  ),
    .m_axi_awburst  ( ddr3_axi_awburst ),
    .m_axi_awlock   ( ddr3_axi_awlock  ),
    .m_axi_awcache  ( ddr3_axi_awcache ),
    .m_axi_awprot   ( ddr3_axi_awprot  ),
    .m_axi_awregion ( ddr3_axi_awregion),
    .m_axi_awqos    ( ddr3_axi_awqos   ),
    .m_axi_awvalid  ( ddr3_axi_awvalid ),
    .m_axi_awready  ( ddr3_axi_awready ),
    .m_axi_wdata    ( ddr3_axi_wdata   ),
    .m_axi_wstrb    ( ddr3_axi_wstrb   ),
    .m_axi_wlast    ( ddr3_axi_wlast   ),
    .m_axi_wvalid   ( ddr3_axi_wvalid  ),
    .m_axi_wready   ( ddr3_axi_wready  ),
    .m_axi_bid      ( ddr3_axi_bid     ),
    .m_axi_bresp    ( ddr3_axi_bresp   ),
    .m_axi_bvalid   ( ddr3_axi_bvalid  ),
    .m_axi_bready   ( ddr3_axi_bready  ),
    .m_axi_arid     ( ddr3_axi_arid    ),
    .m_axi_araddr   ( ddr3_axi_araddr  ),
    .m_axi_arlen    ( ddr3_axi_arlen   ),
    .m_axi_arsize   ( ddr3_axi_arsize  ),
    .m_axi_arburst  ( ddr3_axi_arburst ),
    .m_axi_arlock   ( ddr3_axi_arlock  ),
    .m_axi_arcache  ( ddr3_axi_arcache ),
    .m_axi_arprot   ( ddr3_axi_arprot  ),
    .m_axi_arregion ( ddr3_axi_arregion),
    .m_axi_arqos    ( ddr3_axi_arqos   ),
    .m_axi_arvalid  ( ddr3_axi_arvalid ),
    .m_axi_arready  ( ddr3_axi_arready ),
    .m_axi_rid      ( ddr3_axi_rid     ),
    .m_axi_rdata    ( ddr3_axi_rdata   ),
    .m_axi_rresp    ( ddr3_axi_rresp   ),
    .m_axi_rlast    ( ddr3_axi_rlast   ),
    .m_axi_rvalid   ( ddr3_axi_rvalid  ),
    .m_axi_rready   ( ddr3_axi_rready  )
  );
  // in vivado memory interface generator, there are three important config options.
  // Clock Period, PHY to Controller Clock Ratio, Input Clock Period.
  // The first one specify clock frequency used by memory controller interacting with ddr3.
  // The second one is used to determine frequency of clock used by AXI interface of memory controller,
  // which will be outputed as `ui_clk`. And final one is input clock frequency of memory controller.
  // Recommended configs by genesys2 docs are 800MHz, 4:1, 200MHz, respectively. As the method in
  // fpga-cva6, we will use `ui_clk`, `ui_clk_sync_rst` as system clk and rst respectively.
  xlnx_mig_7_ddr3 i_ddr (
    .sys_clk_p,
    .sys_clk_n,
    .ddr3_dq,
    .ddr3_dqs_n,
    .ddr3_dqs_p,
    .ddr3_addr,
    .ddr3_ba,
    .ddr3_ras_n,
    .ddr3_cas_n,
    .ddr3_we_n,
    .ddr3_reset_n,
    .ddr3_ck_p,
    .ddr3_ck_n,
    .ddr3_cke,
    .ddr3_cs_n,
    .ddr3_dm,
    .ddr3_odt,
    .mmcm_locked     (                ), // keep open
    .app_sr_req      ( '0             ),
    .app_ref_req     ( '0             ),
    .app_zq_req      ( '0             ),
    .app_sr_active   (                ), // keep open
    .app_ref_ack     (                ), // keep open
    .app_zq_ack      (                ), // keep open
    .ui_clk          ( ddr3_clock_out ),
    .ui_clk_sync_rst ( ddr3_sync_rst  ),
    .aresetn         ( '1             ), // or set cpu_resetn?
    .s_axi_awid      ( ddr3_axi_awid  ),
    .s_axi_awaddr    ( ddr3_axi_awaddr[29:0] ),
    .s_axi_awlen    ( ddr3_axi_awlen ),
    .s_axi_awsize   ( ddr3_axi_awsize),
    .s_axi_awburst  ( ddr3_axi_awburst),
    .s_axi_awlock   ( ddr3_axi_awlock),
    .s_axi_awcache  ( ddr3_axi_awcache),
    .s_axi_awprot   ( ddr3_axi_awprot),
    .s_axi_awqos    ( ddr3_axi_awqos),
    .s_axi_awvalid  ( ddr3_axi_awvalid),
    .s_axi_awready  ( ddr3_axi_awready),
    .s_axi_wdata    ( ddr3_axi_wdata),
    .s_axi_wstrb    ( ddr3_axi_wstrb),
    .s_axi_wlast    ( ddr3_axi_wlast),
    .s_axi_wvalid   ( ddr3_axi_wvalid),
    .s_axi_wready   ( ddr3_axi_wready),
    .s_axi_bready   ( ddr3_axi_bready),
    .s_axi_bid      ( ddr3_axi_bid),
    .s_axi_bresp    ( ddr3_axi_bresp),
    .s_axi_bvalid   ( ddr3_axi_bvalid),
    .s_axi_arid     ( ddr3_axi_arid),
    .s_axi_araddr    ( ddr3_axi_araddr[29:0] ),
    .s_axi_arlen    ( ddr3_axi_arlen),
    .s_axi_arsize   ( ddr3_axi_arsize),
    .s_axi_arburst  ( ddr3_axi_arburst),
    .s_axi_arlock   ( ddr3_axi_arlock),
    .s_axi_arcache  ( ddr3_axi_arcache),
    .s_axi_arprot   ( ddr3_axi_arprot),
    .s_axi_arqos    ( ddr3_axi_arqos),
    .s_axi_arvalid  ( ddr3_axi_arvalid),
    .s_axi_arready  ( ddr3_axi_arready),
    .s_axi_rready   ( ddr3_axi_rready),
    .s_axi_rid      ( ddr3_axi_rid),
    .s_axi_rdata    ( ddr3_axi_rdata),
    .s_axi_rresp    ( ddr3_axi_rresp),
    .s_axi_rlast    ( ddr3_axi_rlast),
    .s_axi_rvalid   ( ddr3_axi_rvalid),
    .init_calib_complete (            ), // keep open
    .device_temp         (            ), // keep open
    .sys_rst             ( cpu_resetn )
);

endmodule : xilinx_ara_soc
