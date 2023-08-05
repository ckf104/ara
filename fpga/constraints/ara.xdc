# 10ns -> 100M frequency, be consistent with clk_wiz_0 configuration
create_clock -period 10 -name clk_i [get_pins wiz/clk_i]
