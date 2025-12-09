#ifndef GLYPH_PAINTER_H
#define GLYPH_PAINTER_H

/* Draw a single glyph at the given cursor position.
   On success returns 0, on error returns non–zero. */
int draw_glyph(int ascii_code, float *cursor_x, float *cursor_y, float scale);

#endif
