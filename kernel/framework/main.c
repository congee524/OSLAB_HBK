#include <kernel.h>
#include <klib.h>

void echo_task(void *arg) {
  char *name = (char *)arg;
  device_t *tty = dev_lookup(name);
  while (1) {
    char line[128], text[128];
    sprintf(text, "(%s) $ ", name);
    tty_write(tty, text);
    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.\n", line);
    tty_write(tty, text);
  }
}

int main() {
  _ioe_init();
  _cte_init(os->trap);

  // call sequential init code
  os->init();
  _mpe_init(os->run);  // all cores call os->run()
  return 1;
}