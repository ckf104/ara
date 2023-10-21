# This script was generated automatically by bender.
#set ROOT "/home/ckf104/tmp/riscv-vector-ara"
#add_files -norecurse -fileset [current_fileset] [list \
#    $ROOT/hardware/deps/tech_cells_generic/src/rtl/tc_sram.sv \
#]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/tech_cells_generic/src/deprecated/cluster_clk_cells_xilinx.sv \
    $ROOT/hardware/deps/tech_cells_generic/src/deprecated/pulp_clk_cells_xilinx.sv \
    $ROOT/hardware/deps/tech_cells_generic/src/fpga/tc_clk_xilinx.sv \
    $ROOT/hardware/deps/tech_cells_generic/src/fpga/tc_sram_xilinx.sv \
]

add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/tech_cells_generic/src/deprecated/pulp_clock_gating_async.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/common_cells/src/binary_to_gray.sv \
    $ROOT/hardware/deps/common_cells/src/cb_filter_pkg.sv \
    $ROOT/hardware/deps/common_cells/src/cc_onehot.sv \
    $ROOT/hardware/deps/common_cells/src/cf_math_pkg.sv \
    $ROOT/hardware/deps/common_cells/src/clk_int_div.sv \
    $ROOT/hardware/deps/common_cells/src/delta_counter.sv \
    $ROOT/hardware/deps/common_cells/src/ecc_pkg.sv \
    $ROOT/hardware/deps/common_cells/src/edge_propagator_tx.sv \
    $ROOT/hardware/deps/common_cells/src/exp_backoff.sv \
    $ROOT/hardware/deps/common_cells/src/fifo_v3.sv \
    $ROOT/hardware/deps/common_cells/src/gray_to_binary.sv \
    $ROOT/hardware/deps/common_cells/src/isochronous_4phase_handshake.sv \
    $ROOT/hardware/deps/common_cells/src/isochronous_spill_register.sv \
    $ROOT/hardware/deps/common_cells/src/lfsr.sv \
    $ROOT/hardware/deps/common_cells/src/lfsr_16bit.sv \
    $ROOT/hardware/deps/common_cells/src/lfsr_8bit.sv \
    $ROOT/hardware/deps/common_cells/src/mv_filter.sv \
    $ROOT/hardware/deps/common_cells/src/onehot_to_bin.sv \
    $ROOT/hardware/deps/common_cells/src/plru_tree.sv \
    $ROOT/hardware/deps/common_cells/src/popcount.sv \
    $ROOT/hardware/deps/common_cells/src/rr_arb_tree.sv \
    $ROOT/hardware/deps/common_cells/src/rstgen_bypass.sv \
    $ROOT/hardware/deps/common_cells/src/serial_deglitch.sv \
    $ROOT/hardware/deps/common_cells/src/shift_reg.sv \
    $ROOT/hardware/deps/common_cells/src/spill_register_flushable.sv \
    $ROOT/hardware/deps/common_cells/src/stream_demux.sv \
    $ROOT/hardware/deps/common_cells/src/stream_filter.sv \
    $ROOT/hardware/deps/common_cells/src/stream_fork.sv \
    $ROOT/hardware/deps/common_cells/src/stream_intf.sv \
    $ROOT/hardware/deps/common_cells/src/stream_join.sv \
    $ROOT/hardware/deps/common_cells/src/stream_mux.sv \
    $ROOT/hardware/deps/common_cells/src/sub_per_hash.sv \
    $ROOT/hardware/deps/common_cells/src/sync.sv \
    $ROOT/hardware/deps/common_cells/src/sync_wedge.sv \
    $ROOT/hardware/deps/common_cells/src/unread.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_reset_ctrlr_pkg.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_2phase.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_4phase.sv \
    $ROOT/hardware/deps/common_cells/src/addr_decode.sv \
    $ROOT/hardware/deps/common_cells/src/cb_filter.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_fifo_2phase.sv \
    $ROOT/hardware/deps/common_cells/src/counter.sv \
    $ROOT/hardware/deps/common_cells/src/ecc_decode.sv \
    $ROOT/hardware/deps/common_cells/src/ecc_encode.sv \
    $ROOT/hardware/deps/common_cells/src/edge_detect.sv \
    $ROOT/hardware/deps/common_cells/src/lzc.sv \
    $ROOT/hardware/deps/common_cells/src/max_counter.sv \
    $ROOT/hardware/deps/common_cells/src/rstgen.sv \
    $ROOT/hardware/deps/common_cells/src/spill_register.sv \
    $ROOT/hardware/deps/common_cells/src/stream_delay.sv \
    $ROOT/hardware/deps/common_cells/src/stream_fifo.sv \
    $ROOT/hardware/deps/common_cells/src/stream_fork_dynamic.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_reset_ctrlr.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_fifo_gray.sv \
    $ROOT/hardware/deps/common_cells/src/fall_through_register.sv \
    $ROOT/hardware/deps/common_cells/src/id_queue.sv \
    $ROOT/hardware/deps/common_cells/src/stream_to_mem.sv \
    $ROOT/hardware/deps/common_cells/src/stream_arbiter_flushable.sv \
    $ROOT/hardware/deps/common_cells/src/stream_register.sv \
    $ROOT/hardware/deps/common_cells/src/stream_xbar.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_fifo_gray_clearable.sv \
    $ROOT/hardware/deps/common_cells/src/cdc_2phase_clearable.sv \
    $ROOT/hardware/deps/common_cells/src/stream_arbiter.sv \
    $ROOT/hardware/deps/common_cells/src/stream_omega_net.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/clock_divider_counter.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/find_first_one.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/generic_LFSR_8bit.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/generic_fifo.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/prioarbiter.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/pulp_sync.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/pulp_sync_wedge.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/rrarbiter.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/clock_divider.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/fifo_v2.sv \
    $ROOT/hardware/deps/common_cells/src/deprecated/fifo_v1.sv \
    $ROOT/hardware/deps/common_cells/src/edge_propagator_ack.sv \
    $ROOT/hardware/deps/common_cells/src/edge_propagator.sv \
    $ROOT/hardware/deps/common_cells/src/edge_propagator_rx.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/defs_div_sqrt_mvp.sv \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/iteration_div_sqrt_mvp.sv \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/control_mvp.sv \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/norm_div_sqrt_mvp.sv \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/preprocess_mvp.sv \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/nrbd_nrsc_mvp.sv \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/div_sqrt_top_mvp.sv \
    $ROOT/hardware/deps/fpu_div_sqrt_mvp/hdl/div_sqrt_mvp_wrapper.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/apb/src/apb_pkg.sv \
    $ROOT/hardware/deps/apb/src/apb_intf.sv \
    $ROOT/hardware/deps/apb/src/apb_err_slv.sv \
    $ROOT/hardware/deps/apb/src/apb_regs.sv \
    $ROOT/hardware/deps/apb/src/apb_cdc.sv \
    $ROOT/hardware/deps/apb/src/apb_demux.sv \
]
# replaced by axi_intf.sv in cva6
# $ROOT/hardware/deps/axi/src/axi_intf.sv
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/axi/src/axi_pkg.sv \
    $ROOT/hardware/deps/axi/src/axi_atop_filter.sv \
    $ROOT/hardware/deps/axi/src/axi_burst_splitter.sv \
    $ROOT/hardware/deps/axi/src/axi_cdc_dst.sv \
    $ROOT/hardware/deps/axi/src/axi_cdc_src.sv \
    $ROOT/hardware/deps/axi/src/axi_cut.sv \
    $ROOT/hardware/deps/axi/src/axi_delayer.sv \
    $ROOT/hardware/deps/axi/src/axi_demux.sv \
    $ROOT/hardware/deps/axi/src/axi_dw_downsizer.sv \
    $ROOT/hardware/deps/axi/src/axi_dw_upsizer.sv \
    $ROOT/hardware/deps/axi/src/axi_id_prepend.sv \
    $ROOT/hardware/deps/axi/src/axi_isolate.sv \
    $ROOT/hardware/deps/axi/src/axi_join.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_demux.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_join.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_mailbox.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_mux.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_regs.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_to_apb.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_to_axi.sv \
    $ROOT/hardware/deps/axi/src/axi_modify_address.sv \
    $ROOT/hardware/deps/axi/src/axi_mux.sv \
    $ROOT/hardware/deps/axi/src/axi_serializer.sv \
    $ROOT/hardware/deps/axi/src/axi_cdc.sv \
    $ROOT/hardware/deps/axi/src/axi_err_slv.sv \
    $ROOT/hardware/deps/axi/src/axi_dw_converter.sv \
    $ROOT/hardware/deps/axi/src/axi_multicut.sv \
    $ROOT/hardware/deps/axi/src/axi_to_axi_lite.sv \
    $ROOT/hardware/deps/axi/src/axi_lite_xbar.sv \
    $ROOT/hardware/deps/axi/src/axi_xbar.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/fpnew/src/fpnew_pkg.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_cast_multi.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_classifier.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_divsqrt_multi.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_fma.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_fma_multi.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_noncomp.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_opgroup_block.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_opgroup_fmt_slice.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_opgroup_multifmt_slice.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_rounding.sv \
    $ROOT/hardware/deps/fpnew/src/fpnew_top.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/apb_timer/src/timer.sv \
    $ROOT/hardware/deps/apb_timer/src/apb_timer.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/axi_slice/src/axi_single_slice.sv \
    $ROOT/hardware/deps/axi_slice/src/axi_ar_buffer.sv \
    $ROOT/hardware/deps/axi_slice/src/axi_aw_buffer.sv \
    $ROOT/hardware/deps/axi_slice/src/axi_b_buffer.sv \
    $ROOT/hardware/deps/axi_slice/src/axi_r_buffer.sv \
    $ROOT/hardware/deps/axi_slice/src/axi_slice.sv \
    $ROOT/hardware/deps/axi_slice/src/axi_w_buffer.sv \
    $ROOT/hardware/deps/axi_slice/src/axi_slice_wrap.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/axi2apb/src/axi2apb.sv \
    $ROOT/hardware/deps/axi2apb/src/axi2apb_64_32.sv \
    $ROOT/hardware/deps/axi2apb/src/axi2apb_wrap.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_res_tbl.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_amos_alu.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_amos.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_lrsc.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_atomics.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_lrsc_wrap.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_amos_wrap.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_atomics_wrap.sv \
    $ROOT/hardware/deps/axi_riscv_atomics/src/axi_riscv_atomics_structs.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/cva6/core/include/cv64a6_imafdcv_sv39_config_pkg.sv \
    $ROOT/hardware/deps/cva6/core/include/riscv_pkg.sv \
    $ROOT/hardware/deps/cva6/common/local/rvfi/rvfi_pkg.sv \
    $ROOT/hardware/deps/cva6/core/include/ariane_dm_pkg.sv \
    $ROOT/hardware/deps/cva6/core/include/ariane_pkg.sv \
    $ROOT/hardware/deps/cva6/core/mmu_sv39/tlb.sv \
    $ROOT/hardware/deps/cva6/core/mmu_sv39/mmu.sv \
    $ROOT/hardware/deps/cva6/core/mmu_sv39/ptw.sv \
    $ROOT/hardware/deps/cva6/corev_apu/tb/common/mock_uart.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/cva6/core/include/ariane_axi_pkg.sv \
    $ROOT/hardware/deps/cva6/core/include/wt_cache_pkg.sv \
    $ROOT/hardware/deps/cva6/core/include/std_cache_pkg.sv \
    $ROOT/hardware/deps/cva6/core/include/axi_intf.sv \
    $ROOT/hardware/deps/cva6/core/include/acc_pkg.sv \
    $ROOT/hardware/deps/cva6/vendor/pulp-platform/fpga-support/rtl/SyncDpRam.sv \
    $ROOT/hardware/deps/cva6/vendor/pulp-platform/fpga-support/rtl/AsyncDpRam.sv \
    $ROOT/hardware/deps/cva6/vendor/pulp-platform/fpga-support/rtl/AsyncThreePortRam.sv \
    $ROOT/hardware/deps/cva6/core/include/cvxif_pkg.sv \
    $ROOT/hardware/deps/cva6/core/cvxif_example/include/cvxif_instr_pkg.sv \
    $ROOT/hardware/deps/cva6/core/cvxif_fu.sv \
    $ROOT/hardware/deps/cva6/core/cvxif_example/cvxif_example_coprocessor.sv \
    $ROOT/hardware/deps/cva6/core/cvxif_example/instr_decoder.sv \
    $ROOT/hardware/deps/cva6/corev_apu/src/ariane.sv \
    $ROOT/hardware/deps/cva6/core/cva6.sv \
    $ROOT/hardware/deps/cva6/core/alu.sv \
    $ROOT/hardware/deps/cva6/core/fpu_wrap.sv \
    $ROOT/hardware/deps/cva6/core/branch_unit.sv \
    $ROOT/hardware/deps/cva6/core/compressed_decoder.sv \
    $ROOT/hardware/deps/cva6/core/controller.sv \
    $ROOT/hardware/deps/cva6/core/csr_buffer.sv \
    $ROOT/hardware/deps/cva6/core/csr_regfile.sv \
    $ROOT/hardware/deps/cva6/core/decoder.sv \
    $ROOT/hardware/deps/cva6/core/ex_stage.sv \
    $ROOT/hardware/deps/cva6/core/instr_realign.sv \
    $ROOT/hardware/deps/cva6/core/id_stage.sv \
    $ROOT/hardware/deps/cva6/core/issue_read_operands.sv \
    $ROOT/hardware/deps/cva6/core/acc_dispatcher.sv \
    $ROOT/hardware/deps/cva6/core/issue_stage.sv \
    $ROOT/hardware/deps/cva6/core/load_unit.sv \
    $ROOT/hardware/deps/cva6/core/load_store_unit.sv \
    $ROOT/hardware/deps/cva6/core/lsu_bypass.sv \
    $ROOT/hardware/deps/cva6/core/mult.sv \
    $ROOT/hardware/deps/cva6/core/multiplier.sv \
    $ROOT/hardware/deps/cva6/core/serdiv.sv \
    $ROOT/hardware/deps/cva6/core/perf_counters.sv \
    $ROOT/hardware/deps/cva6/core/ariane_regfile_ff.sv \
    $ROOT/hardware/deps/cva6/core/ariane_regfile_fpga.sv \
    $ROOT/hardware/deps/cva6/core/re_name.sv \
    $ROOT/hardware/deps/cva6/core/scoreboard.sv \
    $ROOT/hardware/deps/cva6/core/store_buffer.sv \
    $ROOT/hardware/deps/cva6/core/amo_buffer.sv \
    $ROOT/hardware/deps/cva6/core/store_unit.sv \
    $ROOT/hardware/deps/cva6/core/commit_stage.sv \
    $ROOT/hardware/deps/cva6/core/axi_shim.sv \
    $ROOT/hardware/deps/cva6/core/frontend/btb.sv \
    $ROOT/hardware/deps/cva6/core/frontend/bht.sv \
    $ROOT/hardware/deps/cva6/core/frontend/ras.sv \
    $ROOT/hardware/deps/cva6/core/frontend/instr_scan.sv \
    $ROOT/hardware/deps/cva6/core/frontend/instr_queue.sv \
    $ROOT/hardware/deps/cva6/core/frontend/frontend.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/wt_dcache_ctrl.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/wt_dcache_mem.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/wt_dcache_missunit.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/wt_dcache_wbuffer.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/wt_dcache.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/cva6_icache.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/wt_cache_subsystem.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/wt_axi_adapter.sv \
    $ROOT/hardware/deps/cva6/core/cache_subsystem/axi_adapter.sv \
    $ROOT/hardware/deps/cva6/core/pmp/src/pmp.sv \
    $ROOT/hardware/deps/cva6/core/pmp/src/pmp_entry.sv \
    $ROOT/hardware/deps/cva6/corev_apu/fpga/src/bootrom/bootrom_64.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/cva6/common/local/util/sram.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/cva6/vendor/pulp-platform/fpga-support/rtl/SyncSpRamBeNx64.sv \
    $ROOT/hardware/deps/cva6/common/local/util/tc_sram_fpga_wrapper.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/cva6/corev_apu/clint/axi_lite_interface.sv \
    $ROOT/hardware/deps/cva6/corev_apu/clint/clint.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/register_interface/src/reg_intf.sv \
    $ROOT/hardware/deps/register_interface/vendor/lowrisc_opentitan/src/prim_subreg_arb.sv \
    $ROOT/hardware/deps/register_interface/vendor/lowrisc_opentitan/src/prim_subreg_ext.sv \
    $ROOT/hardware/deps/register_interface/src/apb_to_reg.sv \
    $ROOT/hardware/deps/register_interface/src/axi_lite_to_reg.sv \
    $ROOT/hardware/deps/register_interface/src/axi_to_reg_v2.sv \
    $ROOT/hardware/deps/register_interface/src/periph_to_reg.sv \
    $ROOT/hardware/deps/register_interface/src/reg_cdc.sv \
    $ROOT/hardware/deps/register_interface/src/reg_cut.sv \
    $ROOT/hardware/deps/register_interface/src/reg_demux.sv \
    $ROOT/hardware/deps/register_interface/src/reg_err_slv.sv \
    $ROOT/hardware/deps/register_interface/src/reg_filter_empty_writes.sv \
    $ROOT/hardware/deps/register_interface/src/reg_mux.sv \
    $ROOT/hardware/deps/register_interface/src/reg_to_apb.sv \
    $ROOT/hardware/deps/register_interface/src/reg_to_mem.sv \
    $ROOT/hardware/deps/register_interface/src/reg_to_tlul.sv \
    $ROOT/hardware/deps/register_interface/src/reg_uniform.sv \
    $ROOT/hardware/deps/register_interface/vendor/lowrisc_opentitan/src/prim_subreg_shadow.sv \
    $ROOT/hardware/deps/register_interface/vendor/lowrisc_opentitan/src/prim_subreg.sv \
    $ROOT/hardware/deps/register_interface/src/deprecated/axi_to_reg.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/riscv_dbg/src/dm_pkg.sv \
    $ROOT/hardware/deps/riscv_dbg/debug_rom/debug_rom.sv \
    $ROOT/hardware/deps/riscv_dbg/debug_rom/debug_rom_one_scratch.sv \
    $ROOT/hardware/deps/riscv_dbg/src/dm_csrs.sv \
    $ROOT/hardware/deps/riscv_dbg/src/dm_mem.sv \
    $ROOT/hardware/deps/riscv_dbg/src/dmi_cdc.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/riscv_dbg/src/dmi_jtag_tap.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/riscv_dbg/src/dm_sba.sv \
    $ROOT/hardware/deps/riscv_dbg/src/dm_top.sv \
    $ROOT/hardware/deps/riscv_dbg/src/dmi_jtag.sv \
    $ROOT/hardware/deps/riscv_dbg/src/dm_obi_top.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/axi_mem_if/src/axi2mem.sv \
    $ROOT/hardware/deps/axi_mem_if/src/deprecated/axi_mem_if.sv \
    $ROOT/hardware/deps/axi_mem_if/src/deprecated/axi_mem_if_var_latency.sv \
    $ROOT/hardware/deps/axi_mem_if/src/deprecated/axi_mem_if_wrap.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/apb_uart/src/slib_clock_div.sv \
    $ROOT/hardware/deps/apb_uart/src/slib_counter.sv \
    $ROOT/hardware/deps/apb_uart/src/slib_edge_detect.sv \
    $ROOT/hardware/deps/apb_uart/src/slib_fifo.sv \
    $ROOT/hardware/deps/apb_uart/src/slib_input_filter.sv \
    $ROOT/hardware/deps/apb_uart/src/slib_input_sync.sv \
    $ROOT/hardware/deps/apb_uart/src/slib_mv_filter.sv \
    $ROOT/hardware/deps/apb_uart/src/uart_baudgen.sv \
    $ROOT/hardware/deps/apb_uart/src/uart_interrupt.sv \
    $ROOT/hardware/deps/apb_uart/src/uart_receiver.sv \
    $ROOT/hardware/deps/apb_uart/src/uart_transmitter.sv \
    $ROOT/hardware/deps/apb_uart/src/apb_uart.sv \
    $ROOT/hardware/deps/apb_uart/src/apb_uart_wrap.sv \
    $ROOT/hardware/deps/apb_uart/src/reg_uart_wrap.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/deps/rv_plic/rtl/rv_plic_target.sv \
    $ROOT/hardware/deps/rv_plic/rtl/rv_plic_gateway.sv \
    $ROOT/hardware/deps/rv_plic/rtl/plic_regmap.sv \
    $ROOT/hardware/deps/rv_plic/rtl/plic_top.sv \
]
add_files -norecurse -fileset [current_fileset] [list \
    $ROOT/hardware/include/rvv_pkg.sv \
    $ROOT/hardware/include/ara_pkg.sv \
    $ROOT/hardware/src/lane/rw_controller.sv \
    $ROOT/hardware/src/axi_to_mem.sv \
    $ROOT/hardware/src/ctrl_registers.sv \
    $ROOT/hardware/src/cva6_accel_first_pass_decoder.sv \
    $ROOT/hardware/src/ara_dispatcher.sv \
    $ROOT/hardware/src/ara_sequencer.sv \
    $ROOT/hardware/src/axi_inval_filter.sv \
    $ROOT/hardware/src/lane/lane_sequencer.sv \
    $ROOT/hardware/src/lane/operand_queue.sv \
    $ROOT/hardware/src/lane/operand_requester.sv \
    $ROOT/hardware/src/lane/simd_alu.sv \
    $ROOT/hardware/src/lane/simd_div.sv \
    $ROOT/hardware/src/lane/simd_mul.sv \
    $ROOT/hardware/src/lane/vector_regfile.sv \
    $ROOT/hardware/src/lane/power_gating_generic.sv \
    $ROOT/hardware/src/masku/masku.sv \
    $ROOT/hardware/src/sldu/p2_stride_gen.sv \
    $ROOT/hardware/src/sldu/sldu_op_dp.sv \
    $ROOT/hardware/src/sldu/sldu.sv \
    $ROOT/hardware/src/vlsu/addrgen.sv \
    $ROOT/hardware/src/vlsu/vldu.sv \
    $ROOT/hardware/src/vlsu/vstu.sv \
    $ROOT/hardware/src/lane/operand_queues_stage.sv \
    $ROOT/hardware/src/lane/valu.sv \
    $ROOT/hardware/src/lane/vmfpu.sv \
    $ROOT/hardware/src/lane/fixed_p_rounding.sv \
    $ROOT/hardware/src/vlsu/vlsu.sv \
    $ROOT/hardware/src/lane/vector_fus_stage.sv \
    $ROOT/hardware/src/lane/lane.sv \
    $ROOT/hardware/src/ara.sv \
    $ROOT/hardware/src/ara_system.sv \
    $ROOT/fpga/src/xilinx_ara_soc.sv \
]

