#ifndef API_H
#define API_H

#include <stdint.h>
#include <core/image_types.h>


using palette_idx = uint8_t;

class handle;
handle* handle_alloc();
void free(handle*);

enum class tool {
	pencil, fill, null
};

void set_tool(handle*, tool t);
bool cursorpress(handle*, int x, int y, unsigned flags);
bool cursordrag(handle*, int x1, int y1, int x2, int y2, unsigned flags);
bool cursorrelease(handle*, unsigned flags);
void pencil(handle*, palette_idx c, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void fill(handle*, palette_idx c, int x, int y, bool global);

void new_image(handle*, uint16_t w, uint16_t h);
void load_image(handle*, const char* filename);
void get_imagesize(handle*, uint16_t* w, uint16_t* h);
const f32* imagedata(handle*);

#define PALETTE_SIZE 256
void set_active_color(handle*, palette_idx c);
void set_pal_color(handle*, palette_idx c, rgba r);
rgba get_pal_color(handle*, palette_idx c);

void undo(handle* hnd); 
void redo(handle* hnd);

#endif //API_H
