// TODO uncomment TEST_CASE12 and TEST_CASE 14 after issue of vl=0 and
// non-zero vstart is resolved
// TODO uncomment TEST_CASE2 after issue of exception is resolved
#include "long_array.h"
#include "vector_macros.h"

#define AXI_DWIDTH 128

static volatile uint64_t GOLD_TMP_I64[512] __attribute__((aligned(AXI_DWIDTH))) = {};

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

// Exception Handler for spike
void handle_trap(void) {
  // Read mepc
  asm volatile("csrr t1, mepc");

  // Increment return address by 4
  asm volatile("addi t1, t1, 4");
  asm volatile("csrw mepc, t1");

  asm volatile("ld ra, 8(sp)");
  asm volatile("ld sp, 16(sp)");
  asm volatile("ld gp, 24(sp)");
  asm volatile("ld tp, 32(sp)");
  asm volatile("ld t0, 40(sp)");
  asm volatile("ld t0, 40(sp)");
  asm volatile("ld t1, 48(sp)");
  asm volatile("ld t2, 56(sp)");
  asm volatile("ld s0, 64(sp)");
  asm volatile("ld s1, 72(sp)");
  asm volatile("ld a0, 80(sp)");
  asm volatile("ld a1, 88(sp)");
  asm volatile("ld a2, 96(sp)");
  asm volatile("ld a3, 104(sp)");
  asm volatile("ld a4, 112(sp)");
  asm volatile("ld a5, 120(sp)");
  asm volatile("ld a6, 128(sp)");
  asm volatile("ld a7, 136(sp)");
  asm volatile("ld s2, 144(sp)");
  asm volatile("ld s3, 152(sp)");
  asm volatile("ld s4, 160(sp)");
  asm volatile("ld s5, 168(sp)");
  asm volatile("ld s6, 176(sp)");
  asm volatile("ld s7, 184(sp)");
  asm volatile("ld s8, 192(sp)");
  asm volatile("ld s9, 200(sp)");
  asm volatile("ld s10, 208(sp)");
  asm volatile("ld s11, 216(sp)");
  asm volatile("ld t3, 224(sp)");
  asm volatile("ld t4, 232(sp)");
  asm volatile("ld t5, 240(sp)");
  asm volatile("ld t6, 248(sp)");

  // Read mcause
  asm volatile("csrr t3, mcause");

  asm volatile("addi sp, sp, 272");

  // Filter with mcause and handle here

  asm volatile("mret");
}

void reset_vec64(volatile uint64_t *vec) {
  for (uint64_t i = 0; i < 1024; ++i)
    vec[i] = 0;
}

static volatile uint64_t ALIGNED_I64[1024] __attribute__((aligned(AXI_DWIDTH)));

