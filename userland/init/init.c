#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// this is a hack
// allocate buffer on stack until we have a malloc in stdlib.h
#define MAX_FRAME_BYTES 65536
uint8_t frame_buffer[MAX_FRAME_BYTES];

uint64_t uptime() {
	uint64_t ret;
	asm volatile("int $0x80" : "=a"(ret) : "a"(3));
	return ret;
}

void draw_frame(void* frame, int w, int h) {
	asm volatile("int $0x80" :: "a"(4), "b"(frame), "c"(w), "d"(h));
}

void yield() {
	asm volatile("int $0x80" :: "a"(5));
}

void _start() {
    uint8_t* file = (uint8_t*)0x80000000;
    int found = 0;
    
    for (int i = 0; i < 4096; i++) {
        if (file[i] == 'B' && file[i+1] == 'A' && file[i+2] == 'D' && file[i+3] == '\0') {
            file = &file[i];
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Error: Invalid or missing video.bad!\n");
        abort();
    }
    
    uint16_t w = file[4] | (file[5] << 8);
    uint16_t h = file[6] | (file[7] << 8);
    uint32_t total_frames = file[8] | (file[9] << 8) | (file[10] << 16) | (file[11] << 24);
    
    int pixels_per_frame = w * h;
    int bytes_per_frame = ((w * h) + 7) / 8;

    if (bytes_per_frame > MAX_FRAME_BYTES) {
        printf("Error: Video resolution too large for frame_buffer!\n");
        abort();
    }

    printf("Playing Video!!\n");
    printf("Resolution: %dx%d\n", w, h);
    printf("Frames:     %d\n", total_frames);

    while (1) {
        uint64_t start_time = uptime();
        uint32_t current_frame = 0;
        
        uint8_t* rle_ptr = file + 12;
        uint8_t current_color = 0;
        uint8_t current_run = 0;
        
        while (current_frame < total_frames) {
            
            for (int i = 0; i < bytes_per_frame; i++) {
                frame_buffer[i] = 0;
            }
            
            int pixels_drawn = 0;
            while (pixels_drawn < pixels_per_frame) {
                if (current_run == 0) {
                    uint8_t b = *rle_ptr++;
                    current_color = b >> 7; // top bit
                    current_run = b & 0x7F; // bottom 7 bits
                }
                
                int to_draw = current_run;
                if (pixels_drawn + to_draw > pixels_per_frame) {
                    to_draw = pixels_per_frame - pixels_drawn;
                }
                
                if (current_color == 1) {
                    for (int i = 0; i < to_draw; i++) {
                        int px = pixels_drawn + i;
                        frame_buffer[px / 8] |= (1 << (px % 8));
                    }
                }
                
                current_run -= to_draw;
                pixels_drawn += to_draw;
            }
            
            draw_frame(frame_buffer, w, h);
            
            uint64_t expected_time = start_time + (current_frame * 1000 / 30);
            while (uptime() < expected_time) {
                yield();
            }
            
            current_frame++;
        }
    }
}
