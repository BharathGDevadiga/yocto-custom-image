# bharath-image.bb
#
# Custom image based on core-image-minimal that includes
# sensor-simulator, netcomm, and the sensor-kmod kernel driver.

SUMMARY = "Bharath's custom Yocto image with kernel sensor driver and networking tools"

# Inherit from core-image so we get the standard image framework
inherit core-image

# Core user-space applications and kernel modules from meta-bharath
IMAGE_INSTALL:append = " sensor-simulator netcomm sensor-kmod"

# Include SSH server for remote access (useful for QEMU testing)
IMAGE_FEATURES += "ssh-server-dropbear"

# Ensure networking tools are available
IMAGE_INSTALL:append = " iproute2 iputils-ping"