//**********Checking functionality of vse64 with different destination
// registers********//
void TEST_CASE1(void) {
  VSET(16, e64, m1);
  VLOAD_64(v0, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  asm volatile("vse64.v v0, (%0)" ::"r"(ALIGNED_I64));
  VVCMP_U64(1, ALIGNED_I64, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3,
            0xa11a9384a7163840, 0x99991348a9f38cd1, 0x9fa831c7a11a9384,
            0x3819759853987548, 0x1893179501093489, 0x81937598aa819388,
            0x1874754791888188, 0x3eeeeeeee33111ae, 0x9013930148815808,
            0xab8b914891484891, 0x9031850931584902, 0x3189759837598759,
            0x8319599991911111, 0x8913984898951989);
}

//******Checking functionality of  with illegal destination register
// specifier for EMUL********//
// In this test case EMUL=2 and register is v1 which will cause illegal
// instruction exception and set mcause = 2
void TEST_CASE2(void) {
  uint8_t mcause;
  uint64_t mtval, vstart;
  uint64_t store_addr;
  reset_vec64(ALIGNED_I64);
  VSET(16, e64, m1);
  VLOAD_64(v1, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  VSET(16, e64, m2);
  asm volatile("vse64.v v1, (%0)" ::"r"(ALIGNED_I64));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  XCMP(2, mcause, 2);

  store_addr = 0x80400008;
  VSET(128, e16, m2);
  asm volatile("vse64.v v8, (%0)" ::"r"(store_addr));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);

  XCMP(2, mcause, 15);
  XCMP(2, mtval, 0x80400008);
  XCMP(2, vstart, 0);
}

//*******Checking functionality of vse16 with different values of masking
// register******//
void TEST_CASE3(void) {
  reset_vec64(ALIGNED_I64);
  VSET(16, e64, m1);
  VLOAD_64(v3, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  VLOAD_8(v0, 0xFF, 0xFF);
  asm volatile("vse64.v v3, (%0), v0.t" ::"r"(ALIGNED_I64));
  VCLEAR(v3);
  VVCMP_U64(3, ALIGNED_I64, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3,
            0xa11a9384a7163840, 0x99991348a9f38cd1, 0x9fa831c7a11a9384,
            0x3819759853987548, 0x1893179501093489, 0x81937598aa819388,
            0x1874754791888188, 0x3eeeeeeee33111ae, 0x9013930148815808,
            0xab8b914891484891, 0x9031850931584902, 0x3189759837598759,
            0x8319599991911111, 0x8913984898951989);
}

void TEST_CASE4(void) {
  VSET(16, e64, m1);
  VLOAD_64(v3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v3, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v3);
  VLOAD_64(v3, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  VLOAD_8(v0, 0x00, 0x00);
  asm volatile("vse64.v v3, (%0), v0.t" ::"r"(ALIGNED_I64));
  VCLEAR(v3);
  VVCMP_U64(4, ALIGNED_I64, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16);
}

void TEST_CASE5(void) {
  VSET(16, e64, m1);
  VLOAD_64(v3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v3, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v3);
  VLOAD_64(v3, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  VLOAD_8(v0, 0xAA, 0xAA);
  asm volatile("vse64.v v3, (%0), v0.t" ::"r"(ALIGNED_I64));
  VCLEAR(v3);
  VVCMP_U64(5, ALIGNED_I64, 1, 0xf9aa71f0c394bbd3, 3, 0x99991348a9f38cd1, 5,
            0x3819759853987548, 7, 0x81937598aa819388, 9, 0x3eeeeeeee33111ae,
            11, 0xab8b914891484891, 13, 0x3189759837598759, 15,
            0x8913984898951989);
}

//******Checking functionality with different combinations of vta and vma*****//
// **** It uses undisturbed policy for tail agnostic and mask agnostic****//
void TEST_CASE6(void) {
  reset_vec64(ALIGNED_I64);
  uint64_t avl;
  VSET(16, e64, m1);
  VLOAD_64(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v4, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VLOAD_8(v0, 0xAA, 0xAA);
  VLOAD_64(v4, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  __asm__ volatile("vsetivli %[A], 12, e64, m1, ta, ma" : [A] "=r"(avl));
  asm volatile("vse64.v v4, (%0),v0.t" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VVCMP_U64(6, ALIGNED_I64, 1, 0xf9aa71f0c394bbd3, 3, 0x99991348a9f38cd1, 5,
            0x3819759853987548, 7, 0x81937598aa819388, 9, 0x3eeeeeeee33111ae,
            11, 0xab8b914891484891, 13, 14, 15, 16);
}

void TEST_CASE7(void) {
  reset_vec64(ALIGNED_I64);
  uint64_t avl;
  VSET(16, e64, m1);
  VLOAD_64(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v4, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VLOAD_8(v0, 0xAA, 0xAA);
  VLOAD_64(v4, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  __asm__ volatile("vsetivli %[A], 12, e64, m1, ta, mu" : [A] "=r"(avl));
  asm volatile("vse64.v v4, (%0), v0.t" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VVCMP_U64(7, ALIGNED_I64, 1, 0xf9aa71f0c394bbd3, 3, 0x99991348a9f38cd1, 5,
            0x3819759853987548, 7, 0x81937598aa819388, 9, 0x3eeeeeeee33111ae,
            11, 0xab8b914891484891, 13, 14, 15, 16);
}

void TEST_CASE8(void) {
  reset_vec64(ALIGNED_I64);
  uint64_t avl;
  VSET(16, e64, m1);
  VLOAD_64(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v4, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VLOAD_8(v0, 0xAA, 0xAA);
  VLOAD_64(v4, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  __asm__ volatile("vsetivli %[A], 12, e64, m1, tu, ma" : [A] "=r"(avl));
  asm volatile("vse64.v v4, (%0), v0.t" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VVCMP_U64(8, ALIGNED_I64, 1, 0xf9aa71f0c394bbd3, 3, 0x99991348a9f38cd1, 5,
            0x3819759853987548, 7, 0x81937598aa819388, 9, 0x3eeeeeeee33111ae,
            11, 0xab8b914891484891, 13, 14, 15, 16);
}

void TEST_CASE9(void) {
  reset_vec64(ALIGNED_I64);
  uint64_t avl;
  VSET(16, e64, m1);
  VLOAD_64(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v4, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VLOAD_8(v0, 0xAA, 0xAA);
  VLOAD_64(v4, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  __asm__ volatile("vsetivli %[A], 12, e16, m1, tu, mu" : [A] "=r"(avl));
  asm volatile("vse64.v v4, (%0), v0.t" ::"r"(ALIGNED_I64));
  VCLEAR(v4);
  VVCMP_U64(9, ALIGNED_I64, 1, 0xf9aa71f0c394bbd3, 3, 0x99991348a9f38cd1, 5,
            0x3819759853987548, 7, 0x81937598aa819388, 9, 0x3eeeeeeee33111ae,
            11, 0xab8b914891484891, 13, 14, 15, 16);
}

//*******Checking functionality if encoded EEW is not supported for given SEW
// and LMUL values because EMUL become out of range*****//
// This test case cover upper bound of EMUL(8). If LMUL is changed to
// m2 it will give error because emul become greater than 8 (EMUL = 16)
void TEST_CASE10(void) {
  VSET(16, e64, m1);
  VLOAD_64(v8, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  VSET(16, e8, m1);
  asm volatile("vse64.v v8, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v8);
  VVCMP_U64(10, ALIGNED_I64, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3,
            0xa11a9384a7163840, 0x99991348a9f38cd1, 0x9fa831c7a11a9384,
            0x3819759853987548, 0x1893179501093489, 0x81937598aa819388,
            0x1874754791888188, 0x3eeeeeeee33111ae, 0x9013930148815808,
            0xab8b914891484891, 0x9031850931584902, 0x3189759837598759,
            0x8319599991911111, 0x8913984898951989);
}

//******Checking functionality with different values of vl******//
void TEST_CASE11(void) {
  reset_vec64(ALIGNED_I64);
  VSET(16, e64, m1);
  VLOAD_64(v6, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  asm volatile("vse64.v v6, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v6);
  VVCMP_U64(11, ALIGNED_I64, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3,
            0xa11a9384a7163840, 0x99991348a9f38cd1, 0x9fa831c7a11a9384,
            0x3819759853987548, 0x1893179501093489, 0x81937598aa819388,
            0x1874754791888188, 0x3eeeeeeee33111ae, 0x9013930148815808,
            0xab8b914891484891, 0x9031850931584902, 0x3189759837598759,
            0x8319599991911111, 0x8913984898951989);
}

void TEST_CASE12(void) {
  uint64_t avl;
  VSET(16, e64, m1);
  VLOAD_64(v6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v6, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v6);
  VLOAD_64(v6, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  __asm__ volatile("vsetivli %[A], 0, e64, m1, tu, ma" : [A] "=r"(avl));
  asm volatile("vse64.v v6, (%0)" ::"r"(ALIGNED_I64));
  VSET(16, e64, m1);
  VVCMP_U64(12, ALIGNED_I64, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16);
}

void TEST_CASE13(void) {
  VSET(16, e64, m1);
  VLOAD_64(v6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v6, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v6);
  VLOAD_64(v6, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  VSET(13, e64, m1);
  asm volatile("vse64.v v6, (%0)" ::"r"(ALIGNED_I64));
  VSET(16, e64, m1);
  VVCMP_U64(13, ALIGNED_I64, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3,
            0xa11a9384a7163840, 0x99991348a9f38cd1, 0x9fa831c7a11a9384,
            0x3819759853987548, 0x1893179501093489, 0x81937598aa819388,
            0x1874754791888188, 0x3eeeeeeee33111ae, 0x9013930148815808,
            0xab8b914891484891, 0x9031850931584902, 14, 15, 16);
}

//******Checking functionality with different vstart value*****//
void TEST_CASE14(void) {
  reset_vec64(ALIGNED_I64);
  VSET(16, e64, m1);
  VLOAD_64(v7, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  asm volatile("vse64.v v7, (%0)" ::"r"(ALIGNED_I64));
  VCLEAR(v7);
  VLOAD_64(v7, 0x9fe419208f2e05e0, 0xf9aa71f0c394bbd3, 0xa11a9384a7163840,
           0x99991348a9f38cd1, 0x9fa831c7a11a9384, 0x3819759853987548,
           0x1893179501093489, 0x81937598aa819388, 0x1874754791888188,
           0x3eeeeeeee33111ae, 0x9013930148815808, 0xab8b914891484891,
           0x9031850931584902, 0x3189759837598759, 0x8319599991911111,
           0x8913984898951989);
  VSET(13, e64, m1);
  write_csr(vstart, 2);
  asm volatile("vse64.v v7, (%0)" ::"r"(ALIGNED_I64));
  VVCMP_U64(14, ALIGNED_I64, 1, 2, 0xa11a9384a7163840, 0x99991348a9f38cd1,
            0x9fa831c7a11a9384, 0x3819759853987548, 0x1893179501093489,
            0x81937598aa819388, 0x1874754791888188, 0x3eeeeeeee33111ae,
            0x9013930148815808, 0xab8b914891484891, 0x9031850931584902, 14, 15,
            16);
}

//****Checking functionality with different values of EMUL and
// large number of elements *******//
void TEST_CASE15(void) {
  reset_vec64(ALIGNED_I64);
  VSET(512, e64, m8);
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_I64[0]));
  asm volatile("vse64.v v8, (%0)" ::"r"(ALIGNED_I64));
  LVVCMP_U64(15, ALIGNED_I64, LONG_I64);
}

void TEST_CASE16(void) {
  reset_vec64(ALIGNED_I64);
  VSET(256, e64, m4);
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_I64[0]));
  asm volatile("vse64.v v8, (%0)" ::"r"(ALIGNED_I64));
  LVVCMP_U64(16, ALIGNED_I64, LONG_I64);
}

void TEST_CASE17(void) {
  reset_vec64(ALIGNED_I64);
  VSET(128, e64, m2);
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_I64[0]));
  asm volatile("vse64.v v8, (%0)" ::"r"(ALIGNED_I64));
  LVVCMP_U64(17, ALIGNED_I64, LONG_I64);
}

void TEST_CASE18(void) {
  reset_vec64(ALIGNED_I64);
  VSET(100, e64, m2);
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_I64[0]));
  asm volatile("vse64.v v8, (%0)" ::"r"(ALIGNED_I64));
  LVVCMP_U64(18, ALIGNED_I64, LONG_I64);
}

// masked load with different vstart value, assume vlen >= 2048
void TEST_CASE19(void) {
  uint64_t mask = 0xAAAAAAAAAAAAAAAA;
  uint64_t vstart = 71;
#define INIT(vstart, vl) \
  for(uint32_t i=0; i < vl; ++i){ \
    if(i < vstart) GOLD_TMP_I64[i] = 0; \
    else GOLD_TMP_I64[i] = (i % 2 == 1) ? LONG_I64[i] : 0; \
    ALIGNED_I64[i] = 0; \
  }

  INIT(vstart, 256);

  VSET(256, e64, m8);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask));
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_I64[0]));
  write_csr(vstart, vstart);
  asm volatile("vse64.v v8, (%0), v0.t" ::"r"(&ALIGNED_I64[0]));
  LVVCMP_U64(19, ALIGNED_I64, GOLD_TMP_I64);

  vstart = 192;
  INIT(vstart, 256);

  VSET(256, e64, m8);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask));
  asm volatile("vle64.v v8, (%0)" ::"r"(&LONG_I64[0]));
  write_csr(vstart, vstart);
  asm volatile("vse64.v v8, (%0), v0.t" ::"r"(&ALIGNED_I64[0]));
  LVVCMP_U64(20, ALIGNED_I64, GOLD_TMP_I64);

#undef INIT
}

int main(void) {
  INIT_CHECK();
  enable_vec();

  printf("*****Running tests for vse64.v*****\n");
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
  TEST_CASE16();
  TEST_CASE17();
  TEST_CASE18();
  TEST_CASE19();

  EXIT_CHECK();
}
