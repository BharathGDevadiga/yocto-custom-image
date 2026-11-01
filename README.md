# Yocto Custom Image with meta-bharath Layer

> **Engineered a custom Yocto (Poky Scarthgap 5.0) Linux image for QEMU x86-64 featuring an out-of-tree Linux 6.6 Kernel Character Driver (`/dev/bharath_sensor`), a user-space telemetry daemon with Fletcher-16 checksum integrity, and a POSIX TCP/UDP socket networking suite.**

A complete, production-grade Yocto Project build creating an embedded Linux distribution for **QEMU x86-64**, powered by the custom BSP layer **`meta-bharath`**:

1. **`sensor-kmod`** — An out-of-tree Linux 6.6 character device driver registering `/dev/bharath_sensor` with dynamic major allocation, `class_create` (single-argument Linux 6.4+ API), and `eudev` auto-node generation streaming 64-bit jiffies timestamps, temperature (m°C), and atmospheric pressure (Pa).
2. **`sensor-simulator`** — An embedded user-space C daemon reading binary telemetry directly from `/dev/bharath_sensor` and verifying frame integrity via Fletcher-16 checksum calculations.
3. **`netcomm`** — POSIX TCP and UDP client-server networking suite validating kernel network stack and IPC functionality.
4. **`bharath-image`** — Tailored embedded Linux rootfs image with dropbear SSH, networking utilities, and automated module autoloading.

---

## Project Structure

```text
yocto-custom-image/
├── README.md
├── setup-build.sh              # Automated build environment setup
├── build-yocto.sh
├── .gitignore
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

## Prerequisites

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

## Build Instructions

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
crw-rw-rw-    1 root     root      246,   0 Nov  1 12:28 /dev/bharath_sensor
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

[   51.576841] sensor_kmod: Device /dev/bharath_sensor opened
[2026-11-01 12:29:26] [JIFFIES=4294718253] KERNEL_SENSOR: TEMP=31.29°C (31294 mC) | PRESS=1039.65 hPa (103965 Pa) | CHECKSUM=0x2ECA (VALID)
[2026-11-01 12:29:28] [JIFFIES=4294720274] KERNEL_SENSOR: TEMP=33.26°C (33258 mC) | PRESS=981.91 hPa (98191 Pa) | CHECKSUM=0x10C6 (VALID)
[2026-11-01 12:29:30] [JIFFIES=4294722279] KERNEL_SENSOR: TEMP=21.07°C (21073 mC) | PRESS=1030.67 hPa (103067 Pa) | CHECKSUM=0x29F9 (VALID)
[2026-11-01 12:29:32] [JIFFIES=4294724283] KERNEL_SENSOR: TEMP=24.26°C (24256 mC) | PRESS=1039.59 hPa (103959 Pa) | CHECKSUM=0xA8D0 (VALID)
[2026-11-01 12:29:34] [JIFFIES=4294726295] KERNEL_SENSOR: TEMP=21.17°C (21174 mC) | PRESS=1034.47 hPa (103447 Pa) | CHECKSUM=0x349C (VALID)
[   59.637229] sensor_kmod: Device /dev/bharath_sensor closed

=== Sensor Reader Stopped (5 readings completed) ===
```

### 4. Test POSIX TCP Networking
```bash
tcp-server 5000 &
tcp-client 127.0.0.1 5000
```
**Output:**
```text
[TCP-SERVER] Listening on port 5000...
[TCP-CLIENT] Connected!
[TCP-CLIENT] Sending: Hello from Yocto TCP client!
[TCP-SERVER] Received: Hello from Yocto TCP client!
[TCP-CLIENT] Server replied: [ECHO] Hello from Yocto TCP client!
```

### 5. Test POSIX UDP Networking
```bash
udp-server 5001 &
udp-client 127.0.0.1 5001
```
**Output:**
```text
[UDP-SERVER] Listening on port 5001...
[UDP-CLIENT] Sending to 127.0.0.1:5001
[UDP-SERVER] From 127.0.0.1:38921 => Hello from Yocto UDP client!
[UDP-CLIENT] Server replied: [UDP-ECHO] Hello from Yocto UDP client!
```

### Exit QEMU
Press **`Ctrl + A`**, release, then press **`x`**.

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
