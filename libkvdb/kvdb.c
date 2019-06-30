#include "kvdb.h"

int errlog(char *buf) { return printf("\33[1;33m %s \33[0m\n", buf); }

// 文件里是key 和 value一行接着一行，读到最后一个
int kvdb_open(kvdb_t *db, const char *filename) {
  if (pthread_mutex_lock(&db->mutex)) {
    errlog("close mutex lock error");
    return -1;
  }
  strcpy(db->name, filename);
  if ((db->fp = fopen(filename, "a+")) == NULL) {
    errlog("file open fail!");
    return -1;
  };
  if (pthread_mutex_unlock(&db->mutex)) {
    errlog("close mutex unlock error");
    return -1;
  }
  return 0;
}

int kvdb_close(kvdb_t *db) {
  if (pthread_mutex_lock(&db->mutex)) {
    errlog("close mutex lock error");
    return -1;
  }
  fclose(db->fp);
  if (pthread_mutex_unlock(&db->mutex)) {
    errlog("close mutex unlock error");
    return -1;
  }
  return 0;
}

int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  if (pthread_mutex_lock(&db->mutex)) {
    errlog("close mutex lock error");
    return -1;
  }

  if (flock(db->fp->_fileno, LOCK_EX) == 0) {
    printf("the file was locked\n");
  }

  fseek(db->fp, 0, SEEK_END);
  fprintf(db->fp, "key:%s\n", key);
  fprintf(db->fp, "val:%s$$\n", value);

  if (flock(db->fp->_fileno, LOCK_UN) == 0) {
    printf("the file was unlocked$$\n");
  }

  if (pthread_mutex_unlock(&db->mutex)) {
    errlog("close mutex unlock error");
    return -1;
  }
  return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
  if (pthread_mutex_lock(&db->mutex)) {
    errlog("close mutex lock error");
    return NULL;
  }
  if (flock(db->fp->_fileno, LOCK_EX) == 0) {
    printf("the file was locked\n");
  }
  fseek(db->fp, 0, SEEK_SET);
  char *key_buf = malloc(2048);
  char *val_buf = malloc(2048);
  char *res = malloc(2048);
  int flag = 0;
  while (1) {
    if (!flag) {
      if (!fgets(key_buf, 2048, db->fp)) break;
      if (strncmp(key_buf, "key:", 4) != 0) continue;
    } else {
      flag = 0;
    }

    if (!fgets(val_buf, 2048, db->fp)) break;
    if (strncmp(val_buf, "val:", 4) != 0) {
      if (strncmp(val_buf, "key:", 4) == 0) {
        strcpy(key_buf, val_buf);
        flag = 1;
      }
      continue;
    }
    if (strncmp(key_buf + 4, key, strlen(key)) != 0) continue;
    if (strncmp(val_buf + (strlen(val_buf) - 3), "$$", 2) != 0) continue;
    val_buf[strlen(val_buf) - 3] = '\0';
    strcpy(res, val_buf + 4);
  }
  if (flock(db->fp->_fileno, LOCK_UN) == 0) {
    printf("the file was unlocked\n");
  }
  free(key_buf);
  free(val_buf);
  if (pthread_mutex_unlock(&db->mutex)) {
    errlog("close mutex unlock error");
    return NULL;
  }
  return res;
}