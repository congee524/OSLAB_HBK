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
extern char **environ;

void my_system(char *const *__argv) {
  int status;
  pid_t pid = fork();
  if (pid == 0) {
    execve(__argv[0], __argv, environ);
    _exit(1);
  }
  waitpid(pid, &status, 0);
}

int main(int argc, char *argv[]) {
  pid_t pid = fork();
  char *rm1[] = {"/bin/rm", "/tmp/crepl_ex.so", NULL};
  my_system(rm1);
  char *rm2[] = {"/bin/rm", "/tmp/crepl_ex.c", NULL};
  my_system(rm2);
  char *rm3[] = {"/bin/rm", "/tmp/crepl_link.c", NULL};
  my_system(rm3);
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

#if defined(__i386__)
      char *gcc_test[] = {"gcc",
                          "-shared",
                          "-fPIC",
                          "-m32",
                          "/tmp/crepl_test.c",
                          "-o",
                          "/tmp/crepl_test.so",
                          "-ldl",
                          NULL};
#elif defined(__x86_64__)
      char *gcc_test[] = {"gcc",
                          "-shared",
                          "-fPIC",
                          "-m64",
                          "/tmp/crepl_test.c",
                          "-o",
                          "/tmp/crepl_test.so",
                          "-ldl",
                          NULL};
#endif

      my_system(gcc_test);

      fp = fopen("/tmp/crepl_link.c", "a+");
      fprintf(fp, "%s", command);
      fclose(fp);
    } else {
      int len = strlen(command) - 1;
      if (command[len] == '\n') {
        command[len] = '\0';
      }

      memset(func_buffer, '\0', sizeof(func_buffer));
      memset(ex_buffer, '\0', sizeof(ex_buffer));
      sprintf(func_buffer, "__expression%d", ++cnt_ex);
      sprintf(ex_buffer, "int %s(){return %s;}", func_buffer, command);

      char *cp_link[] = {"cp", "/tmp/crepl_link.c", "/tmp/crepl_ex.c"};
      my_system(cp_link);
      // system("ls /tmp/");
      // test
      // system("cp /tmp/crepl_link.c /tmp/crepl_ex.c");
      // test

      fp = fopen("/tmp/crepl_ex.c", "a+");
      fprintf(fp, "%s", ex_buffer);
      fclose(fp);

#if defined(__i386__)
      char *gcc_ex[] = {
          "gcc", "-shared",          "-fPIC", "-m32", "/tmp/crepl_ex.c",
          "-o",  "/tmp/crepl_ex.so", "-ldl",  NULL};
#elif defined(__x86_64__)
      char *gcc_ex[] = {
          "gcc", "-shared",          "-fPIC", "-m64", "/tmp/crepl_ex.c",
          "-o",  "/tmp/crepl_ex.so", "-ldl",  NULL};
#endif
      my_system(gcc_ex);

      // system("ls /tmp");
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
  return 0;
}
