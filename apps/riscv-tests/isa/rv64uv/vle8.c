// TODO uncomment TEST_CASE12 and TEST_CASE 14 after issue of vl=0 and
// non-zero vstart is resolved
// TODO uncomment TEST_CASE2 after issue of exception is resolved

#include "long_array.h"
#include "vector_macros.h"

#define AXI_DWIDTH 128

static volatile uint8_t GOLD_TMP_I8[512] __attribute__((aligned(AXI_DWIDTH))) = {};

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

static volatile uint8_t ALIGNED_I8[16] __attribute__((aligned(AXI_DWIDTH))) = {
    0xe0, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x89, 0x88,
    0x88, 0xae, 0x08, 0x91, 0x02, 0x59, 0x11, 0x89};

//**********Checking functionality of vle8 ********//
void TEST_CASE1(void) {
  VSET(15, e8, m1);
  asm volatile("vle8.v v0, (%0)" ::"r"(&ALIGNED_I8[1]));
  VCMP_U8(1, v0, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x89, 0x88, 0x88, 0xae, 0x08,
          0x91, 0x02, 0x59, 0x11, 0x89);
}

//******Checking functionality of  with illegal destination register
// specifier for EMUL********//
// In this test case EMUL=2 and register is v1 which will cause illegal
// instruction exception and set mcause = 2
void TEST_CASE2(void) {
  uint8_t mcause;
  uint64_t mtval, vstart;
  uint64_t load_addr;
  VSET(15, e16, m4);
  asm volatile("vle8.v v1, (%0)" ::"r"(&ALIGNED_I8[1]));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  XCMP(2, mcause, 2);

  load_addr = 0x803ffff0;
  VSET(128, e16, m2);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0x3333));
  asm volatile("vmv.v.x v2, %[A]" ::[A] "r"(0x3333));
  asm volatile("vle8.v v2, (%0), v0.t" ::"r"(load_addr));
  asm volatile("addi %[A], t3, 0" : [A] "=r"(mcause));
  asm volatile("addi %[A], t4, 0" : [A] "=r"(mtval));
  vstart = read_csr(vstart);
  write_csr(vstart, 0);

  XCMP(2, mcause, 13);
  XCMP(2, mtval, 0x80400000);
  XCMP(2, vstart, 16);
  VSET(16, e8, m1);
  uint8_t* p = (uint8_t*)load_addr;
  VCMP_U8(2, v2, p[0], p[1], 0x33, 0x33, p[4], p[5], 0x33, 0x33,
          p[8], p[9], 0x33, 0x33, p[12], p[13], 0x33, 0x33);
}

//*******Checking functionality of vle8 with different values of masking
// register******//
void TEST_CASE3(void) {
  VSET(16, e8, m1);
  VCLEAR(v3);
  VLOAD_8(v0, 0xFF, 0xFF);
  asm volatile("vle8.v v3, (%0), v0.t" ::"r"(&ALIGNED_I8[0]));
  VCMP_U8(3, v3, 0xe0, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x89, 0x88, 0x88, 0xae,
          0x08, 0x91, 0x02, 0x59, 0x11, 0x89);
}

