head = pct1.h
srcs = pct1.c
objs = pct1.o
opts = -g -c
all: pct1
pct1: $(objs)
	gcc $(objs) -o pct1
pct1.o: $(srcs) $(head)
	gcc $(opts) $(srcs)
clean:
	rm pct1 *.o
