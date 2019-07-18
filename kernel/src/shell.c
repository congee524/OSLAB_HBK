#include <cmd.h>
#include <common.h>
#include <dir.h>
#include <klib.h>
#include <shell.h>
extern inode_t *itable[];

void shell_thread(void *tty_id) {
  char buf[128];
  sprintf(buf, "/dev/tty%s", tty_id);
  int stdin = vfs->open(buf, O_RDONLY);
  int stdout = vfs->open(buf, O_WRONLY);
  while (1) {
    char line[128], text[128];
    sprintf(text, "(%s) [%s] $ ", buf, cur_pwd);
    vfs->write(stdout, text, strlen(text));
    int nread = vfs->read(stdin, line, sizeof(line));
    line[nread - 1] = '\0';
    if (cmd_parse(line, text)) {
      vfs->write(stdout, text, strlen(text));
    }
  }
}