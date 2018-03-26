PROGS = armemu

OBJS_ARMEMU = sum_array_s.o find_max_s.o fib_iter_s.o fib_rec_s.o find_str_s.o
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