set_property include_dirs [list \
    $ROOT/hardware/deps/apb/include \
    $ROOT/hardware/deps/axi/include \
    $ROOT/hardware/deps/common_cells/include \
    $ROOT/hardware/deps/cva6/common/local/util \
    $ROOT/hardware/deps/register_interface/include \
    $ROOT/hardware/include \
] [current_fileset]

set_property include_dirs [list \
    $ROOT/hardware/deps/apb/include \
    $ROOT/hardware/deps/axi/include \
    $ROOT/hardware/deps/common_cells/include \
    $ROOT/hardware/deps/cva6/common/local/util \
    $ROOT/hardware/deps/register_interface/include \
    $ROOT/hardware/include \
] [current_fileset -simset]

set_property verilog_define [list \
    ARIANE_ACCELERATOR_PORT=1 \
    NR_LANES=2 \
    TARGET_CV64A6_IMAFDCV_SV39 \
    TARGET_FPGA \
    TARGET_RTL \
    TARGET_SYNTHESIS \
    TARGET_VIVADO \
    TARGET_XILINX \
    VLEN=1024 \
] [current_fileset]

set_property verilog_define [list \
    ARIANE_ACCELERATOR_PORT=1 \
    NR_LANES=2 \
    TARGET_CV64A6_IMAFDCV_SV39 \
    TARGET_FPGA \
    TARGET_RTL \
    TARGET_SYNTHESIS \
    TARGET_VIVADO \
    TARGET_XILINX \
    VLEN=1024 \
] [current_fileset -simset]
