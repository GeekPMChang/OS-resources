#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

int fx(int x) {
	if (x == 1) return 1;
	else return fx(x - 1) * x;
}

int fy(int y) {
	if (y == 1 || y == 2) return 1;
	else return fy(y - 1) + fy(y - 2);
}

void task1(int *);
void task2(int *);
void task3(int *);

int x, y, z;
int pipe1[2], pipe2[2], pipe3[2], pipe4[2];
pthread_t thrd1, thrd2, thrd3;

int main(int argc, char *argv[]) {
	int ret;
	int num1, num2, num3;
	printf("please enter the x, y \n");
	scanf("%d %d", &x, &y);

	// pipe()系统调用四个无名管道，建立不成功就退出，执行终止
	if (pipe(pipe1) < 0) {
		perror("pipe1 not create"); 
		exit(EXIT_FAILURE);
	}
	if (pipe(pipe2) < 0) {
		perror("pipe1 not create"); 
		exit(EXIT_FAILURE);
	}
	if (pipe(pipe3) < 0) {
		perror("pipe1 not create"); 
		exit(EXIT_FAILURE);
	}
	if (pipe(pipe4) < 0) {
		perror("pipe1 not create"); 
		exit(EXIT_FAILURE);
	}

	num1 = 1;
	ret = pthread_create(&thrd1, NULL, (void *) task1, (void *) &num1);
	if (ret) {
		perror("phread_create: task1");
		exit(EXIT_FAILURE);
	}

	num2 = 2;
	ret = pthread_create(&thrd2, NULL, (void *) task2, (void *) &num2);
	if (ret) {
		perror("phread_create: task2");
		exit(EXIT_FAILURE);
	}

	num2 = 3;
	ret = pthread_create(&thrd3, NULL, (void *) task3, (void *) &num3);
	if (ret) {
		perror("phread_create: task3");
		exit(EXIT_FAILURE);
	}

	pthread_join(thrd2, NULL);
	pthread_join(thrd3, NULL);
	pthread_join(thrd1, NULL);
	exit(EXIT_SUCCESS);
}

//线程1执行函数，它首先向管道1的1端写x，管道3的1端写y，
//然后从管道2的0端读f(x)，管道4的1端读f(y)
void task1(int *num) {
	write(pipe1[1], &x, sizeof(int));
	write(pipe3[1], &y, sizeof(int));
	read(pipe2[0], &x, sizeof(int));
	read(pipe4[0], &y, sizeof(int));

	z = x + y;
	printf("Received f(x): %d\n", x);
	printf("Received f(y): %d\n", y);
	printf("The f(x,y): %d\n", z);

	close(pipe1[1]);
	close(pipe3[1]);
	close(pipe2[0]);
	close(pipe4[0]);

}

//线程2执行函数，它首先向管道1的0端读x，然后向管道2的1端写f(x)
void task2(int *num) {
	int x1;
	read(pipe1[0], &x1, sizeof(int));
	printf("Received x: %d\n", x);
	x1 = fx(x1);
	write(pipe2[1], &x1, sizeof(int)); 
	
	close(pipe1[0]);
	close(pipe2[1]);
}

//线程2执行函数，它首先向管道3的0端读y，然后向管道4的1端写f(y)
void task3(int *num) {
	int y1;
	read(pipe3[0], &y1, sizeof(int));
	printf("Received y: %d\n", y);
	y1 = fy(y1);
	write(pipe4[1], &y1, sizeof(int)); 
	
	close(pipe3[0]);
	close(pipe4[1]);
}


