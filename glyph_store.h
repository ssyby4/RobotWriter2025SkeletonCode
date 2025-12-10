#ifndef GLYPH_STORE_H
#define GLYPH_STORE_H

/* One stroke from the font:
 * x, y are in font units, pen is 0 (up) or 1 (down)
 */
typedef struct
{
    int x;
    int y;
    int pen;
} StrokePoint;

/* Load all glyph strokes from the font file into memory.
 * Returns 0 on success, non-zero on error.
 */
int load_font_file(const char *filepath);

/* Look up the stroke range for a given ASCII code.
 * On success, writes the start index and count into the output pointers.
 * Returns 0 if found, non-zero if the glyph does not exist.
 */
int find_glyph(int ascii_code, int *start_index, int *count);

/* Gives read-only access to the internal stroke buffer.
 * The size and indexing are described by find_glyph().
 */
const StrokePoint *get_stroke_buffer(void);

#endif /* GLYPH_STORE_H */
