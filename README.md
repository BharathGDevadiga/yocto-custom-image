# Yocto Custom Image with meta-bharath Layer

> **Engineered a custom Yocto (Poky Scarthgap 5.0) Linux image for QEMU x86-64 featuring an out-of-tree Linux 6.6 Kernel Character Driver (`/dev/bharath_sensor`), a user-space telemetry daemon with Fletcher-16 checksum integrity, and a POSIX TCP/UDP socket networking suite.**

A complete, production-grade Yocto Project build creating an embedded Linux distribution for **QEMU x86-64**, powered by the custom BSP layer **`meta-bharath`**:

1. **`sensor-kmod`** — An out-of-tree Linux 6.6 character device driver registering `/dev/bharath_sensor` with dynamic major allocation, `class_create` (single-argument Linux 6.4+ API), and `eudev` auto-node generation streaming 64-bit jiffies timestamps, temperature (m°C), and atmospheric pressure (Pa).
2. **`sensor-simulator`** — An embedded user-space C daemon reading binary telemetry directly from `/dev/bharath_sensor` and verifying frame integrity via Fletcher-16 checksum calculations.
3. **`netcomm`** — POSIX TCP and UDP client-server networking suite validating kernel network stack and IPC functionality.
4. **`bharath-image`** — Tailored embedded Linux rootfs image with dropbear SSH, networking utilities, and automated module autoloading.

---

## ⚡ Quick Run (Instant Play without 60 GB Yocto Build)

If you just want to test this custom operating system without compiling from source, you do **not** need to install Yocto or download 60 GB of Poky source code!

You can run the pre-built Linux kernel (`bzImage`) and root filesystem (`bharath-image-qemux86-64.rootfs.ext4`) directly using standalone QEMU in under two minutes:

