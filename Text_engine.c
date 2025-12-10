#include <stdio.h>
#include <string.h>
#include "Text_engine.h"
#include "glyph_painter.h"

#define LINE_LIMIT_MM   100.0f
#define BASE_FONT_UNITS 18.0f

int render_text_file(const char *filepath, float height_mm)
{
    FILE *fp;
    int ch;
    char word[256];
    int  w_len = 0;

    float scale      = height_mm / BASE_FONT_UNITS;
    float cell_width = height_mm * 0.9f;
    float line_step  = height_mm * 1.3f;

    float x = 0.0f;
    float y = 0.0f;

    fp = fopen(filepath, "r");
    if (!fp)
    {
        printf("[ERROR] Cannot open text file: %s\n", filepath);
        return 1;
    }

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch != ' ' && ch != '\r' && ch != '\n')
        {
            if (w_len < (int)(sizeof(word) - 1))
            {
                word[w_len++] = (char)ch;
            }
            continue;
        }

        /* end of a word */
        if (w_len > 0)
        {
            float word_width = (float)w_len * cell_width;

            if (word_width > LINE_LIMIT_MM)
            {
                printf("[ERROR] Single word too long for one line.\n");
                fclose(fp);
                return 1;
            }

            if (x + word_width > LINE_LIMIT_MM)
            {
                x  = 0.0f;
                y -= line_step;
            }

            for (int i = 0; i < w_len; ++i)
            {
                if (draw_glyph((unsigned char)word[i], &x, &y, scale) != 0)
                {
                    fclose(fp);
                    return 1;
                }
                x += cell_width;
            }

            w_len = 0;
            memset(word, 0, sizeof(word));
        }

        /* handle separator */
        if (ch == ' ')
        {
            if (x + cell_width > LINE_LIMIT_MM)
            {
                x  = 0.0f;
                y -= line_step;
            }
            else
            {
                x += cell_width;
            }
        }
        else if (ch == '\r' || ch == '\n')
        {
            x  = 0.0f;
            y -= line_step;
        }
    }

    /* process last word if file did not end with separator */
    if (w_len > 0)
    {
        float word_width = (float)w_len * cell_width;

        if (word_width > LINE_LIMIT_MM)
        {
            printf("[ERROR] Single word too long for one line.\n");
            fclose(fp);
            return 1;
        }

        if (x + word_width > LINE_LIMIT_MM)
        {
            x  = 0.0f;
            y -= line_step;
        }

        for (int i = 0; i < w_len; ++i)
        {
            if (draw_glyph((unsigned char)word[i], &x, &y, scale) != 0)
            {
                fclose(fp);
                return 1;
            }
            x += cell_width;
        }
    }

    fclose(fp);
    return 0;
}
