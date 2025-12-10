#include <stdio.h>
#include <ctype.h>

#include "glyph_store.h"
#include "glyph_painter.h"
#include "Text_engine.h"

/* Simple layout constants for spacing between characters and lines */
#define BASE_FONT_UNITS 18.0f      /* nominal font height in font units    */
#define LINE_SPACING    1.2f       /* line spacing multiplier              */
#define SPACE_WIDTH     0.6f       /* width of a space relative to height  */

/* Read a text file one character at a time and draw it. */
int render_text_file(const char *filepath, float height_mm)
{
    FILE *fp;
    int ch;
    float scale;
    float cursor_x = 0.0f;
    float cursor_y = 0.0f;
    int rc = 0;

    fp = fopen(filepath, "r");
    if (!fp)
    {
        printf("[ERROR] Cannot open text file: %s\n", filepath);
        return 1;
    }

    /* Convert desired height in mm to a scale for font units */
    scale = height_mm / BASE_FONT_UNITS;

    while ((ch = fgetc(fp)) != EOF)
    {
        /* Ignore carriage returns in Windows style line endings */
        if (ch == '\r')
            continue;

        if (ch == '\n')
        {
            /* Move down one line and reset to left margin */
            cursor_y -= height_mm * LINE_SPACING;
            cursor_x  = 0.0f;
            continue;
        }

        if (ch == ' ')
        {
            /* Simple horizontal advance for spaces */
            cursor_x += height_mm * SPACE_WIDTH;
            continue;
        }

        /* Draw printable characters only */
        if (!isprint(ch))
            continue;

        rc = draw_glyph(ch, &cursor_x, &cursor_y, scale);
        if (rc != 0)
        {
            printf("[ERROR] Failed to draw character '%c'.\n", ch);
            break;
        }

        /* Advance origin for the next character.
         * Using the nominal width here keeps things simple.
         */
        cursor_x += height_mm;
    }

    fclose(fp);
    return rc;
}
