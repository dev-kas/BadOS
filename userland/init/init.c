#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint64_t uptime() {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(3));
    return ret;
}

void draw_frame(void* frame, int w, int h) {
    asm volatile("int $0x80" :: "a"(4), "b"(frame), "c"(w), "d"(h));
}

void _start() {
    uint8_t* file = (uint8_t*)0x80000000;
    
    if (file[0] != 'B' || file[1] != 'A' || file[2] != 'D') {
        printf("Error: Invalid or missing video.bad!\n");
        abort();
    }
    
    uint16_t w = file[4] | (file[5] << 8);
    uint16_t h = file[6] | (file[7] << 8);
    
    int bytes_per_frame = ((w * h) + 7) / 8;
    uint8_t* frames = file + 8;
    
    printf("Playing Video!! %dx%d\n", w, h);
    
    uint64_t start_time = uptime();
    int fps = 30;
    int current_frame = 0;
    
    while (current_frame < 6570) {
        draw_frame(frames + (current_frame * bytes_per_frame), w, h);
        
        uint64_t expected_time = start_time + (current_frame * 1000 / fps);
        
        while (uptime() < expected_time) { }
        
        current_frame++;
    }
    
    abort();
}
