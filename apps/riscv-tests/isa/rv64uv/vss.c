// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Matteo Perotti <mperotti@iis.ee.ethz.ch>

#include "vector_macros.h"

// Init page table and enable MMU.
// Assume that memory size is 8MB, start from 0x80000000 and SV39 configuration.
// The first 4MB is mapped by 2MB size page. Latter memory used
// for page table entries.
// For memory range 0x80000000 - 0x80400000, they use the second entry of top page and
// 0 - 1 entries of the secondary page.
// In addition, we assume that uart base addr is 0xC0000000.
// Using a 1G mapping for uart, which should cover ctrl device
// located in 0xD0000000;
void rvtest_init(void) {
  uint64_t satp_addr            = 0x80000000UL + 0x400000UL;
  uint64_t secondary_table_addr = 0x80000000UL + 0x500000UL;
  uint64_t uart_base_addr       = 0xC0000000UL;
  uint64_t U_bit = 1 << 4;  // user level access
  uint64_t V_bit = 1 << 0;  // valid page entry
  // cva6 request software to manage A/D bit. If not set, page fault will be
  // raised when loading/storing.
  uint64_t A_bit = 1 << 6;
  uint64_t D_bit = 1 << 7;
  uint64_t RWX_bit = (1 << 1) | (1 << 2) | (1 << 3); // R, W, X bit
  uint64_t* top_page = (uint64_t*)satp_addr;
  uint64_t* secondary_page = (uint64_t*)secondary_table_addr;

  for(int i=0; i<512; ++i) top_page[i] = 0;
  // for new uart addr
  top_page[0] = U_bit | V_bit | ((0UL >> 12) << 10) | RWX_bit | A_bit | D_bit;
  // Only the second entry in top page is valid
  top_page[2] = U_bit | V_bit | ((secondary_table_addr >> 12) << 10);
  // the third entry used for IO device
  top_page[3] = U_bit | V_bit | ((uart_base_addr >> 12) << 10) | RWX_bit | A_bit | D_bit;

  for(int i=0; i<512; ++i) secondary_page[i] = 0;
  // First 2 entries in secondary page are valid
  secondary_page[0] = U_bit | V_bit | ((0x80000000UL >> 12) << 10) | RWX_bit | A_bit | D_bit;
  secondary_page[1] = U_bit | V_bit | ((0x80200000UL >> 12) << 10) | RWX_bit | A_bit | D_bit;

  uint64_t satp_value = (8UL << 60) | (satp_addr >> 12);
  asm volatile("csrw satp, %0" ::"r"(satp_value));
  asm volatile("sfence.vma");
}

// Exception Handler for rtl

void mtvec_handler(void) {
  asm volatile("csrr t3, mcause"); // Read mcause
  asm volatile("csrr t4, mtval");  // Read mtval

  // Read mepc
  asm volatile("csrr t1, mepc");

  // Increment return address by 4
  asm volatile("addi t1, t1, 4");
  asm volatile("csrw mepc, t1");

  // Filter with mcause and handle here

  asm volatile("mret");
}

