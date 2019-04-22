#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  /*
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  */
  if (argc <= 1) {
    printf("please input the program you want to run!\n");
    return 0;
  }

  int status;

  int fildes[2];
  if (pipe(fildes) != 0) {
    printf("create pipe failed!\n");
    return -1;
  }

  char* st_argv[argc + 3];
  st_argv[0] = "strace";
  st_argv[1] = "-T";
  for (int i = 1; i < argc; i++) {
    st_argv[i + 1] = argv[i];
  }
  st_argv[argc + 2] = NULL;

  pid_t pid = fork();
  if (pid == 0) {
    dup2(fildes[1], STDOUT_FILENO);
    close(fildes[0]);
    execvp("strace", st_argv);
    exit(0);
  } else {
    close(fildes[1]);
    /*
        char* pat_func = "^[a-zA-Z0-9_]{2,}";
        char* pat_time = "<[0-9.]{2,}>";
        char errbuf[1024];
        char match[100];
        regex_t reg_func, reg_time;
        int err = 0, nm = 1;
        regmatch_t pmatch_func[nm];
        regmatch_t pmatch_time[nm];

        if (regcomp(&reg_func, pat_func, REG_EXTENDED) < 0) {
          regerror(err, &reg_func, errbuf, sizeof(errbuf));
          printf("err:%s\n", errbuf);
        }

        if (regcomp(&reg_time, pat_time, REG_EXTENDED) < 0) {
          regerror(err, &reg_time, errbuf, sizeof(errbuf));
          printf("err:%s\n", errbuf);
        }
    */
    /*
        char buffer[1024] = {0};
        while (fgets(buffer, 1023, stdout)) {
          // printf("%s", buffer);

          err = regexec(&reg_func, buffer, nm, pmatch_func, 0);
          if (err == REG_NOMATCH) {
            printf("no match\n");
            exit(-1);
          } else if (err) {
            regerror(err, &reg_func, errbuf, sizeof(errbuf));
            printf("err:%s\n", errbuf);
            exit(-1);
          }

          err = regexec(&reg_time, buffer, nm, pmatch_time, 0);
          if (err == REG_NOMATCH) {
            printf("no match\n");
            exit(-1);
          } else if (err) {
            regerror(err, &reg_time, errbuf, sizeof(errbuf));
            printf("err:%s\n", errbuf);
            exit(-1);
          }

          for (int i = 0; i < nm && pmatch_func[i].rm_so != -1; i++) {
            int len = pmatch_func[i].rm_eo - pmatch_func[i].rm_so;
            // printf("\nlen: %d\n", len);
            if (len) {
              memset(match, '\0', sizeof(match));
              memcpy(match, buffer + pmatch_func[i].rm_so, len);
              // printf("%s\n", match);
            }
          }

          for (int i = 0; i < nm && pmatch_time[i].rm_so != -1; i++) {
            int len = pmatch_time[i].rm_eo - pmatch_time[i].rm_so;
            // printf("\nlen: %d\n", len);
            if (len) {
              memset(match, '\0', sizeof(match));
              memcpy(match, buffer + pmatch_time[i].rm_so, len);
              // printf("%s\n", match);
            }
          }
        }
        */
    char buffer[1024] = {0};
    int len;
    while ((len = read(STDIN_FILENO, buffer, 1023)) > 0)
      ;
    waitpid((pid_t)pid, &status, 0);
  }

  return 0;
}
