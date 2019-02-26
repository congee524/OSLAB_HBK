#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>

static const char *shortopts = "pnV";

struct option longopts[] = {
    {"show-pids", no_argument, NULL, 'p'},
    {"numeric-sort", no_argument, NULL, 'n'},
    {"version", no_argument, NULL, 'V'},
    {0, 0, 0, 0},
};

int main(int argc, char *argv[]) {
    printf("Hello, World!\n");
    int i;
    for (i = 0; i < argc; i++) {
        assert(argv[i]); // always true
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]); // always true
    int ch;
    while((ch = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
        printf("optind: %d\n", optind);
        switch(ch) {
            case 'p':
                printf("Have option: -p\n");
                break;
            case 'n':
                printf("Have option: -n\n");
                break;
            case 'V':
                printf("Have option: -V\n");
                break;
            case '?':
                printf("Unknown option: %c\n", (char)optopt);
                break;
        }
    }
    return 0;
}
