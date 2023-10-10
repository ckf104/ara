// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Kefa Chen <1900011634@pku.edu.cn>
// Description:
// This controller generate readable and writable signals from instruction read/wrte
// state and hazard tables. Operand requester will use these signals to control reads
// and writes of the register file.

module rw_controller import ara_pkg::*; import rvv_pkg::*; #(
  parameter  type          vaddr_t = logic
) (
  input vaddr_t [NrVInsn-1:0][NrHazardOperands-1:0] insn_read_proceed_i,
  input vaddr_t [NrVInsn-1:0] insn_write_proceed_i,
  input logic [NrVInsn-1:0][NrVInsn-1:0][NrHazardOperands-1:0] global_read_hazard_table_i,
  input logic [NrVInsn-1:0][NrVInsn-1:0][NrHazardOperands-1:0] global_write_hazard_table_i,
  output logic [NrVInsn-1:0][NrHazardOperands-1:0] readable_o,
  output logic [NrVInsn-1:0] writable_o
);
  // global_read_hazard_table[i][i] / global_write_hazard_table[i][i]
  logic [NrVInsn-1:0][NrVInsn-1:0][NrHazardOperands-1:0] global_read_hazard_table;
  logic [NrVInsn-1:0][NrVInsn-1:0][NrHazardOperands-1:0] global_write_hazard_table;
  logic [NrVInsn-1:0][NrVInsn-1:0] writable;

  always_comb begin
    global_read_hazard_table = global_read_hazard_table_i;
    global_write_hazard_table = global_write_hazard_table_i;
    for(int i=0; i<NrVInsn; ++i) begin
      global_read_hazard_table[i][i] = 'b0;
      global_write_hazard_table[i][i] = 'b0;
    end

    for(int i=0; i<NrVInsn; ++i) begin
      for(int j=0; j<NrVInsn; ++j) begin
        writable[i][j] = 1'b1;
        for(int k=0; k<NrHazardOperands; ++k) begin
          writable[i][j] = writable[i][j] &&
            (!global_write_hazard_table[i][j][k] || insn_write_proceed_i[i] < insn_read_proceed_i[j][k]);
        end
      end
      writable_o[i] = &writable[i];
    end

    for(int i=0; i<NrVInsn; ++i) begin
      for(int j=0; j<NrHazardOperands; ++j) begin
        readable_o[i][j] = 1'b1;
        for(int k=0; k<NrVInsn; ++k) begin
          readable_o[i][j] = readable_o[i][j] &&
            (!global_read_hazard_table[i][k][j] || insn_read_proceed_i[i][j] < insn_write_proceed_i[k]);
        end
      end
    end
  end
endmodule : rw_controller