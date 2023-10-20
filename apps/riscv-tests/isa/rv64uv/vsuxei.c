// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Matteo Perotti <mperotti@iis.ee.ethz.ch>

#include "vector_macros.h"
#include "long_array.h"

#define AXI_DWIDTH 128

#define INIT 98

void reset_vec8(volatile uint8_t *vec, int rst_val, uint64_t len) {
  for (uint64_t i = 0; i < len; ++i)
    vec[i] = rst_val;
}
void reset_vec16(volatile uint16_t *vec, int rst_val, uint64_t len) {
  for (uint64_t i = 0; i < len; ++i)
    vec[i] = rst_val;
}
void reset_vec32(volatile uint32_t *vec, int rst_val, uint64_t len) {
  for (uint64_t i = 0; i < len; ++i)
    vec[i] = rst_val;
}
void reset_vec64(volatile uint64_t *vec, int rst_val, uint64_t len) {
  for (uint64_t i = 0; i < len; ++i)
    vec[i] = rst_val;
}
static volatile uint8_t BUFFER_O8[16] __attribute__((aligned(AXI_DWIDTH))) = {
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT,
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT};
static volatile uint16_t BUFFER_O16[16] __attribute__((aligned(AXI_DWIDTH))) = {
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT,
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT};
static volatile uint32_t BUFFER_O32[16] __attribute__((aligned(AXI_DWIDTH))) = {
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT,
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT};
static volatile uint64_t BUFFER_O64[16] __attribute__((aligned(AXI_DWIDTH))) = {
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT,
    INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT};

