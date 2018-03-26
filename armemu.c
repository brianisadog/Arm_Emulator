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
    dp_cmp = 0b1010
};

struct arm_state {
    unsigned int regs[REG_NUM];
    unsigned int cpsr;
    unsigned char stack[STACK_SIZE];
};

int add_s(int, int);
int mov_s(int, int);
int sub_s(int, int);
int hit_five_s(int);

void arm_state_init(struct arm_state *, unsigned int *, unsigned int,
                    unsigned int, unsigned int, unsigned int);
void arm_state_print(struct arm_state *);
unsigned int armemu(struct arm_state *);
void armemu_one(struct arm_state *);
unsigned int get_type(unsigned int);
bool check_cond(struct arm_state *, unsigned int);

void armemu_dp(struct arm_state *, unsigned int);
void armemu_mem(struct arm_state *, unsigned int);
void armemu_brch(struct arm_state *, unsigned int);

bool is_bx(unsigned int);
bool is_str(unsigned int);
bool is_ldr(unsigned int);
bool is_strb(unsigned int);
bool is_ldrb(unsigned int);

void armemu_mov(struct arm_state *, unsigned int, unsigned int);
void armemu_add(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_sub(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_cmp(struct arm_state *, unsigned int, unsigned int, unsigned int);
void armemu_bx(struct arm_state *);
void armemu_b(struct arm_state *, unsigned int);
void armemu_str(struct arm_state *);

int main(int argc, char **argv) {
    struct arm_state as;
    unsigned int result;

    printf("mov_s(1, 2):\n");
    arm_state_init(&as, (unsigned int *) mov_s, 1, 2, 0, 0);
    result = armemu(&as);
    printf("r0 = %d\n", result);

    printf("add_s(1, 2):\n");
    arm_state_init(&as, (unsigned int *) add_s, 1, 2, 0, 0);
    result = armemu(&as);
    printf("r0 = %d\n", result);

    printf("sub_s(5, 2):\n");
    arm_state_init(&as, (unsigned int *) sub_s, 5, 2, 0, 0);
    result = armemu(&as);
    printf("r0 = %d\n", result);

    printf("hit_five_s(1):\n");
    arm_state_init(&as, (unsigned int *) hit_five_s, 1, 0, 0, 0);
    result = armemu(&as);
    printf("r0 = %d\n", result);
    
    return 0;
}

void arm_state_init(struct arm_state *as, unsigned int *func,
                    unsigned int arg0, unsigned int arg1,
                    unsigned int arg2, unsigned int arg3) {
    int i;

    for (i = 0; i < REG_NUM; i++) {
        as->regs[i] = 0;
    }
    as->cpsr = 0;

    for (i = 0; i < STACK_SIZE; i++) {
        as->stack[i] = 0;
    }

    as->regs[0] = arg0;
    as->regs[1] = arg1;
    as->regs[2] = arg2;
    as->regs[3] = arg3;
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
    printf("iw = %x\n", iw);

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
    }

    as->regs[PC] += 4;
}

void armemu_mem(struct arm_state *as, unsigned int iw) {
    printf("mem!\n");
}

void armemu_brch(struct arm_state *as, unsigned int iw) {
    unsigned int link, imm24;

    if (is_bx(iw)) {
        armemu_bx(as);
    }
    else {
        link = (iw >> 24) & 0b1;
        imm24 = iw & 0xFFFFFF;
        
        if (link == 0) {
            armemu_b(as, imm24);
        }
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
    unsigned int n, z, v, msb_rn, msb_src2, cpsr;
    
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

void armemu_b(struct arm_state * as, unsigned int imm24) {
    unsigned int negative, offset;

    negative = (imm24 >> 23) & 0b1;

    if (negative == 0b1) {
        imm24 = (imm24 | 0xFF000000);
    }

    offset = (imm24 << 2) + 8;

    as->regs[PC] += offset;
}

bool is_str(unsigned int iw) {
    unsigned int op;
    unsigned int load;
    unsigned int byte;

    op = (iw >> 26) & 0b11;
    load = (iw >> 20) & 0b1;
    byte = (iw >> 22) & 0b1;

    return (op == 0b01) && (load == 0) && (byte == 0);
}

void armemu_str(struct arm_state *as) {
    unsigned int iw;
    unsigned int i, p, u, w;
    unsigned int rn, rd, imm;

    iw = *((unsigned int *) as->regs[PC]);
    i = (iw >> 25) & 0b1;
    p = (iw >> 24) & 0b1;
    u = (iw >> 23) & 0b1;
    w = (iw >> 21) & 0b1;
    rn = (iw >> 16) & 0xF;
    rd = (iw >> 12) & 0xF;

    if (i == 0) {
        imm = iw & 0xFFF;
        as->regs[rd] = as->regs[rd];
    }

    as->regs[PC] = as->regs[rn];
}

bool is_ldr(unsigned int iw) {
    unsigned int op;
    unsigned int load;
    unsigned int byte;

    op = (iw >> 26) & 0b11;
    load = (iw >> 20) & 0b1;
    byte = (iw >> 22) & 0b1;

    return (op == 0b01) && (load == 1) && (byte == 0);
}

bool is_strb(unsigned int iw) {
    unsigned int op;
    unsigned int load;
    unsigned int byte;

    op = (iw >> 26) & 0b11;
    load = (iw >> 20) & 0b1;
    byte = (iw >> 22) & 0b1;

    return (op == 0b01) && (load == 0) && (byte == 1);
}

bool is_ldrb(unsigned int iw) {
    unsigned int op;
    unsigned int load;
    unsigned int byte;

    op = (iw >> 26) & 0b11;
    load = (iw >> 20) & 0b1;
    byte = (iw >> 22) & 0b1;

    return (op == 0b01) && (load == 1) && (byte == 1);
}
