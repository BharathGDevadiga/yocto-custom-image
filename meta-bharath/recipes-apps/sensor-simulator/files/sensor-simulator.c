/*
 * sensor-simulator.c
 *
 * Simulates a UART-connected sensor (temperature + humidity)
 * Outputs periodic readings to stdout, mimicking what you'd
 * read from /dev/ttyS0 on real hardware.
 *
 * Author: Bharath
 * License: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define DEFAULT_INTERVAL_SEC  2
#define DEFAULT_NUM_READINGS  0   /* 0 = infinite */

/* Simulated sensor ranges */
#define TEMP_BASE    25.0f
#define TEMP_RANGE   10.0f
#define HUMID_BASE   40.0f
#define HUMID_RANGE  30.0f

static volatile int running = 1;

static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

static float random_float(float base, float range)
{
    return base + ((float)rand() / (float)RAND_MAX) * range;
}

static void print_usage(const char *progname)
{
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("Simulate a UART sensor producing temperature/humidity readings.\n\n");
    printf("Options:\n");
    printf("  -i SEC    Interval between readings (default: %d)\n", DEFAULT_INTERVAL_SEC);
    printf("  -n NUM    Number of readings, 0=infinite (default: %d)\n", DEFAULT_NUM_READINGS);
    printf("  -h        Show this help message\n");
}

int main(int argc, char *argv[])
{
    int interval = DEFAULT_INTERVAL_SEC;
    int num_readings = DEFAULT_NUM_READINGS;
    int opt;

    while ((opt = getopt(argc, argv, "i:n:h")) != -1) {
        switch (opt) {
        case 'i':
            interval = atoi(optarg);
            if (interval <= 0) interval = DEFAULT_INTERVAL_SEC;
            break;
        case 'n':
            num_readings = atoi(optarg);
            if (num_readings < 0) num_readings = 0;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Set up clean shutdown on SIGINT/SIGTERM */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    srand((unsigned int)time(NULL));

    printf("=== Sensor Simulator Started ===\n");
    printf("Simulating UART sensor on /dev/ttyS0 (115200 8N1)\n");
    printf("Interval: %d sec | Readings: %s\n\n",
           interval,
           num_readings == 0 ? "infinite" : "limited");

    int count = 0;
    while (running) {
        float temperature = random_float(TEMP_BASE, TEMP_RANGE);
        float humidity    = random_float(HUMID_BASE, HUMID_RANGE);
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

        /* Output in a format similar to real sensor UART frames */
        printf("[%s] SENSOR_DATA: TEMP=%.2f°C HUMID=%.2f%% CHECKSUM=0x%02X\n",
               timestamp,
               temperature,
               humidity,
               ((unsigned char)(temperature * 10) ^ (unsigned char)(humidity * 10)) & 0xFF);

        fflush(stdout);
        count++;

        if (num_readings > 0 && count >= num_readings)
            break;

        sleep(interval);
    }

    printf("\n=== Sensor Simulator Stopped (%d readings) ===\n", count);
    return 0;
}
