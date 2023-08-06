create_ip -name clk_wiz -vendor xilinx.com -library ip -version 6.0 -module_name xlnx_clk_gen
set_property -dict [list \
  CONFIG.CLKIN1_JITTER_PS {50.0} \
  CONFIG.CLKOUT1_JITTER {129.198} \
  CONFIG.CLKOUT1_PHASE_ERROR {89.971} \
  CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {50.000} \
  CONFIG.Component_Name {xlnx_clk_gen} \
  CONFIG.MMCM_CLKFBOUT_MULT_F {5.000} \
  CONFIG.MMCM_CLKIN1_PERIOD {5.000} \
  CONFIG.MMCM_CLKIN2_PERIOD {10.0} \
  CONFIG.MMCM_CLKOUT0_DIVIDE_F {20.000} \
  CONFIG.PRIM_IN_FREQ {200.000} \
  CONFIG.PRIM_SOURCE {Differential_clock_capable_pin} \
  CONFIG.RESET_PORT {resetn} \
  CONFIG.RESET_TYPE {ACTIVE_LOW} \
] [get_ips xlnx_clk_gen]