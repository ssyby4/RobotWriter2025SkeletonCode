#ifndef glyph_store_h
#define glyph_store_h

typedef struct
{
    int x;
    int y;
    int pen;   /* 0 = pen up, 1 = pen down */
} StrokePoint;

int load_font_file(const char *filepath);

/* Lookup glyph stroke range for a given ASCII code */
int find_glyph(int ascii_code, int *start_index, int *count);

/* Access the internal stroke buffer */
const StrokePoint *get_stroke_buffer(void);

#endif
