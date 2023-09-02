# Run Ara in Genesys2
Forked from [ara](https://github.com/pulp-platform/ara), the goal of this project is to boot linux on ara by genesys2 fpga board. Basically, I add peripherals imitating corev-apu of cva6.

I have added the following peripherals (address mapping is the same with corev-apu):

* bootrom
* uart
* spi (with a sdcard slot)
* timer
* clint
* plic
* riscv-debugger
* ddr3


Currently, linux has been booted successfully and patched appropriately for vector support. **But it seems to ara has no mmu support so U/S mode vector program can't run correctly**.


# Usage Command
To use this repo, first install `fusesoc` and `vivado` in host machine, then:

* Follow the command of [ara project](https://github.com/pulp-platform/ara) first.
* Patch submodules `common_cells` and `cva6` by `fpga/common_cells.patch` and `cva6-dts.patch` respectively.
* Run `fusesoc library add ara .` in root directory of project.
* Run `fusesoc run --build --target synth-genesys2 --build-root fpga-genesys2 ara` in root direcory of project to launch vivado to synthesis ara

After running, you should find generated bitstream `xilinx_ara_soc.bit` in directory `build/fpga-genesys2/synth-genesys2-vivado/ara_0.runs/impl_1`.

# ChangeList for Cva6
In `fpga/cva6-dts.patch`, I add following changes:

* Modify device tree to contain current peripheral information.
* Change AXI4 id width from 4 into 3 for ariane.
* VS bits can be written from `sstatus` register.


# Todo List
* Upload `cva6-sdk` patch (now, I have changed sdcard partition and board device tree).
* Add mmu support for ara.
* Fix illegal instruction error: currently, reading and writing VCSR will raise illegal instruction.
* Understanding why riscv-debugger and timer are essential for whole soc
  * Deleting debugger and timer, instruction access exception will be raised when fetching first U mode instruction.
  * Deleting timer, no output from U mode program.