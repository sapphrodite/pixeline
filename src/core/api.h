#ifndef API_H
#define API_H

#include <stdint.h>
#include <common/image_types.h>


using palette_idx = uint8_t;

class handle;
handle* handle_alloc();
void free(handle*);


void tool_release(handle*);
void tool_cancel(handle*);
void pencil(handle*, palette_idx c, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

void new_image(handle*, uint16_t w, uint16_t h);
void get_imagesize(handle*, uint16_t* w, uint16_t* h);
const rgba* imagedata(handle*);

#define PALETTE_SIZE 256
void set_pal_color(handle*, palette_idx c, rgba r);
rgba get_pal_color(handle*, palette_idx c);

void undo(handle* hnd); 
void redo(handle* hnd);

#endif //API_H
