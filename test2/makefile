srcs = pct2.c
objs = pct2.o 
opts = -g -c
all: pct2
pct2: $(objs)
	gcc $(objs) -o pct2
pct2.o: $(srcs)
	gcc $(opts) $(srcs)
clean:
	rm pct2 *.o

