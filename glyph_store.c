#include <stdio.h>
#include "glyph_store.h"

#define MAX_STROKES  2048
#define MAX_GLYPHS   128

/* Simple index for each glyph inside the stroke buffer */
typedef struct
{
    int ascii;
    int start;
    int count;
} GlyphIndex;

/* Global storage for all strokes and glyph index entries */
static StrokePoint stroke_buffer[MAX_STROKES];
static GlyphIndex  glyph_table[MAX_GLYPHS];

static int stroke_used = 0;
static int glyph_used  = 0;

/* Return pointer to internal stroke buffer.
 * The caller must not modify the data.
 */
const StrokePoint *get_stroke_buffer(void)
{
    return stroke_buffer;
}

/* Linear search for a glyph entry by ASCII code.
 * Writes start index and count when a match is found.
 */
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

/* Read the font file and rebuild stroke_buffer and glyph_table.
 * Expected format is "999 C N" headers followed by N stroke lines.
 */
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
            /* Header: define a new glyph entry */
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
            /* Stroke: add one more drawing command */
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