static volatile uint8_t LONG_INDEX_I8[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint8_t GOLD_TMP_I8[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint8_t ALIGNED_I8[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};

static volatile uint16_t LONG_INDEX_I16[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint16_t GOLD_TMP_I16[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint16_t ALIGNED_I16[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};

static volatile uint32_t LONG_INDEX_I32[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint32_t GOLD_TMP_I32[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint32_t ALIGNED_I32[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};

static volatile uint64_t LONG_INDEX_I64[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint64_t GOLD_TMP_I64[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};
static volatile uint64_t ALIGNED_I64[512]
    __attribute__((aligned(AXI_DWIDTH))) = {0};

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
  for(int i=0; i<256; ++i) GOLD_TMP_I16[i] = 0;
  VSET(16, e16, m1);
  // misaligned index
  VLOAD_32(v4, 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40,
          44, 48, 52, 3, 0x800000);
  asm volatile("vle16.v v0, (%0)" ::"r"(LONG_I16));
  write_csr(vstart, 3);
  asm volatile("vsuxei32.v v0, (%0), v4" ::"r"(GOLD_TMP_I16));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);
  XCMP(0, mcause, 6);
  XCMP(0, mtval, ((uint64_t)GOLD_TMP_I16 + 3));
  XCMP(0, vstart, 14);
  VSET(32, e16, m1);
  VVCMP_U16(0, GOLD_TMP_I16, 0, 0, 0, 0, 0, 0, LONG_I16[3],
          0, LONG_I16[4], 0, LONG_I16[5], 0, LONG_I16[6], 0, LONG_I16[7], 0,
          LONG_I16[8], 0, LONG_I16[9], 0, LONG_I16[10], 0, LONG_I16[11], 0,
          LONG_I16[12], 0, LONG_I16[13], 0, 0, 0, 0, 0);

  for(int i=0; i<256; ++i) GOLD_TMP_I32[i] = 0;
  VSET(16, e32, m1);
  VLOAD_64(v4, 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40,
          44, 48, 52, 56, 0x800000);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0x33333333));
  asm volatile("vle32.v v2, (%0)" ::"r"(LONG_I32));
  asm volatile("vsuxei64.v v2, (%0), v4, v0.t" ::"r"(GOLD_TMP_I32));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);
  XCMP(0, mcause, 15);
  XCMP(0, mtval, ((uint64_t)GOLD_TMP_I32 + 0x800000));
  XCMP(0, vstart, 15);
  VSET(16, e32, m1);
  VVCMP_U32(0, GOLD_TMP_I32, LONG_I32[0], LONG_I32[1], 0, 0, LONG_I32[4], LONG_I32[5],
          0, 0, LONG_I32[8], LONG_I32[9], 0, 0, LONG_I32[12], LONG_I32[13], 0, 0);
}

// Naive test
void TEST_CASE1(void) {
  VSET(12, e8, m1);
  VLOAD_8(v1, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x88, 0x88, 0xae, 0x91, 0x02, 0x59,
          0x89);
  VLOAD_8(v2, 1, 2, 3, 4, 5, 7, 8, 9, 11, 12, 13, 15);
  asm volatile("vsuxei8.v v1, (%0), v2" ::"r"(&BUFFER_O8[0]));
  VVCMP_U8(1, BUFFER_O8, INIT, 0xd3, 0x40, 0xd1, 0x84, 0x48, INIT, 0x88, 0x88,
           0xae, INIT, 0x91, 0x02, 0x59, INIT, 0x89);

  VSET(12, e16, m1);
  VLOAD_16(v1, 0xbbd3, 0x3840, 0x8cd1, 0x9384, 0x7548, 0x9388, 0x8188, 0x11ae,
           0x4891, 0x4902, 0x8759, 0x1989);
  VLOAD_16(v2, 2, 4, 6, 8, 10, 14, 16, 18, 22, 24, 26, 30);
  asm volatile("vsuxei16.v v1, (%0), v2" ::"r"(&BUFFER_O16[0]));
  VVCMP_U16(2, BUFFER_O16, INIT, 0xbbd3, 0x3840, 0x8cd1, 0x9384, 0x7548, INIT,
            0x9388, 0x8188, 0x11ae, INIT, 0x4891, 0x4902, 0x8759, INIT, 0x1989);

  VSET(12, e32, m1);
  VLOAD_32(v1, 0xf9aa71f0, 0xa11a9384, 0x99991348, 0x9fa831c7, 0x38197598,
           0x81937598, 0x18747547, 0x3eeeeeee, 0xab8b9148, 0x90318509,
           0x31897598, 0x89139848);
  VLOAD_32(v2, 4, 8, 12, 16, 20, 28, 32, 36, 44, 48, 52, 60);
  asm volatile("vsuxei32.v v1, (%0), v2" ::"r"(&BUFFER_O32[0]));
  VVCMP_U32(3, BUFFER_O32, INIT, 0xf9aa71f0, 0xa11a9384, 0x99991348, 0x9fa831c7,
            0x38197598, INIT, 0x81937598, 0x18747547, 0x3eeeeeee, INIT,
            0xab8b9148, 0x90318509, 0x31897598, INIT, 0x89139848);

  VSET(12, e64, m1);
  VLOAD_64(v1, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840, 0x99991348a9f38cd1,
           0x9fa831c7a11a9384, 0x3819759853987548, 0x81937598aa819388,
           0x1874754791888188, 0x3eeeeeeee33111ae, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8913984898951989);
  VLOAD_64(v2, 8, 16, 24, 32, 40, 56, 64, 72, 88, 96, 104, 120);
  asm volatile("vsuxei64.v v1, (%0), v2" ::"r"(&BUFFER_O64[0]));
  VVCMP_U64(4, BUFFER_O64, INIT, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
            0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548, INIT,
            0x81937598aa819388, 0x1874754791888188, 0x3eeeeeeee33111ae, INIT,
            0xab8b914891484891, 0x9031850931584902, 0x3189759837598759, INIT,
            0x8913984898951989);
}

// Naive test, masked
void TEST_CASE2(void) {
  reset_vec8(&BUFFER_O8[0], INIT, 16);
  VSET(12, e8, m1);
  VLOAD_8(v0, 0xAA, 0x0A);
  VLOAD_8(v1, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x88, 0x88, 0xae, 0x91, 0x02, 0x59,
          0x89);
  VLOAD_8(v2, 1, 2, 3, 4, 5, 7, 8, 9, 11, 12, 13, 15);
  asm volatile("vsuxei8.v v1, (%0), v2, v0.t" ::"r"(&BUFFER_O8[0]));
  VVCMP_U8(5, BUFFER_O8, INIT, INIT, 0x40, INIT, 0x84, INIT, INIT, 0x88, INIT,
           0xae, INIT, INIT, 0x02, INIT, INIT, 0x89);

  reset_vec16(&BUFFER_O16[0], INIT, 16);
  VSET(12, e16, m1);
  VLOAD_8(v0, 0xAA, 0x0A);
  VLOAD_16(v1, 0xbbd3, 0x3840, 0x8cd1, 0x9384, 0x7548, 0x9388, 0x8188, 0x11ae,
           0x4891, 0x4902, 0x8759, 0x1989);
  VLOAD_16(v2, 2, 4, 6, 8, 10, 14, 16, 18, 22, 24, 26, 30);
  asm volatile("vsuxei16.v v1, (%0), v2, v0.t" ::"r"(&BUFFER_O16[0]));
  VVCMP_U16(6, BUFFER_O16, INIT, INIT, 0x3840, INIT, 0x9384, INIT, INIT, 0x9388,
            INIT, 0x11ae, INIT, INIT, 0x4902, INIT, INIT, 0x1989);

  reset_vec32(&BUFFER_O32[0], INIT, 16);
  VSET(12, e32, m1);
  VLOAD_8(v0, 0xAA, 0x0A);
  VLOAD_32(v1, 0xf9aa71f0, 0xa11a9384, 0x99991348, 0x9fa831c7, 0x38197598,
           0x81937598, 0x18747547, 0x3eeeeeee, 0xab8b9148, 0x90318509,
           0x31897598, 0x89139848);
  VLOAD_32(v2, 4, 8, 12, 16, 20, 28, 32, 36, 44, 48, 52, 60);
  asm volatile("vsuxei32.v v1, (%0), v2, v0.t" ::"r"(&BUFFER_O32[0]));
  VVCMP_U32(7, BUFFER_O32, INIT, INIT, 0xa11a9384, INIT, 0x9fa831c7, INIT, INIT,
            0x81937598, INIT, 0x3eeeeeee, INIT, INIT, 0x90318509, INIT, INIT,
            0x89139848);

  reset_vec64(&BUFFER_O64[0], INIT, 16);
  VSET(12, e64, m1);
  VLOAD_8(v0, 0xAA, 0x0A);
  VLOAD_64(v1, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840, 0x99991348a9f38cd1,
           0x9fa831c7a11a9384, 0x3819759853987548, 0x81937598aa819388,
           0x1874754791888188, 0x3eeeeeeee33111ae, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8913984898951989);
  VLOAD_64(v2, 8, 16, 24, 32, 40, 56, 64, 72, 88, 96, 104, 120);
  asm volatile("vsuxei64.v v1, (%0), v2, v0.t" ::"r"(&BUFFER_O64[0]));
  VVCMP_U64(8, BUFFER_O64, INIT, INIT, 0xa11a9384a7163840, INIT,
            0x9fa831c7a11a9384, INIT, INIT, 0x81937598aa819388, INIT,
            0x3eeeeeeee33111ae, INIT, INIT, 0x9031850931584902, INIT, INIT,
            0x8913984898951989);
}

// masked index load with different vstart value
void TEST_CASE3(void) {
  uint8_t mask8 = 0xAA;
  uint16_t mask16 = ((uint16_t)mask8 << 8) | mask8;
  uint32_t mask32 = ((uint32_t)mask16 << 16) | mask16;
  uint64_t mask64 = ((uint64_t)mask32 << 32) | mask32;

  for(int i = 0; i < 512; ++i)LONG_INDEX_I16[i] = i;
  for(int i = 0; i < 512; ++i){
    if(i < 9 || i % 2 == 0) GOLD_TMP_I8[i] = 0;
    else GOLD_TMP_I8[i] = LONG_I8[i];
    ALIGNED_I8[i] = 0;
  }

  VSET(512, e8, m2);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask8));
  asm volatile("vle16.v v4, (%0)" ::"r"(&LONG_INDEX_I16[0]));
  asm volatile("vle8.v  v2, (%0)" ::"r"(&LONG_I8[0]));
  write_csr(vstart, 9);
  asm volatile("vsuxei16.v v2, (%0), v4, v0.t" ::"r"(&ALIGNED_I8[0]));
  LVVCMP_U8(9, ALIGNED_I8, GOLD_TMP_I8);

  for(int i = 0; i < 512; ++i)LONG_INDEX_I32[i] = 4 * i;
  for(int i = 0; i < 512; ++i){
    ALIGNED_I16[i] = 0;
    GOLD_TMP_I16[i] = 0;
    if(i / 2 >= 207 && i % 4 == 2) GOLD_TMP_I16[i] = LONG_I16[i / 2];
  }

  VSET(256, e16, m2);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask16));
  asm volatile("vle32.v v4, (%0)" ::"r"(&LONG_INDEX_I32[0]));
  asm volatile("vle16.v v2, (%0)" ::"r"(&LONG_I16[0]));
  write_csr(vstart, 207);
  asm volatile("vsuxei32.v v2, (%0), v4, v0.t" ::"r"(&ALIGNED_I16[0]));
  VSET(512, e8, m2);
  LVVCMP_U16(10, ALIGNED_I16, GOLD_TMP_I16);

  for(int i = 0; i < 512; ++i)LONG_INDEX_I64[i] = 8 * i;
  for(int i = 0; i < 512; ++i){
    ALIGNED_I32[i] = 0;
    GOLD_TMP_I32[i] = 0;
    if(i / 2 >= 70 && i % 4 == 2) GOLD_TMP_I32[i] = LONG_I32[i / 2];
  }

  VSET(256, e32, m4);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask32));
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_INDEX_I64[0]));
  asm volatile("vle32.v v4, (%0)" ::"r"(&LONG_I32[0]));
  write_csr(vstart, 70);
  asm volatile("vsuxei64.v v4, (%0), v8, v0.t" ::"r"(&ALIGNED_I32[0]));
  VSET(512, e8, m2);
  LVVCMP_U32(11, ALIGNED_I32, GOLD_TMP_I32);

  for(int i = 0; i < 512; ++i)LONG_INDEX_I8[i] = (uint8_t)((8 * i) % 256);
  for(int i = 0; i < 512; ++i){
    ALIGNED_I64[i] = 0;
    if(i >= 32 || i % 2 == 0) GOLD_TMP_I64[i] = 0;
    else GOLD_TMP_I64[i] = LONG_I64[224 + i];
  }

  VSET(256, e64, m8);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask64));
  asm volatile("vle8.v v1, (%0)" ::"r"(&LONG_INDEX_I8[0]));
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_I64[0]));
  write_csr(vstart, 135);
  asm volatile("vsuxei8.v v8, (%0), v1, v0.t" ::"r"(&ALIGNED_I64[0]));
  VSET(512, e8, m2);
  LVVCMP_U64(12, ALIGNED_I64, GOLD_TMP_I64);
}

int main(void) {
  INIT_CHECK();
  enable_vec();

  TEST_CASE0();

  TEST_CASE1();
  TEST_CASE2();
  TEST_CASE3();

  EXIT_CHECK();
}
