PROGS = armemu

OBJS_ARMEMU = add_s.o mov_s.o sub_s.o cmp_s.o
CFLAGS = -g

%.o : %.s
	as -o $@ $<

%.o : %.c
	gcc -c ${CFLAGS} -o $@ $<

all : ${PROGS}

armemu : armemu.c ${OBJS_ARMEMU}
	gcc -o armemu armemu.c ${OBJS_ARMEMU}

clean :
	rm -rf ${PROGS} ${OBJS_PLAYINST}
