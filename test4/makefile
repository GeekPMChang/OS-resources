hdrs = ipc.h
opts = -g -c
c1_src = consumer1.c ipc.c
c1_obj = consumer1.o ipc.o
c2_src = consumer2.c ipc.c
c2_obj = consumer2.o ipc.o
c3_src = consumer3.c ipc.c
c3_obj = consumer3.o ipc.o
p_src = producer.c ipc.c
p_obj = producer.o ipc.o
all:	producer consumer1 consumer2 consumer3

consumer1:	$(c1_obj)
			gcc $(c1_obj) -o consumer1
consumer1.o:	$(c1_src) $(hdrs)
			gcc $(opts) $(c1_src)

consumer2:	$(c2_obj)
			gcc $(c2_obj) -o consumer2
consumer2.o:	$(c2_src) $(hdrs)
			gcc $(opts) $(c2_src)

consumer3:	$(c3_obj)
			gcc $(c3_obj) -o consumer3
consumer3.o:	$(c3_src) $(hdrs)
			gcc $(opts) $(c3_src)

producer:$(p_obj)
			gcc $(p_obj) -o producer
producer.o:	$(p_src) $(hdrs)
			gcc $(opts) $(p_src)
clean:
			rm consumer producer *.o
