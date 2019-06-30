#include "kvdb.h"

int errlog(char *buf) { return printf("\33[1;33m %s \33[0m\n", buf); }

// 文件里是key 和 value一行接着一行，读到最后一个
int kvdb_open(kvdb_t *db, const char *filename) {
  strcpy(db->name, filename);
  if ((db->fp = fopen(filename, "a+")) == NULL) {
    errlog("file open fail!");
    return -1;
  };

  return 0;
}

int kvdb_close(kvdb_t *db) {
  fclose(db->fp);
  return 0;
}

int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  if (flock(db->fp->_fileno, LOCK_EX) == 0) {
    printf("the file was locked\n");
  }
  fseek(db->fp, 0, SEEK_END);
  fprintf(db->fp, "%s\n", key);
  fprintf(db->fp, "%s\n", value);
  if (flock(db->fp->_fileno, LOCK_UN) == 0) {
    printf("the file was unlocked\n");
  }
  return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
  if (flock(db->fp->_fileno, LOCK_EX) == 0) {
    printf("the file was locked\n");
  }
  fseek(db->fp, 0, SEEK_SET);
  char *key_buf = malloc(2048);
  char *val_buf = malloc(2048);
  while (1) {
    if (!fgets(key_buf, 2048, db->fp)) break;
    if (!fgets(val_buf, 2048, db->fp)) break;
  }
  if (flock(db->fp->_fileno, LOCK_UN) == 0) {
    printf("the file was unlocked\n");
  }
  free(key_buf);
  val_buf[strlen(val_buf) - 1] = '\0';
  return val_buf;
}