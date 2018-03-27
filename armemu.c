#include<stdio.h>
#include<stdbool.h>
#include<time.h>

#define REG_NUM 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15
#define BILLION 1000000000

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

struct instruction_count {
    int dp_count;
    int mem_count;
    int brch_count;
    int brch_taken;
    int brch_not_taken;
    int regs_read[REG_NUM];
    int regs_write[REG_NUM];
    struct timespec start;
    struct timespec finish;
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
unsigned int run(struct arm_state *, struct instruction_count *, unsigned int *,
                 unsigned int, unsigned int, unsigned int, unsigned int);
void arm_state_init(struct arm_state *, unsigned int, unsigned int,
                    unsigned int, unsigned int, unsigned int);
void instruction_count_init(struct instruction_count *);
void print_detail(struct arm_state *, struct instruction_count *);
void arm_state_print(struct arm_state *);
void instruction_count_print(struct instruction_count *);
void register_count_print(struct instruction_count *);
void time_spent_print(struct instruction_count *);
unsigned int armemu(struct arm_state *, struct instruction_count *);
void armemu_one(struct arm_state *, struct instruction_count *);
unsigned int get_type(unsigned int);
bool check_cond(struct arm_state *, unsigned int);
void update_instruction_count(struct instruction_count *, unsigned int, bool);
void update_regs_count(struct instruction_count *, unsigned int, unsigned int);
unsigned int mem_shift(unsigned int, unsigned int, unsigned int);
bool is_bx(unsigned int);
void armemu_dp(struct arm_state *, struct instruction_count *, unsigned int);
void armemu_mem(struct arm_state *, struct instruction_count *, unsigned int);
void armemu_brch(struct arm_state *, struct instruction_count *, unsigned int);
void armemu_mov(struct arm_state *, struct instruction_count *, unsigned int, unsigned int);
void armemu_add(struct arm_state *, struct instruction_count *, unsigned int, unsigned int, unsigned int);
void armemu_sub(struct arm_state *, struct instruction_count *, unsigned int, unsigned int, unsigned int);
void armemu_cmp(struct arm_state *, struct instruction_count *, unsigned int, unsigned int, unsigned int);
void armemu_mvn(struct arm_state *, struct instruction_count *, unsigned int, unsigned int);
void armemu_cmn(struct arm_state *, struct instruction_count *, unsigned int, unsigned int, unsigned int);
void armemu_bx(struct arm_state *, struct instruction_count *, unsigned int);
void armemu_b(struct arm_state *, struct instruction_count *, unsigned int, unsigned int);
void armemu_str(struct arm_state *, struct instruction_count *, unsigned int, unsigned int, unsigned int);
void armemu_ldr(struct arm_state *, struct instruction_count *, unsigned int, unsigned int, unsigned int, unsigned int);

int main(int argc, char **argv) {
    printf("***********************Sum array***********************\n");
    test_sum_array();

    printf("***********************Find max************************\n");
    test_find_max();

    printf("***********************Fib iter************************\n");
    test_fib_iter();

    printf("***********************Fib rec*************************\n");
    test_fib_rec();

    printf("***********************Find str************************\n");
    test_find_str();
    
    return 0;
}

void test_sum_array() {
    struct arm_state as;
    struct instruction_count ic;
    unsigned int result;
    int i, n;
    int array1[4] = {1, 2, 3, 4};
    int array2[4] = {-1, -2, -3, -4};
    int array3[4] = {-1, 0, 0, 1};
    int array4[1000];
    
    n = 4;
    result = run(&as, &ic, (unsigned int *) sum_array_s, (unsigned int) array1, n, 0, 0);
    printf("sum_array_s({1, 2, 3, 4}, %d) = %d\n", n, result);
    print_detail(&as, &ic);

    result = run(&as, &ic, (unsigned int *) sum_array_s, (unsigned int) array2, n, 0, 0);
    printf("sum_array_s({-1, -2, -3, -4}, %d) = %d\n", n, result);
    print_detail(&as, &ic);

    result = run(&as, &ic, (unsigned int *) sum_array_s, (unsigned int) array3, n, 0, 0);
    printf("sum_array_s({-1, 0, 0, 1}, %d) = %d\n", n, result);
    print_detail(&as, &ic);

    n = 1000;
    for (i = 0; i < n; i++) {
        array4[i] = i + 1;
    }
    /*result = run(&as, &ic, (unsigned int *) sum_array_s, (unsigned int) array4, n, 0, 0);
    printf("sum_array_s({1, 2, ..., 999, 1000}, %d) = %d\n", n, result);
    print_detail(&as, &ic);*/
}

void test_find_max() {
    struct arm_state as;
    struct instruction_count ic;
    unsigned int result;
    int i, n;
    int array1[4] = {1, 4, 3, 2};
    int array2[4] = {-3, -2, -1, -4};
    int array3[4] = {-1, 0, -2, 0};
    int array4[1000];

    n = 4;
    result = run(&as, &ic, (unsigned int *) find_max_s, (unsigned int) array1, n, 0, 0);
    printf("find_max_s({1, 4, 3, 2}, %d) = %d\n", n, result);
    print_detail(&as, &ic);

    result = run(&as, &ic, (unsigned int *) find_max_s, (unsigned int) array2, n, 0, 0);
    printf("find_max_s({-3, -2, -1, -4}, %d) = %d\n", n, result);
    print_detail(&as, &ic);

    result = run(&as, &ic, (unsigned int *) find_max_s, (unsigned int) array3, n, 0, 0);
    printf("find_max_s({-1, 0, -2, 0}, %d) = %d\n", n, result);
    print_detail(&as, &ic);

    n = 1000;
    for (i = 0; i < n; i++) {
        array4[i] = i + 1;
    }
    /*
    result = run(&as, &ic, (unsigned int *) find_max_s, (unsigned int) array4, n, 0, 0);
    printf("find_max_s({1, 2, ..., 999, 1000}, %d) = %d\n", n, result);
    print_detail(&as, &ic);*/
}

void test_fib_iter() {
    struct arm_state as;
    struct instruction_count ic;
    unsigned int result;
    int i, n;

    n = 20;
    for (i = 1; i <= n; i++) {
        result = run(&as, &ic, (unsigned int *) fib_iter_s, i, 0, 0, 0);
        printf("fib_iter_s(%d) = %d\n", i, result);
        print_detail(&as, &ic);
    }
}

void test_fib_rec() {
    struct arm_state as;
    struct instruction_count ic;
    unsigned int result;
    int i, n;

    n =20;
    for(i = 1; i <= n; i++) {
        result = run(&as, &ic, (unsigned int *) fib_rec_s, i, 0, 0, 0);
        printf("fib_rec_s(%d) = %d\n", i, result);
        print_detail(&as, &ic);
    }
}

void test_find_str() {
    struct arm_state as;
    struct instruction_count ic;
    unsigned int result;
    char *s, *sub;
    s = "Computer Science is actually art!";

    sub = "art";
    result = run(&as, &ic, (unsigned int *) find_str_s, (unsigned int) s, (unsigned int) sub, 0, 0);
    printf("find_str_s(\"%s\", \"%s\") = %d\n", s, sub, result);
    print_detail(&as, &ic);

    sub = "s actua";
    result = run(&as, &ic, (unsigned int *) find_str_s, (unsigned int) s, (unsigned int) sub, 0, 0);
    printf("find_str_s(\"%s\", \"%s\") = %d\n", s, sub, result);
    print_detail(&as, &ic);

    sub = "Computer Science";
    result = run(&as, &ic, (unsigned int *) find_str_s, (unsigned int) s, (unsigned int) sub, 0, 0);
    printf("find_str_s(\"%s\", \"%s\") = %d\n", s, sub, result);
    print_detail(&as, &ic);

    sub = "art?";
    result = run(&as, &ic, (unsigned int *) find_str_s, (unsigned int) s, (unsigned int) sub, 0, 0);
    printf("find_str_s(\"%s\", \"%s\") = %d\n", s, sub, result);
    print_detail(&as, &ic);
}

unsigned int run(struct arm_state *as, struct instruction_count *ic, unsigned int *func,
                 unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    unsigned int result;

    arm_state_init(as, (unsigned int) func, arg0, arg1, arg2, arg3);
    instruction_count_init(ic);
    result = armemu(as, ic);
    clock_gettime(CLOCK_REALTIME, &ic->finish);

    return result;
}

void arm_state_init(struct arm_state *as, unsigned int func, unsigned int reg0,
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
    as->regs[PC] = func;
}

void instruction_count_init(struct instruction_count *ic) {
    int i;

    ic->dp_count = 0;
    ic->mem_count = 0;
    ic->brch_count = 0;
    ic->brch_taken = 0;
    ic->brch_not_taken = 0;

    for (i = 0; i < REG_NUM; i++) {
        ic->regs_read[i] = 0;
        ic->regs_write[i] = 0;
    }

    ic->regs_read[PC] += 1;
    ic->regs_write[PC] += 1; //every test contains with read/write PC

    clock_gettime(CLOCK_REALTIME, &ic->start);
}

void print_detail(struct arm_state *as, struct instruction_count *ic) {
    arm_state_print(as);
    instruction_count_print(ic);
    register_count_print(ic);
    time_spent_print(ic);
}

void arm_state_print(struct arm_state *as) {
    int i;

    printf("\n*Register state:\n");
    for (i = 0; i < REG_NUM; i++) {
        printf("reg[%d] = %d\n", i, as->regs[i]);
    }
    printf("cpsr = %x\n", as->cpsr);
}

void instruction_count_print(struct instruction_count *ic) {
    printf("\n*Instruction counts:\n");
    printf("Data processing: %d\n", ic->dp_count);
    printf("Memory: %d\n", ic->mem_count);
    printf("Branches: %d (taken: %d, not-taken: %d)\n", ic->brch_count, ic->brch_taken, ic->brch_not_taken);
}

void register_count_print(struct instruction_count *ic) {
    int i;

    printf("\n*Register counts:\n");
    printf("Read:");
    for (i = 0; i < REG_NUM; i++) {
        if (ic->regs_read[i] > 0) {
            if (i < 13) {
                printf(" r%d,", i);
            }
            else {
                if (i == 13) {
                    printf(" SP,");
                }
                else if (i == 14) {
                    printf(" LR,");
                }
                else {
                    printf(" PC");
                }
            }
        }
    }
    printf("\nWrite:");
    for (i = 0; i < REG_NUM; i++) {
        if (ic->regs_write[i] > 0) {
            if (i < 13) {
                printf(" r%d,", i);
            }
            else {
                if (i == 13) {
                    printf(" SP,");
                }
                else if (i == 14) {
                    printf(" LR,");
                }
                else {
                    printf(" PC");
                }
            }
        }
    }
    printf("\n");
}

void time_spent_print(struct instruction_count *ic) {
    long long s, ns;
    double total;
    
    s = ic->finish.tv_sec - ic->start.tv_sec;
    ns = ic->finish.tv_nsec - ic->start.tv_nsec;
    total = (double) s + (double) ns / (double) BILLION;
    
    printf("\n*Time spent:\n");
    printf("%lf seconds\n", total);
    printf("--------------------------------------------------------\n\n");
}

unsigned int armemu(struct arm_state *as, struct instruction_count *ic) {
    while (as->regs[PC] != 0) {
        armemu_one(as, ic);
    }

    return as->regs[0];
}

void armemu_one(struct arm_state *as, struct instruction_count *ic) {
    unsigned int iw, type;
    bool exec;

    iw = *((unsigned int *) as->regs[PC]);
    type = get_type(iw);
    exec = check_cond(as, iw);
    update_instruction_count(ic, type, exec);

    if (exec) {
        switch (type) {
            case op_dp:
                armemu_dp(as, ic, iw);
                break;
            case op_mem:
                armemu_mem(as, ic, iw);
                break;
            case op_brch:
                armemu_brch(as, ic, iw);
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

void update_instruction_count(struct instruction_count *ic, unsigned int type, bool exec) {
    if (type == op_dp) {
        ic->dp_count += 1;
    }
    else if (type == op_mem) {
        ic->mem_count += 1;
    }
    else if (type == op_brch) {
        ic->brch_count += 1;

        if (exec) {
            ic->brch_taken += 1;
        }
        else {
            ic->brch_not_taken += 1;
        }
    }
}

void update_regs_count(struct instruction_count *ic, unsigned int type, unsigned int num) {
    if (type == 0) {
        ic->regs_read[num] += 1;
    }
    else {
        ic->regs_write[num] += 1;
    }
}

unsigned int mem_shift(unsigned int value, unsigned int sh, unsigned int shamt5) {
    if (sh == 0) {
        value <<= shamt5;
    }

    return value;
}

void armemu_dp(struct arm_state *as, struct instruction_count *ic, unsigned int iw) {
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
        update_regs_count(ic, 0, rm);
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
            armemu_mov(as, ic, rd, src2);
            break;
        case dp_add:
            armemu_add(as, ic, rd, rn, src2);
            break;
        case dp_sub:
            armemu_sub(as, ic, rd, rn, src2);
            break;
        case dp_cmp:
            armemu_cmp(as, ic, set, rn, src2);
            break;
        case dp_mvn:
            armemu_mvn(as, ic, rd, src2);
            break;
        case dp_cmn:
            armemu_cmn(as, ic, set, rn, src2);
    }

    as->regs[PC] += 4;
}

void armemu_mem(struct arm_state *as, struct instruction_count *ic, unsigned int iw) {
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
        update_regs_count(ic, 0, rm);
    }

    if (l == 0b1) {
        armemu_ldr(as, ic, b, rn, rd, src2);
    }
    else {
        armemu_str(as, ic, rn, rd, src2);
    }

    as->regs[PC] += 4;
}

void armemu_brch(struct arm_state *as, struct instruction_count *ic, unsigned int iw) {
    unsigned int rn, link, imm24;

    if (is_bx(iw)) {
        rn = iw & 0xF;
        armemu_bx(as, ic, rn);
    }
    else {
        link = (iw >> 24) & 0b1;
        imm24 = iw & 0xFFFFFF;
        armemu_b(as, ic, imm24, link);
    }
}

void armemu_mov(struct arm_state *as, struct instruction_count *ic, unsigned int rd, unsigned int src2) {
    as->regs[rd] = src2;
    update_regs_count(ic, 1, rd);
}

void armemu_add(struct arm_state *as, struct instruction_count *ic,
                unsigned int rd, unsigned int rn, unsigned int src2) {
    as->regs[rd] = as->regs[rn] + src2;
    update_regs_count(ic, 1, rd);
    update_regs_count(ic, 0, rn);
}

void armemu_sub(struct arm_state *as, struct instruction_count *ic,
                unsigned int rd, unsigned int rn, unsigned int src2) {
    as->regs[rd] = as->regs[rn] - src2;
    update_regs_count(ic, 1, rd);
    update_regs_count(ic, 0, rn);
}

void armemu_cmp(struct arm_state *as, struct instruction_count *ic,
                unsigned int set, unsigned int rn, unsigned int src2) {
    unsigned int n, z, v;
    unsigned int msb_rn, msb_src2, cpsr;

    update_regs_count(ic, 0, rn);
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

void armemu_mvn(struct arm_state *as, struct instruction_count *ic,
                unsigned int rd, unsigned int src2) {
    as->regs[rd] = ~(src2);
    update_regs_count(ic, 1, rd);
}

void armemu_cmn(struct arm_state *as, struct instruction_count *ic,
                unsigned int set, unsigned int rn, unsigned int src2) {
    unsigned int n, z, v;
    unsigned int msb_rn, msb_src2, cpsr;

    update_regs_count(ic, 0, rn);
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

void armemu_bx(struct arm_state *as, struct instruction_count *ic, unsigned int rn) {
    as->regs[PC] = as->regs[rn];
    update_regs_count(ic, 0, rn);
}

void armemu_b(struct arm_state *as, struct instruction_count *ic,
              unsigned int imm24, unsigned int link) {
    unsigned int msb, offset;

    if (link == 0b1) {
        as->regs[LR] = as->regs[PC] + 4;
        update_regs_count(ic, 1, LR);
    }
    
    msb = (imm24 >> 23) & 0b1;
    if (msb == 0b1) {
        imm24 = (imm24 | 0xFF000000);
    }

    offset = (imm24 << 2) + 8;
    as->regs[PC] += offset;
}

void armemu_str(struct arm_state *as, struct instruction_count *ic,
                unsigned int rn, unsigned int rd, unsigned int src2) {
    unsigned int *address;

    address = (unsigned int *) (as->regs[rn] + src2);
    *address = as->regs[rd];

    update_regs_count(ic, 1, rn);
    update_regs_count(ic, 0, rd);
}

void armemu_ldr(struct arm_state *as, struct instruction_count *ic, unsigned int b,
                unsigned int rn, unsigned int rd, unsigned int src2) {
    unsigned int *src;

    src = (unsigned int *) (as->regs[rn] + src2);

    if (b == 0b1) {
        as->regs[rd] = *src & 0xFF;
    }
    else {
        as->regs[rd] = *src;
    }

    update_regs_count(ic, 0, rn);
    update_regs_count(ic, 1, rd);
}
