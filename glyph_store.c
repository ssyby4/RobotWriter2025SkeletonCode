#include <stdio.h>
#include "glyph_store.h"

#define MAX_STROKES  2048
#define MAX_GLYPHS   128

typedef struct
{
    int ascii;
    int start;
    int count;
} GlyphIndex;

static StrokePoint stroke_buffer[MAX_STROKES];
static GlyphIndex  glyph_table[MAX_GLYPHS];

static int stroke_used = 0;
static int glyph_used  = 0;

const StrokePoint *get_stroke_buffer(void)
{
    return stroke_buffer;
}

int find_glyph(int ascii_code, int *start_index, int *count)
{
    int i;
    for (i = 0; i < glyph_used; ++i)
    {
        if (glyph_table[i].ascii == ascii_code)
        {
            if (start_index) *start_index = glyph_table[i].start;
            if (count)       *count       = glyph_table[i].count;
            return 0;
        }
    }
    return 1; /* not found */
}

int load_font_file(const char *filepath)
{
    FILE *fp;
    int a, b, c;
    int line_no = 0;

    stroke_used = 0;
    glyph_used  = 0;

    fp = fopen(filepath, "r");
    if (!fp)
    {
        printf("[ERROR] Cannot open font file: %s\n", filepath);
        return 1;
    }

    while (fscanf(fp, "%d %d %d", &a, &b, &c) == 3)
    {
        ++line_no;

        if (a == 999)
        {
            /* glyph header: a = 999, b = ASCII code, c = number of strokes */
            if (glyph_used >= MAX_GLYPHS)
            {
                printf("[ERROR] Glyph table overflow at line %d.\n", line_no);
                fclose(fp);
                return 1;
            }

            glyph_table[glyph_used].ascii = b;
            glyph_table[glyph_used].start = stroke_used;
            glyph_table[glyph_used].count = c;
            glyph_used++;
        }
        else
        {
            /* stroke definition: a = x, b = y, c = pen */
            if (stroke_used >= MAX_STROKES)
            {
                printf("[ERROR] Stroke buffer overflow at line %d.\n", line_no);
                fclose(fp);
                return 1;
            }

            stroke_buffer[stroke_used].x   = a;
            stroke_buffer[stroke_used].y   = b;
            stroke_buffer[stroke_used].pen = c;
            stroke_used++;
        }
    }

    fclose(fp);

    if (glyph_used == 0 || stroke_used == 0)
    {
        printf("[ERROR] Font file did not contain any glyphs.\n");
        return 1;
    }

    printf("[INFO] Loaded %d glyphs and %d strokes from %s.\n",
           glyph_used, stroke_used, filepath);

    return 0;
}