### 1. Download Release Binaries (~109 MB)
Download `bzImage` and `bharath-image-qemux86-64.rootfs.ext4` from the **[GitHub Releases (v1.0.0)](https://github.com/BharathGDevadiga/yocto-custom-image/releases/tag/v1.0.0)** page.

### 2. Boot in QEMU with a Single Command

#### On Linux or WSL:
```bash
# Automated helper script
chmod +x run-prebuilt.sh
./run-prebuilt.sh
```

#### On Windows:
Double-click **`run-prebuilt-windows.bat`** in File Explorer (or run it from PowerShell/CMD).

#### Manual Direct QEMU Invocation:
```bash
qemu-system-x86_64 \
    -kernel bzImage \
    -drive file=bharath-image-qemux86-64.rootfs.ext4,if=virtio,format=raw \
    -append "root=/dev/vda rw console=ttyS0 console=ttyS1 quiet" \
    -cpu IvyBridge,-x2apic,-tsc-deadline \
    -machine q35,i8042=off \
    -smp 4 \
    -m 256 \
    -serial mon:stdio \
    -serial null \
    -nographic
```

### 3. Log In & Verify
* When prompted, log in with username: **`root`** (no password required).
* Verify driver: `lsmod | grep sensor_kmod`
* Verify device node: `ls -l /dev/bharath_sensor`
* Run telemetry client: `sensor-simulator -n 5`
* **Clean Exit:** Simply type **`poweroff`**!

---

## Project Structure

```text
yocto-custom-image/
├── README.md
├── run-prebuilt.sh             # One-click standalone QEMU launcher
├── setup-build.sh              # Automated build environment setup
├── build-yocto.sh
├── .gitignore
├── Yocto_Embedded_Linux_Project_Guide.pdf  # Comprehensive architecture & walkthrough guide
└── meta-bharath/               # Custom Yocto layer
    ├── conf/
    │   └── layer.conf          # Layer configuration (scarthgap compatible)
    ├── recipes-apps/
    │   ├── sensor-simulator/
    │   │   ├── sensor-simulator_1.0.bb    # User-space BitBake recipe
    │   │   └── files/
    │   │       └── sensor-simulator.c     # Kernel character device reader & Fletcher-16
    │   └── netcomm/
    │       ├── netcomm_1.0.bb             # Network suite BitBake recipe
    │       └── files/
    │           ├── tcp-server.c           # POSIX TCP echo server
    │           ├── tcp-client.c           # POSIX TCP client
    │           ├── udp-server.c           # POSIX UDP echo server
    │           ├── udp-client.c           # POSIX UDP client
    │           └── Makefile               # Multi-binary Makefile
    ├── recipes-kernel/
    │   └── sensor-kmod/
    │       ├── sensor-kmod_1.0.bb         # Kernel module recipe (inherits module)
    │       └── files/
    │           ├── sensor_kmod.c          # Linux 6.6 character driver (/dev/bharath_sensor)
    │           └── Makefile               # Kbuild module Makefile
    └── recipes-core/
        └── images/
            └── bharath-image.bb           # Custom image recipe
```

---

## Prerequisites (For Building from Source)

### System Requirements
- **OS:** Ubuntu 22.04 or 24.04 (WSL2 or Native)
- **Disk:** ~100 GB free space
- **RAM:** 8 GB minimum (16 GB recommended)
- **Target:** `qemux86-64`

### Host Dependencies (Ubuntu)
```bash
sudo apt update
sudo apt install -y gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect xz-utils \
    debianutils iputils-ping python3-git python3-jinja2 python3-subunit \
    zstd liblz4-tool file locales libacl1
sudo locale-gen en_US.UTF-8
```

---

## Full Build Instructions from Source

### Step 1: Clone Poky (Yocto Reference Distribution)
```bash
cd ~
git clone -b scarthgap https://github.com/yoctoproject/poky.git
cd poky
```

### Step 2: Add `meta-bharath` Layer
```bash
# Clone this repository
git clone https://github.com/BharathGDevadiga/yocto-custom-image.git ~/yocto-custom-image
cp -r ~/yocto-custom-image/meta-bharath ~/poky/meta-bharath

# Initialize BitBake build environment
source oe-init-build-env build

# Register meta-bharath into bblayers.conf
bitbake-layers add-layer ../meta-bharath
```

### Step 3: Configure `conf/local.conf`
Ensure the following settings are present in `conf/local.conf`:
```bitbake
MACHINE = "qemux86-64"
BB_NUMBER_THREADS = "6"
PARALLEL_MAKE = "-j 6"
INHERIT += "rm_work"
```

### Step 4: Build the Image
```bash
bitbake bharath-image
```
*BitBake will execute all ~6,050 tasks, cross-compiling the Linux 6.6 kernel, building the `sensor_kmod.ko` module, compiling user-space C packages, and producing `bharath-image-qemux86-64.rootfs.ext4`.*

---

## Booting and Verification in QEMU

Run the headless virtual machine directly from the build directory:
```bash
runqemu nographic
```

Log in when prompted:
```text
qemux86-64 login: root
```
*(No password needed)*

### 1. Verify Kernel Module Autoloading
```bash
lsmod | grep sensor_kmod
```
**Output:**
```text
sensor_kmod            12288  0
```

### 2. Verify Character Device Node Creation
```bash
ls -l /dev/bharath_sensor
```
**Output:**
```text
crw-rw-rw-    1 root     root      246,   0 Nov  1 13:26 /dev/bharath_sensor
```

### 3. Test Kernel-to-User Telemetry with Fletcher-16 Checksum
```bash
sensor-simulator -n 5
```
**Verified Output:**
```text
=== Bharath Kernel Sensor Reader ===
Connecting to Character Device: /dev/bharath_sensor
Interval: 2 sec | Readings: limited

[  703.469910] sensor_kmod: Device /dev/bharath_sensor opened
[2026-11-01 13:21:06] [JIFFIES=4295370204] KERNEL_SENSOR: TEMP=28.79°C (28789 mC) | PRESS=1004.17 hPa (100417 Pa) | CHECKSUM=0xBAB9 (VALID)
[2026-11-01 13:21:08] [JIFFIES=4295372222] KERNEL_SENSOR: TEMP=28.26°C (28261 mC) | PRESS=1025.39 hPa (102539 Pa) | CHECKSUM=0x04E3 (VALID)
[2026-11-01 13:21:10] [JIFFIES=4295374227] KERNEL_SENSOR: TEMP=32.67°C (32668 mC) | PRESS=1027.68 hPa (102768 Pa) | CHECKSUM=0x91EE (VALID)
[2026-11-01 13:21:12] [JIFFIES=4295376233] KERNEL_SENSOR: TEMP=23.88°C (23880 mC) | PRESS=1039.14 hPa (103914 Pa) | CHECKSUM=0xCBD4 (VALID)
[2026-11-01 13:21:14] [JIFFIES=4295378249] KERNEL_SENSOR: TEMP=29.52°C (29517 mC) | PRESS=1000.09 hPa (100009 Pa) | CHECKSUM=0xD287 (VALID)
[  711.534301] sensor_kmod: Device /dev/bharath_sensor closed

=== Sensor Reader Stopped (5 readings completed) ===
```

### 4. Clean Shutdown Command
```bash
poweroff
```

---

## Technical Architecture & Implementation

### 1. `sensor_kmod` (Linux 6.6 Kernel Driver)
* **Subsystem:** Linux Character Device Subsystem.
* **Compatibility:** Linux 6.6 / modern kernel APIs (`class_create(name)` single-argument form introduced in Linux 6.4).
* **Data Transmission:** When user space reads `/dev/bharath_sensor`, the driver allocates a packed binary struct (`bharath_sensor_data`), samples `get_jiffies_64()`, draws hardware entropy with `get_random_bytes()`, and performs safe atomic user memory transfer with `copy_to_user()`.
* **Autoloading:** Packaged with `KERNEL_MODULE_AUTOLOAD += "sensor_kmod"` and `RPROVIDES:${PN} += "kernel-module-sensor-kmod"` so systemd/eudev automatically loads the `.ko` during early boot.

### 2. `sensor-simulator` (User-Space Telemetry Client)
* **File Operations:** Directly interfaces with the character device driver via `open("/dev/bharath_sensor", O_RDONLY)` and low-level POSIX `read()`.
* **Checksum Integrity:** Computes a 16-bit Fletcher checksum across the packed binary struct payload to emulate high-reliability aerospace/automotive telemetry pipelines.

### 3. `netcomm` (POSIX Sockets Suite)
* Demonstrates non-blocking and stream socket communication using low-level `sys/socket.h`, `netinet/in.h`, and `arpa/inet.h`.

---

## Resume Bullet Point

> *"Architected a custom embedded Linux distribution using the Yocto Project (Poky Scarthgap 5.0) targeting QEMU x86-64; developed an out-of-tree Linux 6.6 kernel character device driver (`/dev/bharath_sensor`) with dynamic node generation and automated module autoloading; authored C user-space telemetry services with Fletcher-16 integrity validation and a POSIX TCP/UDP socket networking suite in the custom `meta-bharath` layer."*

---

## License

MIT / GPL-2.0-only

---

## Author

**Bharath G Devadiga**  
*Embedded Linux, Firmware & Systems Engineering*  
GitHub: [@BharathGDevadiga](https://github.com/BharathGDevadiga)
