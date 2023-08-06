set_property -dict { PACKAGE_PIN R19   IOSTANDARD LVCMOS33 } [get_ports cpu_resetn]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]

set_property -dict { PACKAGE_PIN AD11  IOSTANDARD LVDS } [ get_ports sys_clk_n ]
set_property -dict { PACKAGE_PIN AD12  IOSTANDARD LVDS } [ get_ports sys_clk_p ]