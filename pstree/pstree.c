#include <stdio.h>
#include <assert.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("Hello, World!\n");
    int i;
    for (i = 0; i < argc; i++) {
        assert(argv[i]); // always true
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]); // always true
    int ch;
    while((ch = getopt(argc, argv, "p:n:V:")) != -1) {
        printf("optind: %d\n", optind);
        switch(ch) {
            case 'p':
                printf("Have opotion: -p\n");
                break;
            case 'n':
                printf("Have option: -n\n");
                break;
            case 'V':
                printf("Have option: -V\b");
                break;
            case '?':
                printf("Unknown option: %c\n", (char)optopt);
                break;
        }
    }
    return 0;
}
