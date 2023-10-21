# project root/build dir are set from Makefile
set ROOT [lindex $argv 0]
set BUILD_DIR [lindex $argv 1]

set XILINX_PART xc7k325tffg900-2
set XILINX_BOARD digilentinc.com:genesys2:part0:1.1
set project ara
# used by tc_sram_xilinx.sv

if {![file exists "ara.xpr"]} {
  create_project $project . -part $XILINX_PART
  set_property board_part $XILINX_BOARD [current_project]
  add_files -fileset constrs_1 -norecurse $ROOT/fpga/constraints/ara.xdc
  add_files -fileset constrs_1 -norecurse $ROOT/fpga/constraints/genesys2.xdc
  source $ROOT/fpga/scripts/add_source.tcl
  source $ROOT/fpga/scripts/genesys2-ip.tcl
  puts "Creating project $project"
} else {
  open_project ${project}.xpr
  puts "Opening project $project"
}

set_property XPM_LIBRARIES XPM_MEMORY [current_project]
reset_run synth_1
launch_runs synth_1 -jobs 8
wait_on_run synth_1
open_run synth_1

report_utilization -hierarchical -file $project.synth.utilization.rpt

reset_run impl_1
launch_runs impl_1 -jobs 8 -to_step write_bitstream
wait_on_run impl_1
report_utilization -hierarchical -file ${project}.impl.utilization.rpt

#write_cfgmem -format mcs -interface SPIx4 -size 256  -loadbit \
#  "up 0x0 xilinx_ara_soc.bit" -file xilinx_ara_soc.mcs -force