void TEST_CASE4(void) {
  VSET(16, e8, m1);
  VLOAD_8(v3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VLOAD_8(v0, 0x00, 0x00);
  asm volatile("vle8.v v3, (%0), v0.t" ::"r"(&ALIGNED_I8[0]));
  VCMP_U8(4, v3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
}

void TEST_CASE5(void) {
  VSET(16, e8, m1);
  VCLEAR(v3);
  VLOAD_8(v3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VLOAD_8(v0, 0xAA, 0xAA);
  asm volatile("vle8.v v3, (%0), v0.t" ::"r"(&ALIGNED_I8[0]));
  VCMP_U8(5, v3, 1, 0xd3, 3, 0xd1, 5, 0x48, 7, 0x88, 9, 0xae, 11, 0x91, 13,
          0x59, 15, 0x89);
}

//******Checking functionality with different combinations of vta and vma*****//
// **** It uses undisturbed policy for tail agnostic and mask agnostic****//
void TEST_CASE6(void) {
  uint64_t avl;
  VSET(16, e8, m1);
  VLOAD_8(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VLOAD_8(v0, 0xAA, 0xAA);
  __asm__ volatile("vsetivli %[A], 12, e8, m1, ta, ma" : [A] "=r"(avl));
  asm volatile("vle8.v v4, (%0), v0.t" ::"r"(&ALIGNED_I8[0]));
  VSET(16, e8, m1);
  VCMP_U8(6, v4, 1, 0xd3, 3, 0xd1, 5, 0x48, 7, 0x88, 9, 0xae, 11, 0x91, 13, 14,
          15, 16);
}

void TEST_CASE7(void) {
  uint64_t avl;
  VSET(16, e8, m1);
  VLOAD_8(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VLOAD_8(v0, 0xAA, 0xAA);
  __asm__ volatile("vsetivli %[A], 12, e8, m1, ta, mu" : [A] "=r"(avl));
  asm volatile("vle8.v v4, (%0), v0.t" ::"r"(&ALIGNED_I8[0]));
  VSET(16, e8, m1);
  VCMP_U8(7, v4, 1, 0xd3, 3, 0xd1, 5, 0x48, 7, 0x88, 9, 0xae, 11, 0x91, 13, 14,
          15, 16);
}

void TEST_CASE8(void) {
  uint64_t avl;
  VSET(16, e8, m1);
  VLOAD_8(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VLOAD_8(v0, 0xAA, 0xAA);
  __asm__ volatile("vsetivli %[A], 12, e8, m1, tu, ma" : [A] "=r"(avl));
  asm volatile("vle8.v v4, (%0), v0.t" ::"r"(&ALIGNED_I8[0]));
  VSET(16, e8, m1);
  VCMP_U8(8, v4, 1, 0xd3, 3, 0xd1, 5, 0x48, 7, 0x88, 9, 0xae, 11, 0x91, 13, 14,
          15, 16);
}

void TEST_CASE9(void) {
  uint64_t avl;
  VSET(16, e8, m1);
  VLOAD_8(v4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VLOAD_8(v0, 0xAA, 0xAA);
  __asm__ volatile("vsetivli %[A], 12, e8, m1, tu, mu" : [A] "=r"(avl));
  asm volatile("vle8.v v4, (%0), v0.t" ::"r"(&ALIGNED_I8[0]));
  VSET(16, e8, m1);
  VCMP_U8(9, v4, 1, 0xd3, 3, 0xd1, 5, 0x48, 7, 0x88, 9, 0xae, 11, 0x91, 13, 14,
          15, 16);
}

//*******Checking functionality if encoded EEW is not supported for given SEW
// and LMUL values because EMUL become out of range*****//
// This test case execute lower bound case of EMUL (1/8). If LMUL is changed to
// mf4 or mf8 it will give error because emul become out of range
void TEST_CASE10(void) {
  VSET(15, e32, mf2);
  asm volatile("vle8.v v5, (%0)" ::"r"(&ALIGNED_I8[1]));
  VCMP_U8(10, v5, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x89, 0x88, 0x88, 0xae, 0x08,
          0x91, 0x02, 0x59, 0x11, 0x89);
}

//******Checking functionality with different values of vl******//
void TEST_CASE11(void) {
  VSET(16, e8, m1);
  VLOAD_8(v6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VSET(16, e8, m1); // Setting vl=16
  asm volatile("vle8.v v6, (%0)" ::"r"(&ALIGNED_I8[0]));
  VSET(16, e8, m1);
  VCMP_U8(11, v6, 0xe0, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x89, 0x88, 0x88, 0xae,
          0x08, 0x91, 0x02, 0x59, 0x11, 0x89);
}

void TEST_CASE12(void) {
  uint64_t avl;
  VSET(16, e8, m1);
  VLOAD_8(v6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  __asm__ volatile("vsetivli %[A], 0, e8, m1, ta, ma"
                   : [A] "=r"(avl)); // Setting vl=0
  asm volatile("vle8.v v6, (%0)" ::"r"(&ALIGNED_I8[0]));
  VSET(16, e8, m1);
  VCMP_U8(12, v6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
}

void TEST_CASE13(void) {
  VSET(16, e8, m1);
  VLOAD_8(v6, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  VSET(13, e8, m1); // Setting vl =13
  asm volatile("vle8.v v6, (%0)" ::"r"(&ALIGNED_I8[0]));
  VSET(16, e8, m1);
  VCMP_U8(13, v6, 0xe0, 0xd3, 0x40, 0xd1, 0x84, 0x48, 0x89, 0x88, 0x88, 0xae,
          0x08, 0x91, 0x02, 14, 15, 16);
}

//******Checking functionality with different vstart value*****//
void TEST_CASE14(void) {
  uint64_t vstart;
  VSET(16, e8, m1);
  VLOAD_8(v7, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  write_csr(vstart, 2);
  asm volatile("vle8.v v7, (%0)" ::"r"(&ALIGNED_I8[0]));
  write_csr(vstart, 0);
  VSET(16, e8, m1);
  VCMP_U8(14, v7, 1, 2, 0x40, 0xd1, 0x84, 0x48, 0x89, 0x88, 0x88, 0xae, 0x08,
          0x91, 0x02, 0x59, 0x11, 0x89);
}

//****Checking functionality with different values of EMUL and
// large number of elements *******//
void TEST_CASE15(void) {
  VSET(1024, e8, m2);
  asm volatile("vle8.v v8, (%0)" ::"r"(&LONG_I8[0]));
  LVCMP_U8(15, v8, LONG_I8);
}

void TEST_CASE16(void) {
  VSET(800, e8, m2);
  asm volatile("vle8.v v8, (%0)" ::"r"(&LONG_I8[0]));
  LVCMP_U8(16, v8, LONG_I8);
}

// masked load with different vstart value, assume vlen >= 2048
void TEST_CASE17(void) {
  uint8_t mask = 0xAA;
  uint64_t vstart = 67;
#define INIT(vstart, vl) \
  for(uint32_t i=0; i < vl; ++i){ \
    if(i < vstart) GOLD_TMP_I8[i] = 0; \
    else GOLD_TMP_I8[i] = (i % 2 == 1) ? LONG_I8[i] : 0; \
  }

  INIT(vstart, 256);

  VSET(256, e8, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask));
  asm volatile("vmv.v.x v3, %[A]" ::[A] "r"(0));
  write_csr(vstart, vstart);
  asm volatile("vle8.v v3, (%0), v0.t" ::"r"(&LONG_I8[0]));
  LVCMP_U8(17, v3, GOLD_TMP_I8);

  vstart = 178;
  INIT(vstart, 256);

  VSET(256, e8, m1);
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(mask));
  asm volatile("vmv.v.x v3, %[A]" ::[A] "r"(0));
  write_csr(vstart, vstart);
  asm volatile("vle8.v v3, (%0), v0.t" ::"r"(&LONG_I8[0]));
  LVCMP_U8(18, v3, GOLD_TMP_I8);

#undef INIT
}

// check hazard detection with implicit register dependency
void TEST_CASE18(void) {
  VSET(256, e64, m8);
  // Use the first vle8 instruction to block execution of vmv.
  // If we can't detect implicit register, the second vle8 will
  // be executed before vmv.
  asm volatile("vle8.v v0, (%0)" ::"r"(&LONG_I8[0]));
  asm volatile("vmv.v.x v0, %[A]" ::[A] "r"(0));
  // vle8 should be stalled because it takes no operand and has hazard
  asm volatile("vle8.v v7, (%0)" ::"r"(&LONG_I8[0]));
  LVCMP_U8(19, v7, LONG_I8);
}

int main(void) {
  INIT_CHECK();
  enable_vec();

  printf("*****Running tests for vle8.v*****\n");
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

  EXIT_CHECK();
}
