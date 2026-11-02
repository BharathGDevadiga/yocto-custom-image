/*
 * sensor_kmod.c
 *
 * Linux 6.6 Compatible Character Device Driver for Virtual Bharath Sensor
 * Exposes /dev/bharath_sensor for real-time telemetry streaming to user space.
 *
 * Author: Bharath
 * License: GPL
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/random.h>

#define DEVICE_NAME "bharath_sensor"
#define CLASS_NAME  "bharath_class"

/* Packed binary struct shared with user-space application */
struct bharath_sensor_data {
    u64 timestamp_jiffies; /* 64-bit kernel timer jiffies */
    s32 temperature_mC;    /* Temperature in milli-degrees Celsius (e.g. 24500 = 24.5°C) */
    u32 pressure_Pa;       /* Atmospheric pressure in Pascals (e.g. 101325 Pa) */
} __attribute__((packed));

static dev_t dev_num;
static struct cdev sensor_cdev;
static struct class *sensor_class = NULL;
static struct device *sensor_device = NULL;

static int sensor_open(struct inode *inode, struct file *file)
{
    pr_info("sensor_kmod: Device /dev/%s opened\n", DEVICE_NAME);
    return 0;
}

static int sensor_release(struct inode *inode, struct file *file)
{
    pr_info("sensor_kmod: Device /dev/%s closed\n", DEVICE_NAME);
    return 0;
}

/*
 * ============================================================================
 * SENSOR READ: Transfer telemetry from kernel to user space
 * ============================================================================
 * HOW TO ADAPT THIS FUNCTION FOR REAL HARDWARE (I2C / SPI / ADC / IIO):
 *
 * Currently, for headless QEMU emulation, this driver synthesizes realistic
 * environmental data using 64-bit kernel jiffies and hardware entropy.
 *
 * To connect a physical sensor on a real target board (e.g. Raspberry Pi,
 * BeagleBone, NXP i.MX, STM32MP1, or x86 SBC):
 *
 * 1. FOR REAL I2C SENSORS (e.g., Bosch BMP280, BME680, MPU6050):
 *    - Include <linux/i2c.h>
 *    - Store the `struct i2c_client *client` from your i2c_driver probe().
 *    - Read raw registers:
 *        s32 raw_t = i2c_smbus_read_word_data(client, BMP280_TEMP_REG);
 *        s32 raw_p = i2c_smbus_read_word_data(client, BMP280_PRESS_REG);
 *    - Convert and assign to:
 *        data.temperature_mC = bmp280_compensate_temp(raw_t);
 *        data.pressure_Pa    = bmp280_compensate_press(raw_p);
 *
 * 2. FOR REAL SPI SENSORS:
 *    - Include <linux/spi/spi.h>
 *    - Use spi_sync_transfer(spi_dev, &transfer, 1) to read ADC registers.
 *
 * 3. FOR DEVICE TREE MATCHING:
 *    - Define an `of_device_id` table:
 *        static const struct of_device_id sensor_dt_ids[] = {
 *            { .compatible = "bharath,sensor-v1" },
 *            { /* sentinel */ }
 *        };
 *        MODULE_DEVICE_TABLE(of, sensor_dt_ids);
 * ============================================================================
 */
static ssize_t sensor_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct bharath_sensor_data data;
    u32 rand_temp = 0;
    u32 rand_press = 0;

    if (count < sizeof(data)) {
        pr_warn("sensor_kmod: Read buffer too small (%zu < %zu)\n", count, sizeof(data));
        return -EINVAL;
    }

    /* 
     * STEP 1: Capture 64-bit kernel timer jiffies for high-precision
     * monotonic timestamping (unaffected by wall-clock time shifts).
     */
    data.timestamp_jiffies = get_jiffies_64();

    /* 
     * STEP 2: Acquire telemetry data.
     * [CHANGE POINT FOR REAL SENSORS]: Replace the mock generator below
     * with your physical I2C/SPI bus read calls.
     */
    get_random_bytes(&rand_temp, sizeof(rand_temp));
    get_random_bytes(&rand_press, sizeof(rand_press));

    /* Simulated Range: 20000 mC to 35000 mC (20.00°C to 35.00°C) */
    data.temperature_mC = 20000 + (s32)(rand_temp % 15001);

    /* Simulated Range: 98000 Pa to 104000 Pa (980.00 hPa to 1040.00 hPa) */
    data.pressure_Pa = 98000 + (rand_press % 6001);

    /* 
     * STEP 3: Safe kernel-to-user memory copy.
     * copy_to_user ensures atomic transfer and memory boundary safety.
     */
    if (copy_to_user(buf, &data, sizeof(data))) {
        pr_err("sensor_kmod: Failed to copy sensor data to user space\n");
        return -EFAULT;
    }

    return sizeof(data);
}

static const struct file_operations sensor_fops = {
    .owner   = THIS_MODULE,
    .open    = sensor_open,
    .release = sensor_release,
    .read    = sensor_read,
};

static char *sensor_devnode(const struct device *dev, umode_t *mode)
{
    if (mode)
        *mode = 0666; /* Allow read/write for non-root users if needed */
    return NULL;
}

static int __init sensor_kmod_init(void)
{
    int ret;

    pr_info("sensor_kmod: Initializing Bharath virtual sensor driver...\n");

    /* Allocate major/minor numbers dynamically */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("sensor_kmod: Failed to allocate major number (error %d)\n", ret);
        return ret;
    }

    /* Initialize and register character device structure */
    cdev_init(&sensor_cdev, &sensor_fops);
    sensor_cdev.owner = THIS_MODULE;

    ret = cdev_add(&sensor_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("sensor_kmod: Failed to add cdev (error %d)\n", ret);
        goto unregister_chrdev;
    }

    /*
     * Linux 6.4+ compatible class_create: single argument (name only)
     * Replaces legacy class_create(THIS_MODULE, name)
     */
    sensor_class = class_create(CLASS_NAME);
    if (IS_ERR(sensor_class)) {
        ret = PTR_ERR(sensor_class);
        pr_err("sensor_kmod: Failed to create device class '%s' (error %d)\n", CLASS_NAME, ret);
        goto del_cdev;
    }

    sensor_class->devnode = sensor_devnode;

    /* Create device node so eudev automatically generates /dev/bharath_sensor */
    sensor_device = device_create(sensor_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(sensor_device)) {
        ret = PTR_ERR(sensor_device);
        pr_err("sensor_kmod: Failed to create device /dev/%s (error %d)\n", DEVICE_NAME, ret);
        goto destroy_class;
    }

    pr_info("sensor_kmod: Loaded successfully. /dev/%s created (Major %d, Minor %d)\n",
            DEVICE_NAME, MAJOR(dev_num), MINOR(dev_num));
    return 0;

destroy_class:
    class_destroy(sensor_class);
del_cdev:
    cdev_del(&sensor_cdev);
unregister_chrdev:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit sensor_kmod_exit(void)
{
    device_destroy(sensor_class, dev_num);
    class_destroy(sensor_class);
    cdev_del(&sensor_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("sensor_kmod: Unloaded successfully\n");
}

module_init(sensor_kmod_init);
module_exit(sensor_kmod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharath");
MODULE_DESCRIPTION("Virtual Linux Kernel Character Driver for Bharath Sensor");
MODULE_VERSION("1.0");
