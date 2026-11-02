SUMMARY = "Linux Kernel Character Driver for Bharath Virtual Sensor"
DESCRIPTION = "Virtual character device driver providing /dev/bharath_sensor"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

# Inheriting 'module' tells BitBake to cross-compile against the Linux kernel headers
# located in ${STAGING_KERNEL_BUILDDIR} using Kbuild and module.bbclass.
inherit module

SRC_URI = "file://Makefile \
           file://sensor_kmod.c \
          "

S = "${WORKDIR}"

# KERNEL_MODULE_AUTOLOAD ensures systemd/eudev loads sensor_kmod.ko automatically at boot
KERNEL_MODULE_AUTOLOAD += "sensor_kmod"

# Satisfies kernel package runtime dependencies for rootfs image generation
RPROVIDES:${PN} += "kernel-module-sensor-kmod"
