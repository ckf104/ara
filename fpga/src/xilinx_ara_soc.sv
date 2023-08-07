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
    input  logic        sys_clk_n,
    input  logic        sys_clk_p,
    input  logic        cpu_resetn,
    //output logic [63:0] exit_o,
    // Scan chain
    // UART
    input  logic       rx_i,
    output logic       tx_o
    );
  logic clk_i, rst_ni, locked;
  xlnx_clk_gen wiz (
    .clk_in1_n(sys_clk_n),
    .clk_in1_p(sys_clk_p),
    .resetn(cpu_resetn),
    .clk_out1(clk_i),
    .locked(locked)
  );
  assign rst_ni = locked & cpu_resetn;

  /*************
   *  Signals  *
   *************/

  // UART
  logic        uart_penable;
  logic        uart_pwrite;
  logic [31:0] uart_paddr;
  logic        uart_psel;
  logic [31:0] uart_pwdata;
  logic [31:0] uart_prdata;
  logic        uart_pready;
  logic        uart_pslverr;

  //////////////////////
  //  Ara SoC         //
  //////////////////////

  ara_soc #(
    .NrLanes     (NrLanes      ),
    .AxiAddrWidth(AxiAddrWidth ),
    .AxiDataWidth(AxiDataWidth ),
    .AxiIdWidth  (AxiIdWidth   ),
    .AxiUserWidth(AxiUserWidth ),
    .L2NumWords  (L2NumWords   )
  ) i_ara_soc (
    .clk_i         (clk_i       ),
    .rst_ni        (rst_ni      ),
    .exit_o        (            ),
    .scan_enable_i (1'b0        ),
    .scan_data_i   (1'b0        ),
    .scan_data_o   (/* Unused */),
    // UART
    .uart_penable_o(uart_penable),
    .uart_pwrite_o (uart_pwrite ),
    .uart_paddr_o  (uart_paddr  ),
    .uart_psel_o   (uart_psel   ),
    .uart_pwdata_o (uart_pwdata ),
    .uart_prdata_i (uart_prdata ),
    .uart_pready_i (uart_pready ),
    .uart_pslverr_i(uart_pslverr)
  );
  //////////////////////
  //  Peripherals     //
  //////////////////////

  //////////////////////
  //       UART       //
  //////////////////////
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
    .INT     (                 ),
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

endmodule : xilinx_ara_soc
