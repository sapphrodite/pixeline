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

void op_cancel(handle*);
void op_finalize(handle*);
bool cursorpress(handle*, vec2i p, unsigned flags);
bool cursordrag(handle*, vec2i p1, vec2i p2, unsigned flags);
bool cursorrelease(handle*, unsigned flags);

void set_tool(handle*, tool t);
void pencil(handle*, palette_idx c, vec2i p1, vec2i p2);
void fill(handle*, palette_idx c, vec2i p, bool global);

void new_image(handle*, vec2u size);
void load_image(handle*, const char* filename);
vec2u get_imagesize(handle*);
const f32* imagedata(handle*);

#define PALETTE_SIZE 256
void set_active_color(handle*, palette_idx c);
void set_pal_color(handle*, palette_idx c, rgba r);
rgba get_pal_color(handle*, palette_idx c);

void undo(handle* hnd); 
void redo(handle* hnd);

void layer_add(handle* hnd);
void layer_remove(handle* hnd, int layer_id);
void layer_select(handle* hnd, int layer_id);
void layer_reorder(handle* hnd, int old_pos, int new_pos);

#endif //API_H
