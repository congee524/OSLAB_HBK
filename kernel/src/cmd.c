#include <cmd.h>
#include <common.h>
#include <dir.h>
#include <klib.h>
#include <shell.h>

// cmd_num 需要手动改！！！！！！
#define CMDNUM 2
static char *cmd_list[] = {"ls", "cd"};
enum CMD_LIST { LS = 0, CD };

int cmd_parse(char *input, char *output) {
  char *tmp_cmd = pmm->alloc(128);
  strcpy(tmp_cmd, input);
  char *pch = strtok(tmp_cmd, " ");
  int cmd_type = -1;
  char *temp = NULL;
  for (cmd_type = 0; cmd_type < CMDNUM; cmd_type) {
    temp = cmd_list[cmd_type];
    if (strlen(temp) == strlen(pch) && strcmp(temp, pch) == 0) break;
  }

  int ret = 0;
  switch (cmd_type) {
    case LS: {
      pch = strtok(NULL, " ");
      int tmp_ind = path_parse(pch);
      dir_t *tmp_dir = itable[tmp_ind]->ptr;
      for (int i = 0; i < MAXDIRITEM; i++) {
        if (tmp_dir->names[i]) {
          strcat(text, tmp_dir->names[i]);
          strcat(text, " ");
        }
      }
      strcat(text, "\n");
      ret = 1;
      break;
    }
    case CD: {
      pch = strtok(NULL, " ");
      output = realpath(pch, output);
      ret = 0;
      break;
    }
    default: {
      strcpy(output, "Unknown cmd!");
      break;
    }
  }
  pmm->free(tmp_cmd);
  return ret;
}