void TEST_CASE0(void) {
  uint8_t mcause;
  uint64_t mtval, vstart;
  VSET(16, e16, m1);
  // misaligned stride
  uint64_t stride = 3;
  volatile uint16_t INP1[] = {0x9fe4, 0x1920, 0x8f2e, 0x05e0,
                              0xf9aa, 0x71f0, 0xc394, 0xbbd3};
  asm volatile("vsse16.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);
  XCMP(0, mcause, 6);
  // TODO: maybe it should be the real misaligned addr?
  XCMP(0, mtval, ((uint64_t)INP1));
  XCMP(0, vstart, 0);

  stride = 0x400000;
  VSET(16, e8, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0x33));
  asm volatile("vmv.v.x v1, %[A]" ::[A] "r"(0x33));
  asm volatile("vsse8.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);
  XCMP(0, mcause, 15);
  XCMP(0, mtval, ((uint64_t)INP1 + stride));
  XCMP(0, vstart, 1);
  XCMP(0, (uint8_t)(INP1[0]), 0x33);
}

// Positive-stride tests
void TEST_CASE1(void) {
  VSET(4, e8, m1);
  volatile uint8_t OUT1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint64_t stride = 3;
  VLOAD_8(v1, 0x9f, 0xe4, 0x19, 0x20);
  asm volatile("vsse8.v v1, (%0), %1" ::"r"(OUT1), "r"(stride));
  VVCMP_U8(1, OUT1, 0x9f, 0x00, 0x00, 0xe4, 0x00, 0x00, 0x19, 0x00, 0x00, 0x20,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

void TEST_CASE2(void) {
  VSET(8, e16, m1);
  volatile uint16_t OUT1[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                              0x0000, 0x0000, 0x0000, 0x0000};
  uint64_t stride = 4;
  VLOAD_16(v1, 0x9f11, 0xe478, 0x1549, 0x3240, 0x2f11, 0xe448, 0x1546, 0x3220);
  asm volatile("vsse16.v v1, (%0), %1" ::"r"(OUT1), "r"(stride));
  VVCMP_U16(2, OUT1, 0x9f11, 0x0000, 0xe478, 0x0000, 0x1549, 0x0000, 0x3240,
            0x0000, 0x2f11, 0x0000, 0xe448, 0x0000, 0x1546, 0x0000, 0x3220,
            0x0000);
}

void TEST_CASE3(void) {
  VSET(4, e32, m1);
  volatile uint32_t OUT1[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000};
  uint64_t stride = 8;
  VLOAD_32(v1, 0x9f872456, 0xe1356784, 0x13241139, 0x20862497);
  asm volatile("vsse32.v v1, (%0), %1" ::"r"(OUT1), "r"(stride));
  VVCMP_U32(3, OUT1, 0x9f872456, 0x00000000, 0xe1356784, 0x00000000, 0x13241139,
            0x00000000, 0x20862497, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000);
}

void TEST_CASE4(void) {
  VSET(16, e64, m1);
  volatile uint64_t OUT1[] = {
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000};
  uint64_t stride = 16;
  VLOAD_64(v1, 0x9f87245315434136, 0xe135578794246784, 0x1315345345241139,
           0x2086252110062497, 0x1100229933847136, 0xaaffaaffaaffaaff,
           0xaf87245315434136, 0xa135578794246784, 0x2315345345241139,
           0x1086252110062497, 0x1100229933847134, 0xaaffaaffaaffaaf4,
           0x9315345345241139, 0x9086252110062497, 0x9100229933847134,
           0x9affaaffaaffaaf4);
  asm volatile("vsse64.v v1, (%0), %1" ::"r"(OUT1), "r"(stride));
  VVCMP_U64(4, OUT1, 0x9f87245315434136, 0x0000000000000000, 0xe135578794246784,
            0x0000000000000000, 0x1315345345241139, 0x0000000000000000,
            0x2086252110062497, 0x0000000000000000, 0x1100229933847136,
            0x0000000000000000, 0xaaffaaffaaffaaff, 0x0000000000000000,
            0xaf87245315434136, 0x0000000000000000, 0xa135578794246784,
            0x0000000000000000, 0x2315345345241139, 0x0000000000000000,
            0x1086252110062497, 0x0000000000000000, 0x1100229933847134,
            0x0000000000000000, 0xaaffaaffaaffaaf4, 0x0000000000000000,
            0x9315345345241139, 0x0000000000000000, 0x9086252110062497,
            0x0000000000000000, 0x9100229933847134, 0x0000000000000000,
            0x9affaaffaaffaaf4, 0x0000000000000000);
}

// Masked strided store
void TEST_CASE5(void) {
  VSET(4, e8, m1);
  volatile uint8_t OUT1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint64_t stride = 3;
  VLOAD_8(v0, 0xAA);
  VLOAD_8(v1, 0x9f, 0xe4, 0x19, 0x20);
  asm volatile("vsse8.v v1, (%0), %1, v0.t" ::"r"(OUT1), "r"(stride));
  VVCMP_U8(5, OUT1, 0x00, 0x00, 0x00, 0xe4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

void TEST_CASE6(void) {
  VSET(16, e64, m1);
  volatile uint64_t OUT1[] = {
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000};
  uint64_t stride = 16;
  VLOAD_64(v1, 0x9f87245315434136, 0xe135578794246784, 0x1315345345241139,
           0x2086252110062497, 0x1100229933847136, 0xaaffaaffaaffaaff,
           0xaf87245315434136, 0xa135578794246784, 0x2315345345241139,
           0x1086252110062497, 0x1100229933847134, 0xaaffaaffaaffaaf4,
           0x9315345345241139, 0x9086252110062497, 0x9100229933847134,
           0x9affaaffaaffaaf4);
  VLOAD_8(v0, 0xAA, 0xAA);
  asm volatile("vsse64.v v1, (%0), %1, v0.t" ::"r"(OUT1), "r"(stride));
  VVCMP_U64(6, OUT1, 0x0000000000000000, 0x0000000000000000, 0xe135578794246784,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x2086252110062497, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0xaaffaaffaaffaaff, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0xa135578794246784,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x1086252110062497, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0xaaffaaffaaffaaf4, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x9086252110062497,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x9affaaffaaffaaf4, 0x0000000000000000);
}

// Masked stride store with different vstart value
void TEST_CASE7(void) {
  uint64_t stride = 16;
  volatile uint64_t OUT1[] = {
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
      0x0000000000000000, 0x0000000000000000};
  VSET(16, e64, m1);

  VLOAD_64(v1, 0x9f87245315434136, 0xe135578794246784, 0x1315345345241139,
           0x2086252110062497, 0x1100229933847136, 0xaaffaaffaaffaaff,
           0xaf87245315434136, 0xa135578794246784, 0x2315345345241139,
           0x1086252110062497, 0x1100229933847134, 0xaaffaaffaaffaaf4,
           0x9315345345241139, 0x9086252110062497, 0x9100229933847134,
           0x9affaaffaaffaaf4);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0xAAAAAAAAAAAAAAAA));
  write_csr(vstart, 7);
  asm volatile("vsse64.v v1, (%0), %1, v0.t" ::"r"(OUT1), "r"(stride));
  VVCMP_U64(7, OUT1, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0xa135578794246784,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x1086252110062497, 0x0000000000000000, 0x0000000000000000,
            0x0000000000000000, 0xaaffaaffaaffaaf4, 0x0000000000000000,
            0x0000000000000000, 0x0000000000000000, 0x9086252110062497,
            0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
            0x9affaaffaaffaaf4, 0x0000000000000000);

  for(int i=0;i<32;++i) OUT1[i] = 0;
  VSET(16, e32, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0xAAAAAAAA));
  stride = 16;
  VLOAD_32(v1, 0x15434136, 0x13557879, 0x13153453, 0x10062497, 0x33847136, 0xaaffaaff,
           0x15434136, 0x94246784, 0x45241139, 0x10862521, 0x11002299, 0xaaffaaf4,
           0x93153453, 0x90862521, 0x33847134, 0x9affaaf4);
  write_csr(vstart, 2);
  asm volatile("vsse32.v v1, (%0), %1, v0.t" ::"r"(OUT1), "r"(stride));
  VVCMP_U64(8, (OUT1), 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x10062497, 0x0, 0x0,
            0x0, 0xaaffaaff, 0x0, 0x0, 0x0, 0x94246784, 0x0, 0x0, 0x0,
            0x10862521, 0x0, 0x0, 0x0, 0xaaffaaf4, 0x0, 0x0, 0x0, 0x90862521,
            0x0, 0x0, 0x0, 0x9affaaf4, 0x0);

  for(int i=0;i<32;++i) OUT1[i] = 0;
  VSET(16, e16, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0xAAAA));
  stride = 2;
  VLOAD_16(v1, 0x4136, 0x7879, 0x3453, 0x2497, 0x7136, 0xaaff,
           0x4136, 0x6784, 0x1139, 0x2521, 0x2299, 0xaaf4,
           0x3453, 0x2521, 0x7134, 0xaaf4);
  write_csr(vstart, 9);
  asm volatile("vsse16.v v1, (%0), %1, v0.t" ::"r"(OUT1), "r"(stride));
  VVCMP_U64(9, (OUT1), 0x0, 0x0, 0xaaf4000025210000, 0xaaf4000025210000, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

  for(int i=0;i<32;++i) OUT1[i] = 0;
  VSET(16, e8, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0xAA));
  stride = 1;
  VLOAD_8(v1, 0x36, 0x78, 0x53, 0x97, 0x71, 0xaa, 0x41, 0x67,
           0x11, 0x25, 0x22, 0xf4, 0x53, 0x21, 0x71, 0xf4);
  write_csr(vstart, 4);
  asm volatile("vsse8.v v1, (%0), %1, v0.t" ::"r"(OUT1), "r"(stride));
  VVCMP_U64(10, (OUT1), 0x6700aa0000000000, 0xf4002100f4002500, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
}

int main(void) {
  INIT_CHECK();
  enable_vec();

  TEST_CASE0();

  TEST_CASE1();
  TEST_CASE2();
  TEST_CASE3();
  TEST_CASE4();

  TEST_CASE5();
  TEST_CASE6();
  TEST_CASE7();

  EXIT_CHECK();
}
