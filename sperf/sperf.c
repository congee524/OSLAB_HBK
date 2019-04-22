#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char buffer[1024];

int main(int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  if (argc <= 1) {
    printf("please input parameters!\n");
    return 0;
  }

  int status;

  int pfd[2];
  if (pipe(pfd) < 0) {
    return -1;
  }

  char* st_argv[argc + 2];
  st_argv[0] = "strace";
  st_argv[1] = "-T";
  for (int i = 1; i < argc; i++) {
    st_argv[i + 1] = argv[i];
  }

  pid_t pid = fork();
  if (pid == 0) {
    printf("!!\n");
    dup2(pfd[1], STDOUT_FILENO);
    close(pfd[0]);
    execvp("strace", st_argv);
    exit(0);
  } else {
    close(pfd[1]);
    int len;
    while ((len = read(pfd[0], buffer, 1023)) > 0) {
      buffer[len] = '\0';
      printf("%s\n", buffer);
    }

    waitpid((pid_t)pid, &status, 0);
  }

  return 0;
}
