#!/usr/bin/env bash
#
# run-prebuilt.sh
#
# Boots the pre-compiled Bharath Embedded Linux image in standalone QEMU
# without requiring Yocto or BitBake installed.
#

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARTIFACTS_DIR="${DIR}/release-artifacts"

KERNEL="${ARTIFACTS_DIR}/bzImage"
ROOTFS="${ARTIFACTS_DIR}/bharath-image-qemux86-64.rootfs.ext4"

# Check if QEMU is installed
if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Error: qemu-system-x86_64 is not installed."
    echo "Install with: sudo apt update && sudo apt install -y qemu-system-x86"
    exit 1
fi

# Fallback check if user placed files in current directory
if [ ! -f "${KERNEL}" ]; then
    KERNEL="${DIR}/bzImage"
fi
if [ ! -f "${ROOTFS}" ]; then
    ROOTFS="${DIR}/bharath-image-qemux86-64.rootfs.ext4"
fi

if [ ! -f "${KERNEL}" ] || [ ! -f "${ROOTFS}" ]; then
    echo "Error: Release artifacts not found."
    echo "Please ensure bzImage and bharath-image-qemux86-64.rootfs.ext4 are present in:"
    echo "  ${ARTIFACTS_DIR}/ or ${DIR}/"
    echo "Download them from the GitHub Releases page: https://github.com/BharathGDevadiga/yocto-custom-image/releases"
    exit 1
fi

echo "=========================================================="
echo " Booting Bharath Custom Embedded Linux in QEMU"
echo " Kernel : ${KERNEL}"
echo " RootFS : ${ROOTFS}"
echo " To exit: type 'poweroff' inside the QEMU shell"
echo "=========================================================="

qemu-system-x86_64 \
    -kernel "${KERNEL}" \
    -drive file="${ROOTFS}",if=virtio,format=raw \
    -append "root=/dev/vda rw console=ttyS0 quiet" \
    -cpu qemu64 \
    -machine q35 \
    -smp 2 \
    -m 256 \
    -nographic
