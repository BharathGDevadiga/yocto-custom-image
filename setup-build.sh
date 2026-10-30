#!/bin/bash
# setup-build.sh
#
# One-shot script to set up the Yocto build environment.
# Run this from the directory where you cloned Poky.
#
# Usage:
#   cd /path/to/poky
#   cp -r /path/to/yocto-custom-image/meta-bharath .
#   source ./setup-build.sh
#
# Or if you've already sourced oe-init-build-env:
#   bash ./setup-build.sh   (just adds the layer)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
POKY_DIR="${SCRIPT_DIR}"

echo "============================================"
echo "  Yocto Build Setup - meta-bharath"
echo "============================================"

# Source the OE build environment if not already done
if [ -z "$BUILDDIR" ]; then
    echo "[*] Sourcing oe-init-build-env..."
    source "${POKY_DIR}/oe-init-build-env" build
else
    echo "[*] Build environment already set up: $BUILDDIR"
fi

# Add meta-bharath layer if not already present
if ! bitbake-layers show-layers 2>/dev/null | grep -q "meta-bharath"; then
    echo "[*] Adding meta-bharath layer..."
    bitbake-layers add-layer "${POKY_DIR}/meta-bharath"
    echo "[+] meta-bharath added successfully."
else
    echo "[=] meta-bharath already in bblayers.conf"
fi

# Set MACHINE to qemux86-64 if not already configured
if grep -q '^MACHINE ??= "qemux86-64"' "$BUILDDIR/conf/local.conf" 2>/dev/null || \
   grep -q '^MACHINE = "qemux86-64"' "$BUILDDIR/conf/local.conf" 2>/dev/null; then
    echo "[=] MACHINE already set to qemux86-64"
else
    echo "[*] Setting MACHINE to qemux86-64..."
    echo '' >> "$BUILDDIR/conf/local.conf"
    echo '# Custom: build for QEMU x86-64' >> "$BUILDDIR/conf/local.conf"
    echo 'MACHINE = "qemux86-64"' >> "$BUILDDIR/conf/local.conf"
fi

echo ""
echo "============================================"
echo "  Setup complete!"
echo ""
echo "  To build the custom image:"
echo "    bitbake bharath-image"
echo ""
echo "  To build just core-image-minimal:"
echo "    bitbake core-image-minimal"
echo ""
echo "  To boot in QEMU after build:"
echo "    runqemu qemux86-64 nographic"
echo "============================================"
