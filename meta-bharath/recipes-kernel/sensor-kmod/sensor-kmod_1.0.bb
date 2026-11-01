SUMMARY = "Linux Kernel Character Driver for Bharath Virtual Sensor"
DESCRIPTION = "Virtual character device driver providing /dev/bharath_sensor"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = "file://Makefile \
           file://sensor_kmod.c \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "sensor_kmod"
RPROVIDES:${PN} += "kernel-module-sensor-kmod"
