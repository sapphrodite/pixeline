#ifndef API_H
#define API_H

#include <core/image_types.h>
#include <include/integral_types.h>

class handle;
handle* handle_new();
void handle_free(handle*);

enum class tool {
	pencil, fill, null
};

void op_cancel(handle*);
void op_finalize(handle*);
void cursor_press(handle*, vec2i p);
void cursor_drag(handle*, vec2i p);
void cursor_release(handle*);

void tool_select(handle*, tool t);
void pencil(handle*, u8 pal_idx, vec2i p1, vec2i p2);
void fill(handle*, u8 pal_idx, vec2i p);

void image_new(handle*, vec2u size);
vec2u image_size(handle* hnd);
const f32* image_data(handle*);

void pal_select(handle*, u8 pal_idx);
void pal_set(handle*, u8 pal_idx, rgba color);
rgba pal_get(handle*, u8 pal_idx);

void undo(handle* hnd);
void redo(handle* hnd);

#endif //API_H
