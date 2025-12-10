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

    /* Wake-up handshake and wait for '$' or 'ok' */
    initialise_plotter();
    printf("\nThe robot is now ready to draw\n");

    /* Move to origin, set feed rate, ensure pen is up */
    prepare_motion();

    /* Load font description into memory */
    rc = load_font_file("SingleStrokeFont.txt");
    if (rc != 0)
    {
        printf("[ERROR] Failed to load font file.\n");
        finalise_motion();
        CloseRS232Port();
        return 0;
    }

    /* Ask the user for a valid character height */
    printf("\nEnter character height (4–10 mm): ");
    fflush(stdout);

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

    /* Render the full text file using the chosen height */
    rc = render_text_file(inputFile, (float)height_mm);
    if (rc != 0)
    {
        printf("[ERROR] Rendering failed (code %d).\n", rc);
        finalise_motion();
        CloseRS232Port();
        return 0;
    }

    printf("\n[INFO] Rendering complete.\n");

    /* Park the robot and make sure the pen is up */
    finalise_motion();

    /* Close communication channel */
    CloseRS232Port();
    printf("[INFO] COM port closed.\n");

    return 0;
}

/* Send one line of G-code and wait for the reply */
static void issue_line(const char *line)
{
    PrintBuffer((char *)line);
    WaitForReply();
    Sleep(100);
}

/* Perform the initial handshake with the robot/emulator */
static void initialise_plotter(void)
{
    char buf[16];

    /* A blank line is enough to wake up the robot code */
    sprintf(buf, "\n");
    PrintBuffer(buf);
    Sleep(100);
    WaitForDollar();
}

/* Put the robot into “ready to draw” mode:
 * origin, feed rate, motor on, pen up.
 */
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

/* Finish the drawing: make sure pen is up and go back to origin */
static void finalise_motion(void)
{
    char cmd[64];

    sprintf(cmd, "S0\n");
    issue_line(cmd);

    sprintf(cmd, "G0 X0 Y0\n");
    issue_line(cmd);
}
