#include <stdio.h>
#include <math.h>
#include "glyph_painter.h"
#include "glyph_store.h"
#include "serial.h"


#define WORKSPACE_X_MAX 100.0f
#define WORKSPACE_Y_MIN -50.0f

static void send_move(float x, float y, int pen_down, int *current_state)
{
    char buffer[64];

    if (pen_down)
    {
        if (*current_state == 0)
        {
            sprintf(buffer, "S1000\n");
            PrintBuffer(buffer);
            WaitForReply();
        }
        sprintf(buffer, "G1 X%.2f Y%.2f\n", x, y);
        PrintBuffer(buffer);
        WaitForReply();
        *current_state = 1;
    }
    else
    {
        if (*current_state == 1)
        {
            sprintf(buffer, "S0\n");
            PrintBuffer(buffer);
            WaitForReply();
        }
        sprintf(buffer, "G0 X%.2f Y%.2f\n", x, y);
        PrintBuffer(buffer);
        WaitForReply();
        *current_state = 0;
    }
}

/* check bounds without sending any G-code */
static int check_glyph_bounds(int start, int count,
                              float origin_x, float origin_y,
                              float scale)
{
    const StrokePoint *buf = get_stroke_buffer();
    float max_x = -1e9f;
    float min_y =  1e9f;
    int i;

    for (i = 0; i < count; ++i)
    {
        float gx = (float)buf[start + i].x;
        float gy = (float)buf[start + i].y;

        float wx = origin_x + gx * scale;
        float wy = origin_y + gy * scale;

        if (wx > max_x) max_x = wx;
        if (wy < min_y) min_y = wy;
    }

    if (max_x > WORKSPACE_X_MAX || min_y < WORKSPACE_Y_MIN)
    {
        printf("[ERROR] Glyph would exceed drawing area (%.1f x %.1f).\n",
               WORKSPACE_X_MAX, fabsf(WORKSPACE_Y_MIN));
        return 1;
    }
    return 0;
}

int draw_glyph(int ascii_code, float *cursor_x, float *cursor_y, float scale)
{
    int start, count;
    const StrokePoint *buf;
    int pen_state = 0;
    int i;

    if (find_glyph(ascii_code, &start, &count) != 0)
    {
        printf("[WARN] Unsupported character '%c' (code %d).\n",
               (char)ascii_code, ascii_code);
        return 0; /* skip but do not treat as fatal error */
    }

    buf = get_stroke_buffer();

    /* first pass: safety check */
    if (check_glyph_bounds(start, count, *cursor_x, *cursor_y, scale) != 0)
    {
        return 1;
    }

    /* second pass: send G-code */
    for (i = 0; i < count; ++i)
    {
        float gx = (float)buf[start + i].x;
        float gy = (float)buf[start + i].y;
        int pen  = buf[start + i].pen;

        float wx = *cursor_x + gx * scale;
        float wy = *cursor_y + gy * scale;

        send_move(wx, wy, pen, &pen_state);
    }

    /* ensure pen-up at the end of this glyph */
    if (pen_state != 0)
    {
        char buffer[16];
        sprintf(buffer, "S0\n");
        PrintBuffer(buffer);
        WaitForReply();
    }

    return 0;
}
