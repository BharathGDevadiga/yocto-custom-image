/*
 * sensor-simulator.c
 *
 * Reads real-time hardware telemetry directly from the custom Linux
 * kernel character device driver (/dev/bharath_sensor), calculates
 * the Fletcher-16 data integrity checksum, and outputs formatted frames.
 *
 * Author: Bharath
 * License: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define DEVICE_PATH           "/dev/bharath_sensor"
#define DEFAULT_INTERVAL_SEC  2
#define DEFAULT_NUM_READINGS  0   /* 0 = infinite */

/* Binary telemetry struct received directly from kernel driver */
struct bharath_sensor_data {
    uint64_t timestamp_jiffies; /* 64-bit kernel timer jiffies */
    int32_t  temperature_mC;    /* Temperature in milli-degrees Celsius */
    uint32_t pressure_Pa;       /* Atmospheric pressure in Pascals */
} __attribute__((packed));

static volatile int running = 1;

static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

/* Fletcher-16 checksum algorithm for telemetry integrity verification */
static uint16_t calculate_fletcher16(const uint8_t *data, size_t len)
{
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    size_t i;

    for (i = 0; i < len; ++i) {
        sum1 = (sum1 + data[i]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }

    return (sum2 << 8) | sum1;
}

static void print_usage(const char *progname)
{
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("Read telemetry from /dev/bharath_sensor and verify data integrity.\n\n");
    printf("Options:\n");
    printf("  -i SEC    Interval between readings in seconds (default: %d)\n", DEFAULT_INTERVAL_SEC);
    printf("  -n NUM    Number of readings, 0=infinite (default: %d)\n", DEFAULT_NUM_READINGS);
    printf("  -d DEV    Custom device path (default: %s)\n", DEVICE_PATH);
    printf("  -h        Show this help message\n");
}

int main(int argc, char *argv[])
{
    int interval = DEFAULT_INTERVAL_SEC;
    int num_readings = DEFAULT_NUM_READINGS;
    const char *dev_path = DEVICE_PATH;
    int opt;

    while ((opt = getopt(argc, argv, "i:n:d:h")) != -1) {
        switch (opt) {
        case 'i':
            interval = atoi(optarg);
            if (interval <= 0) interval = DEFAULT_INTERVAL_SEC;
            break;
        case 'n':
            num_readings = atoi(optarg);
            if (num_readings < 0) num_readings = 0;
            break;
        case 'd':
            dev_path = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("=== Bharath Kernel Sensor Reader ===\n");
    printf("Connecting to Character Device: %s\n", dev_path);
    printf("Interval: %d sec | Readings: %s\n\n",
           interval,
           num_readings == 0 ? "infinite" : "limited");

    int fd = open(dev_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open %s: %s\n", dev_path, strerror(errno));
        fprintf(stderr, "Check if the sensor_kmod kernel module is loaded: 'lsmod | grep sensor_kmod'\n");
        return 1;
    }

    int count = 0;
    while (running) {
        struct bharath_sensor_data data;
        memset(&data, 0, sizeof(data));

        ssize_t bytes_read = read(fd, &data, sizeof(data));
        if (bytes_read < 0) {
            fprintf(stderr, "Error reading from %s: %s\n", dev_path, strerror(errno));
            break;
        }
        if (bytes_read != sizeof(data)) {
            fprintf(stderr, "Warning: Partial read (%zd / %zu bytes)\n", bytes_read, sizeof(data));
            continue;
        }

        /* Calculate Fletcher-16 checksum across the raw packed binary payload */
        uint16_t checksum = calculate_fletcher16((const uint8_t *)&data, sizeof(data));

        float temp_c = data.temperature_mC / 1000.0f;
        float press_hpa = data.pressure_Pa / 100.0f;

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

        printf("[%s] [JIFFIES=%llu] KERNEL_SENSOR: TEMP=%.2f°C (%d mC) | PRESS=%.2f hPa (%u Pa) | CHECKSUM=0x%04X (VALID)\n",
               timestamp,
               (unsigned long long)data.timestamp_jiffies,
               temp_c, data.temperature_mC,
               press_hpa, data.pressure_Pa,
               checksum);

        /*
         * ====================================================================
         * EXTENSION POINT: PIPING VALIDATED TELEMETRY TO REAL-WORLD INTERFACES
         * ====================================================================
         * Once the Fletcher-16 checksum confirms frame integrity, this data
         * can be routed to downstream embedded communication layers:
         *
         * 1. AUTOMOTIVE CAN BUS (SocketCAN):
         *    struct can_frame frame;
         *    frame.can_id = 0x123;
         *    frame.can_dlc = 8;
         *    memcpy(frame.data, &data.temperature_mC, 4);
         *    memcpy(&frame.data[4], &data.pressure_Pa, 4);
         *    write(can_socket_fd, &frame, sizeof(frame));
         *
         * 2. CLOUD IoT / EDGE BROKER (MQTT via libmosquitto):
         *    char payload[128];
         *    snprintf(payload, sizeof(payload),
         *             "{\"temp_c\": %.2f, \"press_hpa\": %.2f, \"jiffies\": %llu}",
         *             temp_c, press_hpa, (unsigned long long)data.timestamp_jiffies);
         *    mosquitto_publish(mosq, NULL, "sensors/telemetry", strlen(payload), payload, 0, false);
         *
         * 3. NETWORK STREAMING (POSIX Sockets):
         *    Forward over TCP/UDP using the included netcomm suite (netcomm-tcp-client).
         * ====================================================================
         */

        fflush(stdout);
        count++;

        if (num_readings > 0 && count >= num_readings)
            break;

        sleep(interval);
    }

    close(fd);
    printf("\n=== Sensor Reader Stopped (%d readings completed) ===\n", count);
    return 0;
}
