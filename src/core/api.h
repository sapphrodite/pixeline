#ifndef API_H
#define API_H

#include <core/image_types.h>
#include <include/integral_types.h>

class handle;
handle* handle_new();
void handle_free(handle*);

void cursor_press(handle*, int x, int y);
void cursor_drag(handle*, int x, int y);
void cursor_release(handle*);

void pencil(handle*, u8 pal_idx, int x1, int y1, int x2, int y2);

void image_new(handle*, u16 w, u16 h);
void image_size(handle* hnd, uint16_t* w, uint16_t* h);
const f32* image_data(handle*);

void pal_select(handle*, u8 pal_idx);
void pal_set(handle*, u8 pal_idx, rgba color);
rgba pal_get(handle*, u8 pal_idx);

#endif //API_H
