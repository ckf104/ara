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
  asm volatile("vlse16.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);
  XCMP(0, mcause, 4);
  // TODO: maybe it should be the real misaligned addr?
  XCMP(0, mtval, ((uint64_t)INP1));
  XCMP(0, vstart, 0);

  stride = 0x400000;
  VSET(16, e8, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0x33));
  asm volatile("vlse8.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);
  XCMP(0, mcause, 13);
  XCMP(0, mtval, ((uint64_t)INP1 + stride));
  XCMP(0, vstart, 1);
  VSET(1, e8, m1);
  VCMP_U8(0, v1, 0xe4);
}

// Positive-stride tests
void TEST_CASE1(void) {
  VSET(4, e8, m1);
  volatile uint8_t INP1[] = {0x9f, 0xe4, 0x19, 0x20, 0x8f, 0x2e, 0x05, 0xe0,
                             0xf9, 0xaa, 0x71, 0xf0, 0xc3, 0x94, 0xbb, 0xd3};
  uint64_t stride = 3;
  asm volatile("vlse8.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  VCMP_U8(1, v1, 0x9f, 0x20, 0x05, 0xaa);
}

void TEST_CASE2(void) {
  VSET(4, e16, m1);
  volatile uint16_t INP1[] = {0x9fe4, 0x1920, 0x8f2e, 0x05e0,
                              0xf9aa, 0x71f0, 0xc394, 0xbbd3};
  uint64_t stride = 4;
  asm volatile("vlse16.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  VCMP_U16(2, v1, 0x9fe4, 0x8f2e, 0xf9aa, 0xc394);
}

void TEST_CASE3(void) {
  VSET(4, e32, m1);
  volatile uint32_t INP1[] = {0x9fe41920, 0x8f2e05e0, 0xf9aa71f0, 0xc394bbd3,
                              0xa11a9384, 0xa7163840, 0x99991348, 0xa9f38cd1};
  uint64_t stride = 8;
  asm volatile("vlse32.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  VCMP_U32(3, v1, 0x9fe41920, 0xf9aa71f0, 0xa11a9384, 0x99991348);
}

void TEST_CASE4(void) {
  VSET(4, e64, m1);
  volatile uint64_t INP1[] = {0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3,
                              0xa11a9384a7163840, 0x99991348a9f38cd1};
  uint64_t stride = 8;
  asm volatile("vlse64.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  VCMP_U64(4, v1, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1);
}

// Zero-stride tests
// The implementation must perform all the memory accesses
void TEST_CASE5(void) {
  VSET(16, e8, m1);
  volatile uint8_t INP1[] = {0x9f};
  uint64_t stride = 0;
  asm volatile("vlse8.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  VCMP_U8(5, v1, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f,
          0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f);
}

// The implementation can also perform fewer accesses
void TEST_CASE6(void) {
  VSET(16, e8, m1);
  volatile uint8_t INP1[] = {0x9f};
  asm volatile("vlse8.v v1, (%0), x0" ::"r"(INP1));
  VCMP_U8(6, v1, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f,
          0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f);
}

// Different LMUL
void TEST_CASE7(void) {
  VSET(8, e64, m2);
  volatile uint64_t INP1[] = {0x9fa831c7a11a9384};
  asm volatile("vlse64.v v2, (%0), x0" ::"r"(INP1));
  VCMP_U64(7, v2, 0x9fa831c7a11a9384, 0x9fa831c7a11a9384, 0x9fa831c7a11a9384,
           0x9fa831c7a11a9384, 0x9fa831c7a11a9384, 0x9fa831c7a11a9384,
           0x9fa831c7a11a9384, 0x9fa831c7a11a9384);
}

// Others
// Negative-stride test
void TEST_CASE8(void) {
  VSET(4, e16, m1);
  volatile uint16_t INP1[] = {0x9fe4, 0x1920, 0x8f2e, 0x05e0,
                              0xf9aa, 0x71f0, 0xc394, 0xbbd3};
  uint64_t stride = -4;
  asm volatile("vlse16.v v1, (%0), %1" ::"r"(&INP1[7]), "r"(stride));
  VCMP_U16(8, v1, 0xbbd3, 0x71f0, 0x05e0, 0x1920);
}

// Stride greater than default Ara AXI width == 128-bit (4 lanes)
void TEST_CASE9(void) {
  VSET(2, e64, m1);
  volatile uint64_t INP1[] = {0x99991348a9f38cd1, 0x9fa831c7a11a9384,
                              0x9fa831c7a11a9384, 0x9fa831c7a11a9384,
                              0x9fa831c7a11a9384, 0x01015ac1309bb678};
  uint64_t stride = 40;
  asm volatile("vlse64.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  VCMP_U64(9, v1, 0x99991348a9f38cd1, 0x01015ac1309bb678);
}

// Fill Ara internal Load Buffer
void TEST_CASE10(void) {
  VSET(8, e64, m1);
  volatile uint64_t INP1[] = {
      0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
      0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
      0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
      0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
      0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
      0x8913984898951989};
  uint64_t stride = 16;
  asm volatile("vlse64.v v1, (%0), %1" ::"r"(INP1), "r"(stride));
  VCMP_U64(10, v1, 0x9fe419208f2e05e0, 0xa11a9384a7163840, 0x9fa831c7a11a9384,
           0x1893179501093489, 0x1874754791888188, 0x9013930148815808,
           0x9031850931584902, 0x8319599991911111);
}

// Masked stride loads
void TEST_CASE11(void) {
  VSET(4, e8, m1);
  volatile uint8_t INP1[] = {0x9f, 0xe4, 0x19, 0x20, 0x8f, 0x2e, 0x05, 0xe0,
                             0xf9, 0xaa, 0x71, 0xf0, 0xc3, 0x94, 0xbb, 0xd3};
  uint64_t stride = 3;
  VLOAD_8(v0, 0xAA);
  VCLEAR(v1);
  asm volatile("vlse8.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U8(11, v1, 0x00, 0x20, 0x00, 0xaa);
}

void TEST_CASE12(void) {
  VSET(4, e16, m1);
  volatile uint16_t INP1[] = {0x9fe4, 0x1920, 0x8f2e, 0x05e0,
                              0xf9aa, 0x71f0, 0xc394, 0xbbd3};
  uint64_t stride = 4;
  VLOAD_8(v0, 0xAA);
  VCLEAR(v1);
  asm volatile("vlse16.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U16(12, v1, 0, 0x8f2e, 0, 0xc394);
}

void TEST_CASE13(void) {
  VSET(4, e32, m1);
  volatile uint32_t INP1[] = {0x9fe41920, 0x8f2e05e0, 0xf9aa71f0, 0xc394bbd3,
                              0xa11a9384, 0xa7163840, 0x99991348, 0xa9f38cd1};
  uint64_t stride = 8;
  VLOAD_8(v0, 0xAA);
  VCLEAR(v1);
  asm volatile("vlse32.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U32(13, v1, 0, 0xf9aa71f0, 0, 0x99991348);
}

void TEST_CASE14(void) {
  VSET(8, e64, m1);
  volatile uint64_t INP1[] = {
      0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
      0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
      0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
      0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
      0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
      0x8913984898951989};
  uint64_t stride = 16;
  VLOAD_8(v0, 0xAA);
  VCLEAR(v1);
  asm volatile("vlse64.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U64(14, v1, 0, 0xa11a9384a7163840, 0, 0x1893179501093489, 0,
           0x9013930148815808, 0, 0x8319599991911111);
}

// Masked stride load with different vstart value
void TEST_CASE15(void) {
  uint8_t mask8 = 0xAA;
  uint16_t mask16 = ((uint16_t)mask8 << 8) | mask8;
  uint32_t mask32 = ((uint32_t)mask16 << 16) | mask16;
  uint64_t mask64 = ((uint64_t)mask32 << 32) | mask32;
  uint64_t stride = 0;
  volatile uint64_t INP1[] = {
      0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
      0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
      0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
      0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
      0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
      0x8913984898951989};

  stride = 16;
  VSET(8, e64, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask64));
  VCLEAR(v1);
  write_csr(vstart, 5);
  asm volatile("vlse64.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U64(15, v1, 0, 0, 0, 0, 0, 0x9013930148815808, 0, 0x8319599991911111);

  stride = 8;
  VSET(16, e32, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask32));
  VCLEAR(v1);
  write_csr(vstart, 12);
  asm volatile("vlse32.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U32(16, v1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x37598759, 0, 0x98951989);

  stride = 4;
  VSET(16, e16, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask16));
  VCLEAR(v1);
  write_csr(vstart, 3);
  asm volatile("vlse16.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U16(17, v1, 0, 0, 0, 0x71f0, 0, 0x9384, 0, 0x1348, 0, 0x31c7,
           0, 0x7598, 0, 0x1795, 0, 0x7598);

  stride = 3;
  VSET(16, e8, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask8));
  VCLEAR(v1);
  write_csr(vstart, 1);
  asm volatile("vlse8.v v1, (%0), %1, v0.t" ::"r"(INP1), "r"(stride));
  VCMP_U8(18, v1, 0, 0x8f, 0, 0xbb, 0, 0xf9, 0, 0x93, 0, 0xa9,
           0, 0x93, 0, 0x9f, 0, 0x75);
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

  TEST_CASE8();
  TEST_CASE9();
  TEST_CASE10();

  TEST_CASE11();
  TEST_CASE12();
  TEST_CASE13();
  TEST_CASE14();

  TEST_CASE15();

  EXIT_CHECK();
}
