#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/wait.h>

void redirect(int k, int fd[2]) {
	close(k);
	dup(fd[k]);
	close(fd[0]);
	close(fd[1]);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Useage: tester ./<EXEC>\n");
		exit(-1);
	}
	int fd[2], pid;
	pipe(fd);
	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork failed\n");
		exit(-1);
	} else if(pid == 0) {
		// redirect stderr to pipe `fd[0]` as stdout
		close(2);
		dup(fd[1]);
		close(fd[0]);
		close(fd[1]);
		// close stdout
		close(1);
		// use `strace` to trace syscall
		char *argvs[] = {
			"strace",
			"-f",
			"-e",
			"trace=process",
			argv[1],
			NULL
		};
		execvp(argvs[0], argvs);
		exit(-1);
	} else {
		printf("test lab1: \n");
		// redirect stdin to `fd[0]`	
		redirect(0, fd);
		static char buf[1024];
		regmatch_t pmatch[4];
		const size_t nmatch = 4;
		const char * pattern = "\\[pid\\s*([0-9]*)\\]\\s*execve\\(\"(.*?)\", \\[";
		// const char * pattern2 = "strace: Process ([0-9]*) attached";
		// const char * pattern2 = "clone\\(.*?\\)\\s=\\s([0-9]*)";
		const char * pattern2 = "child_tidptr=.*\\)\\s*=\\s*([0-9]*)";
		regex_t reg, reg2;
		if (regcomp(&reg, pattern, REG_EXTENDED)) {
			fprintf(stderr, "regex compile failed\n");
			exit(-1);
		}
		if (regcomp(&reg2, pattern2, REG_EXTENDED)) {
			fprintf(stderr, "regex compile failed\n");
			exit(-1);
		}
		// pid of first process and second process
		char fp[30] = {0}, sp[30] = {0};
		// pid of process that execute `ls` and `ps`
		char fpe[30] = {0}, spe[30] = {0};
		int cnt = 0;
		while(fgets(buf, 1024, stdin)) {
			// fork() will call syscall `clone` and the return value is `pid`
			if (!regexec(&reg2, buf, nmatch, pmatch, 0)) {
				buf[pmatch[1].rm_eo] = 0;
				char *p = buf + pmatch[1].rm_so;
				if (strlen(fp)) {
					strcpy(sp, p);
					printf("\tget second child: %s\n", sp);
				} else {
					strcpy(fp, p);
					printf("\tget first child: %s\n", fp);
				}
			}
			// execve() will call syscall `execve` where the first argument is the program name that will be execute
			// and we check WHO call execve()
			if (!regexec(&reg, buf, nmatch, pmatch, 0)) {
				buf[pmatch[1].rm_eo] = 0;
				buf[pmatch[2].rm_eo] = 0;
				char *p1 = buf + pmatch[1].rm_so;
				char *p2 = buf + pmatch[2].rm_so;
				if (strstr(p2, "ps")) {
					strcpy(spe, p1);	
				}
				if(strstr(p2, "ls")) {
					strcpy(fpe, p1);
				}
			}
		}
		regfree(&reg);
		regfree(&reg2);
		int status;
		wait(&status);
		if (WEXITSTATUS(status) != 0) {
			printf("child process exit abnormally\n");			
		} else {
			if (!strcmp(sp, spe)) {
				// second process matched
				printf("\t[pid %s] ps: \033[0;32mok\033[0m\n", sp);
				cnt ++;
			}
			if (!strcmp(fp, fpe)) {
				// first process matched
				printf("\t[pid %s] ls: \033[0;32mok\033[0m\n", fp);
				cnt ++;
			}
			if (cnt == 2) {
				puts("\tlab1: \033[0;32mPASS\033[0m");
				return 0;
			}
		}
		
	}
	puts("\tlab1: \033[0;31mFAIL\033[0m");
	return 1;
}
