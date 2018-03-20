#include<stdio.h>
#include<stdbool.h>

#define REG_NUM 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

struct arm_state {
    unsigned int regs[REG_NUM];
    unsigned int cpsr;
    unsigned char stack[STACK_SIZE];
};

int add_s(int, int);
int mov_s(int, int);
int sub_s(int, int);

void arm_state_init(struct arm_state *, unsigned int *, unsigned int,
                    unsigned int, unsigned int, unsigned int);
void arm_state_print(struct arm_state *);
unsigned int armemu(struct arm_state *);
void armemu_one(struct arm_state *);

bool is_mov(unsigned int);
bool is_add(unsigned int);
bool is_sub(unsigned int);
bool is_cmp(unsigned int);
bool is_b(unsigned int);
bool is_bl(unsigned int);
bool is_bx(unsigned int);
bool is_str(unsigned int);
bool is_ldr(unsigned int);
bool is_strb(unsigned int);
bool is_ldrb(unsigned int);
bool is_eq(unsigned int);
bool is_ne(unsigned int);
bool is_le(unsigned int);
bool is_gt(unsigned int);

void armemu_bx(struct arm_state *);
void armemu_mov(struct arm_state *);
void armemu_add(struct arm_state *);
void armemu_sub(struct arm_state *);
void armemu_cmp(struct arm_state *);
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
    unsigned int iw;

    iw = *((unsigned int *) as->regs[PC]);
    printf("iw = %x\n", iw);

    if (is_mov(iw)) {
        armemu_mov(as);
    }
    else if (is_add(iw)) {
        armemu_add(as);
    }
    else if (is_sub(iw)) {
        armemu_sub(as);
    }
    else if (is_bx(iw)) {
        armemu_bx(as);
    }
}

bool is_mov(unsigned int iw) {
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0xF;

    return (op == 0) && (opcode == 0b1101);
}

void armemu_mov(struct arm_state *as) {
    unsigned int iw;
    unsigned imm;
    unsigned int rd, rm;

    iw = *((unsigned int *) as->regs[PC]);
    imm = (iw >> 25) & 0b1;
    rd = (iw >> 12) & 0xF;

    if (imm == 0) {
        rm = iw & 0xF;
    }
    else {
        rm = iw & 0xFF;
    }

    as->regs[rd] = as->regs[rm];

    if (rd != PC) {
        as->regs[PC] += 4;
    }
}

bool is_add(unsigned int iw) {
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0xF;

    return (op == 0) && (opcode == 0b0100);
}

void armemu_add(struct arm_state *as) {
    unsigned int iw;
    unsigned int imm;
    unsigned int rd, rn, rm;

    iw = *((unsigned int *) as->regs[PC]);
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

    if (rd != PC) {
        as->regs[PC] += 4;
    }
}

bool is_sub(unsigned int iw) {
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0xF;

    return (op == 0) && (opcode == 0b0010);
}

void armemu_sub(struct arm_state *as) {
    unsigned int iw;
    unsigned int imm;
    unsigned int rd, rn, rm;

    iw = *((unsigned int *) as->regs[PC]);
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
    
    if (rd != PC) {
        as->regs[PC] += 4;
    }
}

bool is_cmp(unsigned int iw) {
    unsigned int op;
    unsigned int opcode;
    unsigned int con;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0xF;
    con = (iw >> 20) & 0b1;

    return (op == 0) && (opcode == 0b1010) && (con == 0b1);
}

void armemu_cmp(struct arm_state *as) {
    unsigned int iw;
    unsigned int imm;
    unsigned int rd, rm;

    iw = *((unsigned int *) as->regs[PC]);
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

    if (rd != PC) {
        as->regs[PC] += 4;
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
        as->regs[rd] = as->regs[rd]
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

bool is_eq(unsigned int iw) {
    unsigned int cond;

    cond = (iw >> 28) 0xF;

    return (cond == 0);
}

bool is_ne(unsigned int iw) {
    unsigned int cond;

    cond = (iw >> 28) 0xF;

    return (cond == 0b1);
}

bool is_le(unsigned int iw) {
    unsigned int cond;

    cond = (iw >> 28) 0xF;

    return (cond == 0b1101);
}

bool is_gt(unsigned int iw) {
    unsigned int cond;

    cond = (iw >> 28) 0xF;

    return (cond == 0b1100);
}