#include<stdio.h>
#include<stdbool.h>

#define REG_NUM 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

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
int hit_five_s(int, int);

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

bool is_b(unsigned int);
bool is_bl(unsigned int);
bool is_bx(unsigned int);
bool is_str(unsigned int);
bool is_ldr(unsigned int);
bool is_strb(unsigned int);
bool is_ldrb(unsigned int);

void armemu_bx(struct arm_state *);
void armemu_mov(struct arm_state *, unsigned int);
void armemu_add(struct arm_state *, unsigned int);
void armemu_sub(struct arm_state *, unsigned int);
void armemu_cmp(struct arm_state *, unsigned int);
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
        
        if (is_bx(iw)) {
            armemu_bx(as);
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
    unsigned int cond, cpsr, n, z, c, v;
    bool exec = false;
    
    cond = (iw >> 28) & 0xF;
    cpsr = as->cpsr;
    n = cpsr >> 3;
    z = (cpsr >> 2) & 0b1;
    c = (cpsr >> 1) & 0b1;
    v = cpsr & 0b1;

    if (
        cond == 0b1110 || //AL
        cond == 0b0000 && z || //EQ
        cond == 0b0001 && ~(z) || //NE
        cond == 0b1100 && (~(z) && ~(n ^ v)) || //GT
        cond == 0b1101 && (z || (n ^ v)) //LE
        ) {
        exec = true;
    }

    return exec;
}

void armemu_dp(struct arm_state *as, unsigned int iw) {
    unsigned int opcode;
    
    opcode = (iw >> 21) & 0xF;

    switch (opcode) {
        case dp_mov:
            armemu_mov(as, iw);
            break;
        case dp_add:
            armemu_add(as, iw);
            break;
        case dp_sub:
            armemu_sub(as, iw);
            break;
        case dp_cmp:
            armemu_cmp(as, iw);
    }

    as->regs[PC] += 4;
}

void armemu_mem(struct arm_state *as, unsigned int iw) {
    printf("mem!\n");
}

void armemu_brch(struct arm_state *as, unsigned int iw) {
    printf("brch!\n");
}

void armemu_mov(struct arm_state *as, unsigned int iw) {
    unsigned imm;
    unsigned int rd, rm;

    imm = (iw >> 25) & 0b1;
    rd = (iw >> 12) & 0xF;

    if (imm == 0) {
        rm = iw & 0xF;
    }
    else {
        rm = iw & 0xFF;
    }

    as->regs[rd] = as->regs[rm];
}

void armemu_add(struct arm_state *as, unsigned int iw) {
    unsigned int imm;
    unsigned int rd, rn, rm;

    imm = (iw >> 25) & 0b1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (imm == 0) {
        rm = iw & 0xF;
        as->regs[rd] = as->regs[rn] + as->regs[rm];
    }
    else {
        imm = iw & 0xFF;
        as->regs[rd] = as->regs[rn] + imm;
    }
}

void armemu_sub(struct arm_state *as, unsigned int iw) {
    unsigned int imm;
    unsigned int rd, rn, rm;

    imm = (iw >> 25) & 0b1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (imm == 0) {
        rm = iw & 0xF;
        as->regs[rd] = as->regs[rn] - as->regs[rm];
    }
    else {
        imm = iw & 0xFF;
        as->regs[rd] = as->regs[rn] - imm;
    }
}

void armemu_cmp(struct arm_state *as, unsigned int iw) {
    unsigned int imm;
    unsigned int rd, rm;

    imm = (iw >> 25) & 0b1;
    rd = (iw >> 12) & 0xF;

    as->cpsr = 0b1;
    
    if (imm == 0) {
        rm = iw & 0xF;

        if (as->regs[rd] == as->regs[rm]) {
            as->cpsr = 0;
        }
    }
    else {
        imm = iw & 0xFF;

        if (as->regs[rd] == imm) {
            as->cpsr = 0;
        }
    }
}

bool is_b(unsigned int iw) {
    unsigned int op;
    unsigned int func;

    op = (iw >> 25) & 0b111;
    func = (iw >> 24) & 0b1;

    return (op == 0b101) && (func == 0b1);
}

bool is_bl(unsigned int iw) {
    unsigned int op;
    unsigned int func;

    op = (iw >> 25) & 0b111;
    func = (iw >> 24) & 0b1;

    return (op == 0b101) && (func == 0);
}

bool is_bx(unsigned int iw) {
    unsigned int bx_bin_code;

    bx_bin_code = (iw >> 4) & 0xFFFFFF;

    return (bx_bin_code == 0b000100101111111111110001);
}

void armemu_bx(struct arm_state *as) {
    unsigned int iw;
    unsigned int rn;

    iw = *((unsigned int *) as->regs[PC]);
    rn = iw & 0xF;

    as->regs[PC] = as->regs[rn];
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
