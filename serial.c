#include <stdio.h>
#include <stdlib.h>

#include "serial.h"
/* #include "rs232.h"  // normally pulled in by serial.h if needed */

/* ------------- Simple G-code logging support ------------- */
/* All lines sent via PrintBuffer() are also written to this file. */

static FILE *gcode_log = NULL;

/* Open log file on first use, overwrite any previous content. */
static void open_gcode_log(void)
{
    if (gcode_log == NULL)
    {
        /* Make sure you have an "output" folder in the project root */
        gcode_log = fopen("output/output.gcode", "w");
        if (!gcode_log)
        {
            /* Logging is optional, so do not abort on failure */
            printf("[WARN] Could not open output/output.gcode for writing.\n");
        }
    }
}

/* Close the log file when we are done. */
static void close_gcode_log(void)
{
    if (gcode_log != NULL)
    {
        fclose(gcode_log);
        gcode_log = NULL;
    }
}

/* --------------------------------------------------------- */
/*                  Serial / Emulator modes                  */
/* --------------------------------------------------------- */

/* Uncomment this line when using real serial hardware. */
/* #define Serial_Mode */

#ifdef Serial_Mode

/* Open port with checking */
int CanRS232PortBeOpened(void)
{
    char mode[] = { '8', 'N', '1', 0 };
    if (RS232_OpenComport(cport_nr, bdrate, mode))
    {
        printf("Can not open comport\n");
        return -1;
    }
    return 0;      /* Success */
}

/* Function to close the COM port */
void CloseRS232Port(void)
{
    RS232_CloseComport(cport_nr);
    close_gcode_log();
}

/* Write text out via the serial port (and log it) */
int PrintBuffer(char *buffer)
{
    RS232_cputs(cport_nr, buffer);
    printf("sent: %s\n", buffer);

    /* Also log to file */
    open_gcode_log();
    if (gcode_log)
    {
        fputs(buffer, gcode_log);
        fflush(gcode_log);
    }

    return 0;
}

/* Wait for '$' or "ok" from the controller */
int WaitForDollar(void)
{
    int i, n;
    unsigned char buf[4096];

    while (1)
    {
        printf(".");
        n = RS232_PollComport(cport_nr, buf, 4095);

        if (n > 0)
        {
            printf("RCVD: N = %d ", n);
            buf[n] = 0;   /* always zero-terminate */

            for (i = 0; i < n; i++)
            {
                if (buf[i] == '$')
                {
                    printf("received %i bytes: %s \n", n, (char *)buf);
                    printf("\nSaw the Dollar");
                    return 0;
                }
            }

            printf("received %i bytes: %s \n", n, (char *)buf);

            if ((buf[0] == 'o') && (buf[1] == 'k'))
                return 0;
        }

        Sleep(100);
    }

    return 0;
}

/* Wait for an "ok" reply from the controller */
int WaitForReply(void)
{
    int i, n;
    unsigned char buf[4096];

    while (1)
    {
        printf(".");
        n = RS232_PollComport(cport_nr, buf, 4095);

        if (n > 0)
        {
            printf("RCVD: N = %d ", n);
            buf[n] = 0;   /* always zero-terminate */

            for (i = 0; i < n; i++)
            {
                if (buf[i] < 32)
                {
                    buf[i] = '.';
                }
            }

            printf("received %i bytes: %s\n", n, (char *)buf);

            if ((buf[0] == 'o') && (buf[1] == 'k'))
                return 0;
        }

        Sleep(100);
    }

    return 0;
}

#else  /* -------- PC / emulator mode (no real serial) -------- */

/* Open port with checking (always OK in emulator mode) */
int CanRS232PortBeOpened(void)
{
    return 0;
}

/* Function to close the "port" (just close the log file) */
void CloseRS232Port(void)
{
    close_gcode_log();
}

/* Print buffer to console and also log it to a file. */
int PrintBuffer(char *buffer)
{
    printf("%s\n", buffer);

    /* Also log to file */
    open_gcode_log();
    if (gcode_log)
    {
        fputs(buffer, gcode_log);
        fflush(gcode_log);
    }

    return 0;
}

/* In emulator mode we do not need to wait for any reply. */
int WaitForReply(void)
{
    return 0;
}

/* In emulator mode we also do not need to wait for '$'. */
int WaitForDollar(void)
{
    return 0;
}

#endif /* Serial_Mode */
