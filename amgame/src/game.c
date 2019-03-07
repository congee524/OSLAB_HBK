#include <game.h>

void init_screen();
void splash();
void read_key2();
void draw_rect2();
void init_background();
void chess();

#define LEN 15
#define HZ 100
#define CURSOR_PER_SECOND 4000
#define BLACK 0
#define WHITE 1
#define GREY 2
#define LGRAY 3
#define SGREY 4
#define LSGRAY 5
#define INIT_POS 7

int pre_x, pre_y, player;
int w, h;
uint32_t bg[LEN][LEN], pg[6];

int main() {
    // Operating system is a C program
    _ioe_init();
    init_screen();
    init_background();
    pre_x = pre_y = INIT_POS;
    player = BLACK;
    int frames = 0;
    while (1) {
        splash();
        frames ++;
        read_key2();
        /*
        if (frames % CURSOR_PER_SECOND == 0) {
            draw_rect2(pre_x * SIDE, pre_y * SIDE, SIDE, SIDE, 0xffffff);
        } else if (frames % CURSOR_PER_SECOND == CURSOR_PER_SECOND / 2) {
            if ((pre_x & 1) ^ (pre_y & 1)) {
                // draw_rect(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
                draw_rect2(pre_x * SIDE, pre_y * SIDE, SIDE, SIDE, 0x708090); // white
            } else {
                draw_rect2(pre_x * SIDE, pre_y * SIDE, SIDE, SIDE, 0x778899);
            }
        }
        */
    }
    return 0;
}

void move_cursor(int key) {
    if (key == 30 || key == 73) {
        if (pre_y > 0) {
            pre_y--;
        }
    } else if (key == 44 || key == 74) {
        if (pre_y < 14) {
            pre_y++;
        }
    } else if (key == 43 || key == 75) {
        if (pre_x > 0) {
            pre_x--;
        }
    } else if (key == 45 || key == 76) {
        if (pre_x < 14) {
            pre_x++;
        }
    }
    /*
    if (flag == 1) {
        splash();
    }
    */
}

void chess() {
    bg[pre_x][pre_y] = pg[player];
    player ^= 1;
}

void read_key2() {
    _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
#define KEYNAME(key) \
    [_KEY_##key] = #key,
    static const char *key_names[] = {
        _KEYS(KEYNAME)
    };
    _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
    if (event.keycode != _KEY_NONE && event.keydown) {
        puts("Key pressed: ");
        puts(key_names[event.keycode]);
        printf(" %d", event.keycode);
        puts("\n");
        move_cursor(event.keycode);
        if(event.keycode == 70) {
            chess();
        }
        printf("pre_x: %d, pre_y: %d\n", pre_x, pre_y);
    }
}


void init_screen() {
    _DEV_VIDEO_INFO_t info = {0};
    _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
    w = info.width;
    h = info.height;
}

void draw_rect2(int x, int y, int w, int h, uint32_t color) {
    uint32_t pixels[w * h]; // WARNING: allocated on stack
    _DEV_VIDEO_FBCTL_t event = {
        .x = x, .y = y, .w = w, .h = h, .sync = 1,
        .pixels = pixels,
    };
    for (int i = 0; i < w * h; i++) {
        pixels[i] = color;
    }
    _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTL, &event, sizeof(event));
}

void splash() {
    for (int x = 0; x < LEN; x++) {
        for (int y = 0; y < LEN; y++) {
            if (x == pre_x && y == pre_y) {
                if (bg[x][y] == pg[WHITE] || bg[x][y] == pg[BLACK]) {
                    draw_rect2(x * SIDE, y * SIDE, SIDE, SIDE, pg[LGRAY]);
                } else {
                    draw_rect2(x * SIDE, y * SIDE, SIDE, SIDE, pg[GREY]);
                }
            } else {
                draw_rect2(x * SIDE, y * SIDE, SIDE, SIDE, bg[x][y]);
            }
        }
    }

    /*
    for (int x = 0; x * SIDE <= w; x ++) {
        for (int y = 0; y * SIDE <= h; y++) {
            if ((x & 1) ^ (y & 1)) {
                // draw_rect(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
                draw_rect2(x * SIDE, y * SIDE, SIDE, SIDE, 0x708090); // white
            } else {
                draw_rect2(x * SIDE, y * SIDE, SIDE, SIDE, 0x778899);
            }
        }
    }
    */
}

void init_background() {
    pg[WHITE] = 0xffffff;
    pg[BLACK] = 0x000000;
    pg[SGREY] = 0x708090;
    pg[LSGRAY] = 0x778899;
    pg[GREY] = 0xbebebe;
    pg[LGRAY] = 0xd3d3d3;

    for (int x = 0; x < LEN; x++) {
        for (int y = 0; y < LEN; y++) {
            if ((x & 1) ^ (y & 1)) {
                bg[x][y] = pg[SGREY];
            } else {
                bg[x][y] = pg[LSGRAY];
            }
            draw_rect2(x * SIDE, y * SIDE, SIDE, SIDE, bg[x][y]);
        }
    }
}
