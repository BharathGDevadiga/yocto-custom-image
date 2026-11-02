SUMMARY = "Simulated UART sensor application"
DESCRIPTION = "A C program that simulates a UART-connected temperature and \
humidity sensor, producing periodic readings to stdout. Useful for testing \
embedded data pipelines without real hardware."
HOMEPAGE = "https://github.com/bharath/yocto-custom-image"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://sensor-simulator.c"

S = "${WORKDIR}"

# BitBake Task: do_compile
# Cross-compiles the C binary for the target architecture (x86_64-poky-linux-gcc)
# using OE cross-compiler flags (${CC}, ${CFLAGS}, ${LDFLAGS}).
do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o sensor-simulator sensor-simulator.c
}

# BitBake Task: do_install
# Stages the cross-compiled binary into the mock target filesystem image (${D})
# under the standard binary directory (${bindir} = /usr/bin).
do_install() {
    install -d ${D}${bindir}
    install -m 0755 sensor-simulator ${D}${bindir}/sensor-simulator
}
