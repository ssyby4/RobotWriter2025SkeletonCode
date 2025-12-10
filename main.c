#include <stdio.h>
#include "rs232.h"
#include "serial.h"
#include "glyph_store.h"
#include "glyph_painter.h"
#include "Text_engine.h"

/* Local helper prototypes */
static void issue_line(const char *line);
static void initialise_plotter(void);
static void prepare_motion(void);
static void finalise_motion(void);

int main(void)
{
    char inputFile[256] = "test.txt";
    int height_mm = 0;
    int rc;

    printf("About to wake up the robot\n\n");

    /* Open serial or emulator channel */
    if (CanRS232PortBeOpened() == -1)
    {
        printf("[ERROR] Could not open COM port.\n");
        return 0;
    }

    initialise_plotter();
    printf("\nThe robot is now ready to draw\n");

    /* Move to origin, set speed, ensure pen-up */
    prepare_motion();

    /* Load font description */
    rc = load_font_file("SingleStrokeFont.txt");
    if (rc != 0)
    {
        printf("[ERROR] Failed to load font file.\n");
        finalise_motion();
        CloseRS232Port();
        return 0;
    }

    /* ===== 关键：询问高度 ===== */
    printf("\nEnter character height (4–10 mm): ");
    fflush(stdout);              /* 确保提示立即显示 */

    if (scanf("%d", &height_mm) != 1)
    {
        printf("[ERROR] Invalid input.\n");
        finalise_motion();
        CloseRS232Port();
        return 0;
    }

    if (height_mm < 4 || height_mm > 10)
    {
        printf("[ERROR] Height out of range.\n");
        finalise_motion();
        CloseRS232Port();
        return 0;
    }

    /* ===== 调用文本渲染 ===== */
    rc = render_text_file(inputFile, (float)height_mm);
    if (rc != 0)
    {
        printf("[ERROR] Rendering failed (code %d).\n", rc);
        finalise_motion();
        CloseRS232Port();
        return 0;
    }

    printf("\n[INFO] Rendering complete.\n");

    /* Pen-up and return to home */
    finalise_motion();

    /* Close communication channel */
    CloseRS232Port();
    printf("[INFO] COM port closed.\n");

    return 0;
}


/* Helper functions */


static void issue_line(const char *line)
{
    PrintBuffer((char *)line);
    WaitForReply();
    Sleep(100);
}

static void initialise_plotter(void)
{
    char buf[16];
    sprintf(buf, "\n");
    PrintBuffer(buf);
    Sleep(100);
    WaitForDollar();
}

static void prepare_motion(void)
{
    char cmd[64];

    sprintf(cmd, "G1 X0 Y0 F1000\n");
    issue_line(cmd);

    sprintf(cmd, "M3\n");
    issue_line(cmd);

    sprintf(cmd, "S0\n");
    issue_line(cmd);
}

static void finalise_motion(void)
{
    char cmd[64];

    sprintf(cmd, "S0\n");
    issue_line(cmd);

    sprintf(cmd, "G0 X0 Y0\n");
    issue_line(cmd);
}

