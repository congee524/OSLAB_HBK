#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char command[1024];
char ex_buffer[1024];
char func_buffer[1024];
int cnt_ex;
void *handle;
char *error;
int (*ex_func)();

int main(int argc, char *argv[]) {
  cnt_ex = 0;

  // create a tmp file to be complied
  FILE *fp = fopen("/tmp/crepl_link.c", "w");
  // "w" Opens an empty file for writing
  // If the given file exists, its contents are destroyed.
  fclose(fp);

  while (fgets(command, 1024, stdin)) {
    if (strncmp(command, "int", 3) == 0) {
      fp = fopen("/tmp/crepl_test.c", "w");
      fprintf(fp, "%s", command);
      fclose(fp);

      int status;
      pid_t pid = fork();
      if (pid == 0) {
        char *cflags[] = {
          "gcc",
          "-shared",
          "-fPIC",
#if defined(__i386__)
          "-m32",
#elif defined(__x86_64__)
          "-m64",
#endif
          "/tmp/crepl_test.c",
          "-o",
          "/tmp/crepl_test.so",
          "-ldl"
        };
        execvp("gcc", cflags);
        _exit(1);
      } else {
        wait(&status);

        fp = fopen("/tmp/crepl_link.c", "a+");
        fprintf(fp, "%s", command);
        fclose(fp);
      }
    } else {
      int len = strlen(command) - 1;
      if (command[len] == '\n') {
        command[len] = '\0';
      }

      memset(func_buffer, '\0', sizeof(func_buffer));
      memset(ex_buffer, '\0', sizeof(ex_buffer));
      sprintf(func_buffer, "__expression%d", ++cnt_ex);
      sprintf(ex_buffer, "int %s(){return %s;}", func_buffer, command);

      int status;
      pid_t pid = fork();
      if (pid == 0) {
        char *cflags[] = {"cp", "/tmp/crepl_link.c", "/tmp/crepl_ex.c"};
        execvp("cp", cflags);
        _exit(1);
      } else {
        wait(&status);

        // test
        // system("cp /tmp/crepl_link.c /tmp/crepl_ex.c");
        // test

        fp = fopen("/tmp/crepl_ex.c", "a+");
        fprintf(fp, "%s", ex_buffer);
        fclose(fp);

        int status2;
        pid_t pid2 = fork();
        if (pid2 == 0) {
          char *cflags[] = {
            "gcc",
            "-shared",
            "-fPIC",
#if defined(__i386__)
            "-m32",
#elif defined(__x86_64__)
            "-m64",
#endif
            "/tmp/crepl_ex.c",
            "-o",
            "/tmp/crepl_ex.so"
          };
          execvp("gcc", cflags);
          _exit(1);
        } else {
          wait(&status2);

          // test
          //#if defined(__x86_64__)
          //      system("gcc -shared -fPIC -m64 /tmp/crepl_ex.c -o
          //      /tmp/crepl_ex.so -ldl");
          //#else
          //      system("gcc -shared -fPIC -m32 /tmp/crepl_ex.c -o
          //      /tmp/crepl_ex.so -ldl");
          //#endif
          // test

          handle = dlopen("/tmp/crepl_ex.so", RTLD_LAZY);
          if (!handle) {
            fprintf(stderr, "%s\n", dlerror());
            return 1;
          }
          dlerror();

          ex_func = dlsym(handle, func_buffer);
          if ((error = dlerror()) != NULL) {
            fprintf(stderr, "%s\n", error);
            return 1;
          }
          printf("%s = %d\n", command, (*ex_func)());
          dlclose(handle);
        }
      }
    }
  }
  return 0;
}
