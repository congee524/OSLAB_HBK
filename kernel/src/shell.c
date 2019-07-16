#include <common.h>
#include <dir.h>
#include <klib.h>
#include <shell.h>

void shell_thread(void *tty_id) {
  kmt->spin_lock(&print_lk);
  log("cur_pwd[%d]: %s\n", _cpu(), cur_pwd);
  kmt->spin_unlock(&print_lk);
  char buf[128];
  sprintf(buf, "/dev/tty%s", tty_id);
  int stdin = vfs->open(buf, O_RDONLY);
  int stdout = vfs->open(buf, O_WRONLY);
  while (1) {
    char line[128], text[128];
    // printf("sizeof text before: %d", strlen(text));
    sprintf(text, "(%s) [%s] $ ", buf, cur_pwd);

    kmt->spin_lock(&print_lk);
    printf("cur pwd: %s\n", cur_pwd);
    kmt->spin_unlock(&print_lk);

    vfs->write(stdout, text, strlen(text));
    // printf("sh2\n");
    int nread = vfs->read(stdin, line, strlen(text));
    // printf("sh3\n");
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.\n", line);
    vfs->write(stdout, text, strlen(text));
    // printf("sh4\n");
  }
}

/*
void shell_thread(int tty_id) {
  char buf[128];
  sprintf(buf, "/dev/tty%d", tty_id);
  int stdin = vfs->open(buf, O_RDONLY);
  int stdout = vfs->open(buf, O_WRONLY);
  while (1) {
    if (got_a_line()) {
    } else {
      ssize_t nread = fs->read(stdin, buf, sizeof(buf));
    }
    // supported commands:
    //   ls
    //   cd /proc
    //   cat filename
    //   mkdir /bin
    //   rm /bin/abc
    //   ...
  }
}

void echo_task(void *name) {
  device_t *tty = dev_lookup(name);
  while (1) {
    char line[128], text[128];
    sprintf(text, "(%s) $ ", name); tty_write(tty, text);
    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.\n", line); tty_write(tty, text);
  }
}
*/