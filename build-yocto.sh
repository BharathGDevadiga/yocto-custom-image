#!/bin/bash
# =============================================================
#  Yocto Build Script - Run this inside WSL Ubuntu
#  Usage: bash /mnt/c/Users/devad/Downloads/yocto-custom-image/build-yocto.sh
# =============================================================
set -e

echo "============================================"
echo "  Step 1: Installing Yocto Dependencies"
echo "============================================"
sudo apt-get update
sudo apt-get install -y gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect xz-utils \
    debianutils iputils-ping python3-git python3-jinja2 python3-subunit \
    zstd liblz4-tool file locales libacl1
sudo locale-gen en_US.UTF-8

echo ""
echo "============================================"
echo "  Step 2: Cloning Poky (scarthgap LTS)"
echo "============================================"
cd ~
if [ -d "poky" ]; then
    echo "[=] ~/poky already exists, skipping clone"
else
    git clone -b scarthgap https://github.com/yoctoproject/poky.git
fi
cd ~/poky

echo ""
echo "============================================"
echo "  Step 3: Copying meta-bharath layer"
echo "============================================"
cp -r /mnt/c/Users/devad/Downloads/yocto-custom-image/meta-bharath ~/poky/meta-bharath
echo "[+] meta-bharath copied to ~/poky/meta-bharath"

echo ""
echo "============================================"
echo "  Step 4: Initializing build environment"
echo "============================================"
source oe-init-build-env build

echo ""
echo "============================================"
echo "  Step 5: Adding meta-bharath layer"
echo "============================================"
bitbake-layers add-layer ~/poky/meta-bharath
bitbake-layers show-layers

echo ""
echo "============================================"
echo "  Step 6: Configuring MACHINE"
echo "============================================"
if ! grep -q '^MACHINE = "qemux86-64"' conf/local.conf; then
    echo '' >> conf/local.conf
    echo '# Build for QEMU x86-64' >> conf/local.conf
    echo 'MACHINE = "qemux86-64"' >> conf/local.conf
    echo "[+] MACHINE set to qemux86-64"
else
    echo "[=] MACHINE already set"
fi

echo ""
echo "============================================"
echo "  Step 7: Starting BitBake build"
echo "  This will take 2-4 hours on first run!"
echo "============================================"
echo ""
echo "Building bharath-image..."
bitbake bharath-image

echo ""
echo "============================================"
echo "  BUILD COMPLETE!"
echo ""
echo "  Boot with:"
echo "    runqemu qemux86-64 nographic"
echo ""
echo "  Then test:"
echo "    sensor-simulator -n 3"
echo "    tcp-server 5000 &"
echo "    tcp-client 127.0.0.1 5000"
echo "    udp-server 5001 &"
echo "    udp-client 127.0.0.1 5001"
echo "============================================"
