#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
// reference: blog.csdn.net/zhenjiangge/article/details/5121294
static const char *shortopts = "pnV";

struct option longopts[] = {
    {"show-pids", no_argument, NULL, 'p'},
    {"numeric-sort", no_argument, NULL, 'n'},
    {"version", no_argument, NULL, 'V'},
    {0, 0, 0, 0},
};

int filter(const struct dirent *dir) {
    // only choose the digits
    int i;
    int n = strlen(dir->d_name);
    for (i = 0; i < n; i++) {
        if (!isdigit(dir->d_name[i]))
            return 0;
        else
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    struct dirent **namelist;
    // namelist struction: d_ino, d_off, d_reclen, d_type, d_name

    printf("Hello, World!\n");
    for (int i = 0; i < argc; i++) {
        assert(argv[i]); // always true
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]); // always true

    int ch, p_flag = 0, n_flag = 0, v_flag = 0;
    while((ch = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
        printf("optind: %d\n", optind);
        switch(ch) {
            case 'p':
                // printf("Have option: -p\n");
                p_flag = 1;
                break;
            case 'n':
                // printf("Have option: -n\n");
                n_flag = 1;
                break;
            case 'V':
                // printf("Have option: -V\n");
                v_flag = 1;
                break;
            case '?':
                printf("Unknown option: %c\n", (char)optopt);
                break;
        }
    }
    printf("%d%d%d\n", p_flag, n_flag, v_flag);

    if (v_flag == 1) {
        printf("pstree(minilab) 0.1\n");
        printf("OSminilab, the supported arguments are:\n");
        printf("-p(show-pids), -n(numeric-sort), -V(version).");
        return 0;
    }

    int total;
    total = scandir("/proc", &namelist, filter, alphasort);
    if (total < 0) {
        printf("\033[31mscandir error!\033[0m");
        return -1;
    } else {
        printf("\033[4mtotal processes: %d.\033[0m\n\n", total);
    }

    //printf("%s", namelist[6]->d_name);

    FILE *fp;
    char pid_path[64];
    for (int i = 0; i < total; i++) {
        strcpy(pid_path, "/proc/");
        strcat(pid_path, namelist[i]->d_name);
        strcat(pid_path, "/status");
        // printf("%s\n", pid_path);

        fp = fopen(pid_path, "r");
        while(!feof(fp)) {
            fgets(str, 1024, fp);
            printf("%s\n", str);
        }
        fclose(fp);
    }
    return 0;
}
