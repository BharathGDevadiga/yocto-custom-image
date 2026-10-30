# bharath-image.bb
#
# Custom image based on core-image-minimal that includes
# the sensor-simulator and netcomm packages from meta-bharath.

SUMMARY = "Bharath's custom Yocto image with sensor simulator and networking tools"

# Inherit from core-image so we get the standard image framework
inherit core-image

# Start from core-image-minimal's package set
IMAGE_INSTALL:append = " sensor-simulator netcomm"

# Include SSH server for remote access (useful for QEMU testing)
IMAGE_FEATURES += "ssh-server-dropbear"

# Ensure networking tools are available
IMAGE_INSTALL:append = " iproute2 iputils-ping"
