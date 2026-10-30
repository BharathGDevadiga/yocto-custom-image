SUMMARY = "TCP/UDP client-server networking applications"
DESCRIPTION = "A set of four C programs: TCP echo server, TCP client, \
UDP echo server, and UDP client. Demonstrates socket programming and \
network communication in an embedded Linux environment."
HOMEPAGE = "https://github.com/bharath/yocto-custom-image"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://tcp-server.c \
           file://tcp-client.c \
           file://udp-server.c \
           file://udp-client.c \
           file://Makefile"

S = "${WORKDIR}"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 tcp-server ${D}${bindir}/tcp-server
    install -m 0755 tcp-client ${D}${bindir}/tcp-client
    install -m 0755 udp-server ${D}${bindir}/udp-server
    install -m 0755 udp-client ${D}${bindir}/udp-client
}
