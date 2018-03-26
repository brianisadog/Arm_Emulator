#include<stdio.h>
#include<stdbool.h>

#define REG_NUM 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

enum condition {
    cond_al = 0b1110,
    cond_eq = 0b0000,
    cond_ne = 0b0001,
    cond_gt = 0b1100,
    cond_le = 0b1101
};

enum operation {
    op_dp = 0b00,
    op_mem = 0b01,
    op_brch = 0b10
};

enum dp_func {
    dp_mov = 0b1101,
    dp_add = 0b0100,
    dp_sub = 0b0010,
    dp_cmp = 0b1010,
    dp_mvn = 0b1111,
    dp_cmn = 0b1011
};

struct arm_state {
    unsigned int regs[REG_NUM];
    unsigned int cpsr;
    unsigned char stack[STACK_SIZE];
};

int sum_array_s(int *, int);
int find_max_s(int *, int);
int fib_iter_s(int *, int);
int fib_rec_s(int *, int);
int find_str_s(char *, char *);
void test_sum_array();
void test_find_max();
void test_fib_iter();
void test_fib_rec();
void test_find_str();

void arm_state_init(struct arm_state *, unsigned int *, unsigned int,
                    unsigned int, unsigned int, unsigned int);
void arm_state_print(struct arm_state *);
unsigned int armemu(struct arm_state *);
void armemu_one(struct arm_state *);
unsigned int get_type(unsigned int);
bool check_cond(struct arm_state *, unsigned int);
unsigned int mem_shift(unsigned int, unsigned int, unsigned int);
bool is_bx(unsigned int);
void armemu_dp(struct arm_state *, unsigned int);
void armemu_mem(struct arm_state *, unsigned int);
void armemu_brch(struct arm_state *, unsigned int);
void armemu_mov(struct arm_state *, unsigned int, unsigned int);
void armemu_add(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_sub(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_cmp(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_mvn(struct arm_state *, unsigned int, unsigned int);
void armemu_cmn(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_bx(struct arm_state *);
void armemu_b(struct arm_state *, unsigned int, unsigned int);
void armemu_str(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_ldr(struct arm_state *, unsigned int, unsigned int, unsigned int, unsigned int);

int main(int argc, char **argv) {
    //test_sum_array();
    //test_find_max();
    //test_fib_iter();
    //test_fib_rec();
    test_find_str();
    
    return 0;
}

void test_sum_array() {
    struct arm_state as;
    unsigned int result;
    int n = 4;
    int array[4] = {1, 2, 3, 4};

    arm_state_init(&as, (unsigned int *) sum_array_s, (unsigned int) array, n, 0, 0);
    result = armemu(&as);
    printf("sum_array_s({1, 2, 3, 4}, %d) = %d\n", n, result);
}

void test_find_max() {
    struct arm_state as;
    unsigned int result;
    int n = 4;
    int array[4] = {1, 4, 3, 2};

    arm_state_init(&as, (unsigned int *) find_max_s, (unsigned int) array, n, 0, 0);
    result = armemu(&as);
    printf("find_max_s({1, 4, 3, 2}, %d) = %d\n", n, result);
}

void test_fib_iter() {
    struct arm_state as;
    unsigned int result;
    int n = 20;

    arm_state_init(&as, (unsigned int *) fib_iter_s, n, 0, 0, 0);
    result = armemu(&as);
    printf("fib_iter_s(%d) = %d\n", n, result);
}

void test_fib_rec() {
    struct arm_state as;
    unsigned int result;
    int n = 20;

    arm_state_init(&as, (unsigned int *) fib_rec_s, n, 0, 0, 0);
    result = armemu(&as);
    printf("fib_rec_s(%d) = %d\n", n, result);
}

void test_find_str() {
    struct arm_state as;
    unsigned int result;
    char *s = "Computer Science is actually art!";
    char *sub = "art";

    arm_state_init(&as, (unsigned int *) find_str_s, (unsigned int) s, (unsigned int) sub, 0, 0);
    result = armemu(&as);
    printf("find_str_s(\"%s\", \"%s\") = %d\n", s, sub, result);
}

void arm_state_init(struct arm_state *as, unsigned int *func, unsigned int reg0,
                    unsigned int reg1, unsigned int reg2, unsigned int reg3) {
    int i;

    for (i = 0; i < REG_NUM; i++) {
        as->regs[i] = 0;
    }
    as->cpsr = 0;

    for (i = 0; i < STACK_SIZE; i++) {
        as->stack[i] = 0;
    }

    as->regs[0] = reg0;
    as->regs[1] = reg1;
    as->regs[2] = reg2;
    as->regs[3] = reg3;
    as->regs[SP] = (unsigned int) &as->stack[STACK_SIZE];
    as->regs[LR] = 0;
    as->regs[PC] = (unsigned int) func;
}

void arm_state_print(struct arm_state *as) {
    int i;

    for (i = 0; i < REG_NUM; i++) {
        printf("reg[%d] = %d\n", i, as->regs[i]);
    }
    printf("cpsr = %x\n", as->cpsr);
}

unsigned int armemu(struct arm_state *as) {
    while (as->regs[PC] != 0) {
        armemu_one(as);
    }

    return as->regs[0];
}

void armemu_one(struct arm_state *as) {
    unsigned int iw, type;
    bool exec;

    iw = *((unsigned int *) as->regs[PC]);
    type = get_type(iw);
    exec = check_cond(as, iw);

    if (exec) {
        switch (type) {
            case op_dp:
                armemu_dp(as, iw);
                break;
            case op_mem:
                armemu_mem(as, iw);
                break;
            case op_brch:
                armemu_brch(as, iw);
        }
    }
    else {
        as->regs[PC] += 4;
    }
}

unsigned int get_type(unsigned int iw) {
    unsigned int op;
    
    op = (iw >> 26) & 0b11;

    if (((iw >> 4) & 0xFFFFFF) == 0b000100101111111111110001) {
        op = 0b10; //bx considered as branch
    }

    return op;
}

bool check_cond(struct arm_state *as, unsigned int iw) {
    unsigned int cond, cpsr, n, z, v;
    bool exec = false;
    
    cond = (iw >> 28) & 0xF;
    cpsr = as->cpsr;
    n = (cpsr >> 3) & 0b1;
    z = (cpsr >> 2) & 0b1;
    v = cpsr & 0b1;

    if (
        (cond == cond_al) ||
        (cond == cond_eq && z == 1) ||
        (cond == cond_ne && z == 0) ||
        (cond == cond_gt && z == 0 && n == v) ||
        (cond == cond_le && (z == 1 || n != v))
        ) {
        exec = true;
    }

    return exec;
}

unsigned int mem_shift(unsigned int value, unsigned int sh, unsigned int shamt5) {
    if (sh == 0) {
        value <<= shamt5;
    }

    return value;
}

void armemu_dp(struct arm_state *as, unsigned int iw) {
    unsigned int opcode;
    unsigned int i, set, rn, rd, rm, imm8, src2;
    
    opcode = (iw >> 21) & 0xF;
    i = (iw >> 25) & 0b1;
    set = (iw >> 20) & 0b1;
    rn = (iw >> 16) & 0xF;
    rd = (iw >> 12) & 0xF;
    
    if (i == 0) {
        rm = iw & 0xF;
        src2 = as->regs[rm];
    }
    else {
        imm8 = iw & 0xFF;
        if (((imm8 >> 7) & 0b1) == 0b1) {
            src2 = (imm8 | 0xFFFFFF00);
        }
        else {
            src2 = imm8;
        }
    }

    switch (opcode) {
        case dp_mov:
            armemu_mov(as, rd, src2);
            break;
        case dp_add:
            armemu_add(as, rd, rn, src2);
            break;
        case dp_sub:
            armemu_sub(as, rd, rn, src2);
            break;
        case dp_cmp:
            armemu_cmp(as, set, rn, src2);
            break;
        case dp_mvn:
            armemu_mvn(as, rd, src2);
            break;
        case dp_cmn:
            armemu_cmn(as, set, rn, src2);
    }

    as->regs[PC] += 4;
}

void armemu_mem(struct arm_state *as, unsigned int iw) {
    unsigned int i, b, l;
    unsigned int rn, rd, rm, sh, shamt5, imm12, src2;

    i = (iw >> 25) & 0b1;
    b = (iw >> 22) & 0b1;
    l = (iw >> 20) & 0b1;
    rn = (iw >> 16) & 0xF;
    rd = (iw >> 12) & 0xF;

    if (i == 0) {
        imm12 = iw & 0xFFF;
        if (((imm12 >> 11) & 0b1) == 0b1) {
            src2 = (imm12 | 0xFFFFF000);
        }
        else {
            src2 = imm12;
        }
    }
    else {
        rm = iw & 0xF;
        sh = (iw >> 5) & 0b11;
        shamt5 = (iw >> 7) & 0b11111;
        src2 = mem_shift(as->regs[rm], sh, shamt5);
    }

    if (l == 0b1) {
        armemu_ldr(as, b, rn, rd, src2);
    }
    else {
        armemu_str(as, rn, rd, src2);
    }

    as->regs[PC] += 4;
}

void armemu_brch(struct arm_state *as, unsigned int iw) {
    unsigned int link, imm24;

    if (is_bx(iw)) {
        armemu_bx(as);
    }
    else {
        link = (iw >> 24) & 0b1;
        imm24 = iw & 0xFFFFFF;
        armemu_b(as, imm24, link);
    }
}

void armemu_mov(struct arm_state *as, unsigned int rd, unsigned int src2) {
    as->regs[rd] = src2;
}

void armemu_add(struct arm_state *as, unsigned int rd, unsigned int rn, unsigned int src2) {
    as->regs[rd] = as->regs[rn] + src2;
}

void armemu_sub(struct arm_state *as, unsigned int rd, unsigned int rn, unsigned int src2) {
    as->regs[rd] = as->regs[rn] - src2;
}

void armemu_cmp(struct arm_state *as, unsigned int set, unsigned int rn, unsigned int src2) {
    unsigned int n, z, v;
    unsigned int msb_rn, msb_src2, cpsr;
    
    rn = as->regs[rn];
    src2 = (src2 ^ 0xFFFFFFFF) + 0b1;
    n = ((rn + src2) >> 31) & 0b1;
    z = (rn + src2 == 0) ? 1 : 0;
    msb_rn = (rn >> 31) & 0b1;
    msb_src2 = (src2 >> 31) & 0b1;
    if (msb_rn != msb_src2) {
        v = 0;
    }
    else {
        v = (n != msb_rn) ? 1 : 0;
    }

    if (set == 0b1) {
        cpsr = (n << 3) + (z << 2) + v;
        as->cpsr = cpsr;
    }
}

void armemu_mvn(struct arm_state *as, unsigned int rd, unsigned int src2) {
    as->regs[rd] = ~(src2);
}

void armemu_cmn(struct arm_state *as, unsigned int set, unsigned int rn, unsigned int src2) {
    unsigned int n, z, v;
    unsigned int msb_rn, msb_src2, cpsr;
    
    rn = as->regs[rn];
    n = ((rn + src2) >> 31) & 0b1;
    z = (rn + src2 == 0) ? 1 : 0;
    msb_rn = (rn >> 31) & 0b1;
    msb_src2 = (src2 >> 31) & 0b1;
    if (msb_rn != msb_src2) {
        v = 0;
    }
    else {
        v = (n != msb_rn) ? 1 : 0;
    }

    if (set == 0b1) {
        cpsr = (n << 3) + (z << 2) + v;
        as->cpsr = cpsr;
    }
}

bool is_bx(unsigned int iw) {
    unsigned int bx_code;

    bx_code = (iw >> 4) & 0xFFFFFF;

    return (bx_code == 0b000100101111111111110001);
}

void armemu_bx(struct arm_state *as) {
    unsigned int iw;
    unsigned int rn;

    iw = *((unsigned int *) as->regs[PC]);
    rn = iw & 0xF;

    as->regs[PC] = as->regs[rn];
}

void armemu_b(struct arm_state *as, unsigned int imm24, unsigned int link) {
    unsigned int msb, offset;

    if (link == 0b1) {
        as->regs[LR] = as->regs[PC] + 4;
    }
    
    msb = (imm24 >> 23) & 0b1;
    if (msb == 0b1) {
        imm24 = (imm24 | 0xFF000000);
    }

    offset = (imm24 << 2) + 8;
    as->regs[PC] += offset;
}

void armemu_str(struct arm_state *as, unsigned int rn, unsigned int rd, unsigned int src2) {
    unsigned int *address;

    address = (unsigned int *) (as->regs[rn] + src2);
    *address = as->regs[rd];
}

void armemu_ldr(struct arm_state *as, unsigned int b,
                unsigned int rn, unsigned int rd, unsigned int src2) {
    unsigned int *src;

    src = (unsigned int *) (as->regs[rn] + src2);

    if (b == 0b1) {
        as->regs[rd] = *src & 0xFF;
    }
    else {
        as->regs[rd] = *src;
    }
}
