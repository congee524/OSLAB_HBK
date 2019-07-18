#include <cmd.h>
#include <common.h>
#include <dir.h>
#include <klib.h>
#include <shell.h>

static char *cmd_list[] = {"ls",  "cd",    "pwd",   "mkdir",
                           "cat", "touch", "rmdir", "rm"};
#define CMDNUM \
    ( sizeof(cmd_list) / sizeof(cmd_list[0]) )
enum CMD_LIST { LS = 0, CD, PWD, MKDIR, CAT, TOUCH, RMDIR, RM };

extern inode_t *itable[];

int cmd_parse(char *input, char output[128]) {
  memset(output, '\0', sizeof(output));
  char *tmp_cmd = pmm->alloc(128);
  strcpy(tmp_cmd, input);
  char *pch = strtok(tmp_cmd, " ");
  int cmd_type = 0;
  char *temp = NULL;
  for (cmd_type = 0; cmd_type < CMDNUM; cmd_type++) {
    temp = cmd_list[cmd_type];
    if (strlen(temp) == strlen(pch) && strcmp(temp, pch) == 0) break;
  }

  int ret = 0;
  switch (cmd_type) {
    case LS: {
      pch = strtok(NULL, " ");
      if (!pch) {
        pch = pmm->alloc(MAXPATHLEN);
        strcpy(pch, cur_pwd);
      }
      int tmp_ind = path_parse(pch);
      dir_t *tmp_dir = itable[tmp_ind]->ptr;
      for (int i = 0; i < MAXDIRITEM; i++) {
        if (tmp_dir->names[i]) {
          strcat(output, tmp_dir->names[i]);
          strcat(output, " ");
        }
      }
      ret = 1;
      break;
    }
    case CD: {
      pch = strtok(NULL, " ");
      if (!pch) {
        ret = 0;
        break;
      }
      // printf("cd: %s\n", pch);
      output = realpath(pch, output);
      strcpy(cur_pwd, output);
      ret = 0;
      break;
    }
    case PWD: {
      strcpy(output, cur_pwd);
      ret = 1;
      break;
    }
    case MKDIR: {
      pch = strtok(NULL, " ");
      if (!pch) {
        ret = 0;
        break;
      }
      vfs->mkdir(pch);
      ret = 0;
      break;
    }
    case CAT: {
      pch = strtok(NULL, " ");
      if (!pch) {
        ret = 0;
        break;
      }
      int cat_finode_ind = path_parse(pch);
      if (itable[cat_finode_ind]->type == VFILE_DIR) {
        realpath(pch, output);
        strcat(output, " is a directory.");
        ret = 1;
        break;
      }
      char tmp_name[MAXNAMELEN];
      find_parent_dir(pch, tmp_name);
      if (strncmp(tmp_name, "tty", 3) == 0) {
        strcpy(output, "tty doesn't support cat!");
        ret = 1;
        break;
      } else if (strncmp(tmp_name, "input", 5) == 0) {
        strcpy(output, "input doesn't support cat!");
        ret = 1;
        break;
      }
      int cat_fd = vfs->open(pch, O_RDONLY);
      vfs->read(cat_fd, output, 126);
      vfs->close(cat_fd);
      ret = 1;
      break;
    }
    case TOUCH: {
      pch = strtok(NULL, " ");
      if (!pch) {
        ret = 0;
        break;
      }
      vfs->touch(pch);
      ret = 0;
      break;
    }
    case RMDIR: {
      pch = strtok(NULL, " ");
      if (!pch) {
        ret = 0;
        break;
      }
      vfs->rmdir(pch);
      ret = 0;
      break;
    }
    case RM: {
      pch = strtok(NULL, " ");
      if (!pch) {
        ret = 0;
        break;
      }
      vfs->rm(pch);
      ret = 0;
      break;
    }
    default: {
      strcpy(output, "Unknown cmd!");
      ret = 1;
      break;
    }
  }
  strcat(output, "\n");
  pmm->free(tmp_cmd);
  return ret;
}
