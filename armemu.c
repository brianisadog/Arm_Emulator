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

void arm_state_init(struct arm_state *, unsigned int *, unsigned int,
                    unsigned int, unsigned int, unsigned int);
void arm_state_print(struct arm_state *);
unsigned int armemu(struct arm_state *);
void armemu_one(struct arm_state *);
bool is_bx(unsigned int);
void armemu_bx(struct arm_state *);
bool is_mov(unsigned int);
void armemu_mov(struct arm_state *);
bool is_add(unsigned int);
void armemu_add(struct arm_state *);

int main(int argc, char **argv) {
    struct arm_state as;
    unsigned int result;

    printf("add_s(1, 2):\n");
    arm_state_init(&as, (unsigned int *) add_s, 1, 2, 0, 0);
    result = armemu(&as);
    printf("r0 = %d\n", result);

    printf("mov_s(1, 2):\n");
    arm_state_init(&as, (unsigned int *) mov_s, 1, 2, 0, 0);
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

    if (is_bx(iw)) {
        armemu_bx(as);
    }
    else if (is_add(iw)) {
        armemu_add(as);
    }
    else if (is_mov(iw)) {
        armemu_mov(as);
    }
}

bool is_bx(unsigned int iw) {
    unsigned int bx_bin_code;

    bx_bin_code = (iw >> 4) & 0x00FFFFFF;

    return (bx_bin_code == 0b000100101111111111110001);
}

void armemu_bx(struct arm_state *as) {
    unsigned int iw;
    unsigned int rn;

    iw = *((unsigned int *) as->regs[PC]);
    rn = iw & 0b1111;

    as->regs[PC] = as->regs[rn];
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
    unsigned int rd, rm;

    iw = *((unsigned int *) as->regs[PC]);
    rd = (iw >> 12) & 0xF;
    rm = iw & 0xF;

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
    unsigned int rd, rn, rm;

    iw = *((unsigned int *) as->regs[PC]);
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;
    rm = iw & 0xF;

    as->regs[rd] = as->regs[rn] + as->regs[rm];

    if (rd != PC) {
        as->regs[PC] += 4;
    }
}
