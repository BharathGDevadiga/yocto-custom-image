# Yocto Custom Image with meta-bharath Layer

> **Built a custom Yocto (Poky) Linux image for QEMU with a custom layer; packaged a C application and UDP/TCP client-server as BitBake recipes.**

A complete Yocto Project build that creates a custom Linux image for **QEMU x86-64** (or aarch64), featuring a custom BSP layer (`meta-bharath`) with two BitBake recipes:

1. **sensor-simulator** — A C program simulating a UART-connected temperature/humidity sensor
2. **netcomm** — TCP and UDP client-server echo applications demonstrating socket programming

---

## Project Structure

```
yocto-custom-image/
├── README.md
├── setup-build.sh              # Automated build environment setup
├── .gitignore
└── meta-bharath/               # Custom Yocto layer
    ├── conf/
    │   └── layer.conf           # Layer configuration
    ├── recipes-apps/
    │   ├── sensor-simulator/
    │   │   ├── sensor-simulator_1.0.bb    # BitBake recipe
    │   │   └── files/
    │   │       └── sensor-simulator.c     # Sensor simulator source
    │   └── netcomm/
    │       ├── netcomm_1.0.bb             # BitBake recipe
    │       └── files/
    │           ├── tcp-server.c           # TCP echo server
    │           ├── tcp-client.c           # TCP client
    │           ├── udp-server.c           # UDP echo server
    │           ├── udp-client.c           # UDP client
    │           └── Makefile               # Build system for netcomm
    └── recipes-core/
        └── images/
            └── bharath-image.bb           # Custom image recipe
```

---

## Prerequisites

### System Requirements
- **OS:** Ubuntu 22.04 or 24.04 (native or WSL2)
- **Disk:** ~100 GB free space
- **RAM:** 8 GB minimum (16 GB recommended)
- **Time:** 2–4 hours for first build (subsequent builds are incremental)

### Install Required Packages (Ubuntu)

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
# Use the 'scarthgap' LTS release (Yocto 5.0)
cd ~
git clone -b scarthgap https://github.com/yoctoproject/poky.git
cd poky
```

### Step 2: Copy the Custom Layer

```bash
# Clone this repo (or copy meta-bharath into poky/)
git clone https://github.com/<your-username>/yocto-custom-image.git /tmp/yocto-custom-image
cp -r /tmp/yocto-custom-image/meta-bharath ~/poky/meta-bharath
cp /tmp/yocto-custom-image/setup-build.sh ~/poky/setup-build.sh
```

### Step 3: Initialize Build Environment

```bash
cd ~/poky
source oe-init-build-env build
```

This creates `~/poky/build/` and drops you into it.

### Step 4: Add the Custom Layer

```bash
bitbake-layers add-layer ../meta-bharath
```

Verify:
```bash
bitbake-layers show-layers
```

Expected output includes:
```
layer                 path                                      priority
==========================================================================
meta                  /home/user/poky/meta                      5
meta-poky             /home/user/poky/meta-poky                 5
meta-yocto-bsp        /home/user/poky/meta-yocto-bsp            5
meta-bharath          /home/user/poky/meta-bharath               6
```

### Step 5: Configure the Machine

Edit `conf/local.conf`:
```bash
# Set the target machine (already default in most Poky setups)
MACHINE = "qemux86-64"
```

Or for ARM:
```bash
MACHINE = "qemuarm64"
```

### Step 6: Build the Image

```bash
# Build the custom image (includes sensor-simulator + netcomm)
bitbake bharath-image
```

> ⏱️ **First build takes 2–4 hours.** Subsequent builds take minutes.

#### Or: Build Individual Recipes

```bash
bitbake sensor-simulator    # Build only the sensor simulator
bitbake netcomm              # Build only the network apps
bitbake core-image-minimal   # Build the base image without custom packages
```

---

## Running in QEMU

### Boot the Image

```bash
runqemu qemux86-64 nographic
```

For ARM:
```bash
runqemu qemuarm64 nographic
```

Login with `root` (no password).

### Test the Sensor Simulator

```bash
# Run with default settings (readings every 2 seconds)
sensor-simulator

# Run with custom interval and limited readings
sensor-simulator -i 1 -n 5
```

**Sample output:**
```
=== Sensor Simulator Started ===
Simulating UART sensor on /dev/ttyS0 (115200 8N1)
Interval: 2 sec | Readings: infinite

[2026-10-30 15:30:01] SENSOR_DATA: TEMP=28.45°C HUMID=52.30% CHECKSUM=0x3A
[2026-10-30 15:30:03] SENSOR_DATA: TEMP=31.12°C HUMID=47.85% CHECKSUM=0x1F
[2026-10-30 15:30:05] SENSOR_DATA: TEMP=26.78°C HUMID=61.20% CHECKSUM=0x5C
```

### Test TCP Client-Server

Open two terminals in QEMU (or use `&` to background):

**Terminal 1 — Start the server:**
```bash
tcp-server 5000 &
```

**Terminal 2 — Run the client:**
```bash
tcp-client 127.0.0.1 5000
```

**Expected output:**
```
[TCP-SERVER] Listening on port 5000...
[TCP-SERVER] Client connected: 127.0.0.1:49152
[TCP-CLIENT] Connecting to 127.0.0.1:5000...
[TCP-CLIENT] Connected!
[TCP-CLIENT] Sending: Hello from Yocto TCP client!
[TCP-SERVER] Received: Hello from Yocto TCP client!
[TCP-CLIENT] Server replied: [ECHO] Hello from Yocto TCP client!
[TCP-CLIENT] Sending: Embedded Linux networking test
[TCP-CLIENT] Server replied: [ECHO] Embedded Linux networking test
[TCP-CLIENT] Sending: Custom layer meta-bharath works!
[TCP-CLIENT] Server replied: [ECHO] Custom layer meta-bharath works!
[TCP-CLIENT] Done. Closing connection.
```

### Test UDP Client-Server

```bash
udp-server 5001 &
udp-client 127.0.0.1 5001
```

**Expected output:**
```
[UDP-SERVER] Listening on port 5001...
[UDP-CLIENT] Sending to 127.0.0.1:5001
[UDP-CLIENT] Sending: Hello from Yocto UDP client!
[UDP-SERVER] From 127.0.0.1:38921 => Hello from Yocto UDP client!
[UDP-CLIENT] Server replied: [UDP-ECHO] Hello from Yocto UDP client!
[UDP-CLIENT] Sending: Testing UDP datagram delivery
[UDP-CLIENT] Server replied: [UDP-ECHO] Testing UDP datagram delivery
[UDP-CLIENT] Done.
```

---

## Recipes Explained

### sensor-simulator_1.0.bb

| Field | Value |
|-------|-------|
| Source | Single C file, compiled directly with `${CC}` |
| Install | Binary placed in `/usr/bin/sensor-simulator` |
| Dependencies | None (uses only libc) |
| Build method | Direct `${CC}` invocation |

### netcomm_1.0.bb

| Field | Value |
|-------|-------|
| Source | 4 C files + Makefile |
| Install | 4 binaries in `/usr/bin/` |
| Dependencies | None (POSIX sockets) |
| Build method | `oe_runmake` (standard Makefile) |

### bharath-image.bb

Inherits `core-image` and adds:
- `sensor-simulator` and `netcomm` packages
- `ssh-server-dropbear` for remote shell access
- `iproute2` and `iputils-ping` for network debugging

---

## Skills Demonstrated

| JD Requirement | How This Project Covers It |
|----------------|---------------------------|
| **Embedded Linux / Yocto** | Full Yocto build from source with Poky |
| **Custom BSP Layers** | `meta-bharath` with `layer.conf`, proper priority |
| **BitBake Recipes** | Two recipes: direct compile + Makefile-based |
| **C Programming** | ~500 lines across 5 C source files |
| **Networking (TCP/UDP)** | Client-server echo apps using POSIX sockets |
| **Build Systems** | Makefile + BitBake integration via `oe_runmake` |
| **QEMU / Virtualization** | Image boots and runs in QEMU without hardware |
| **Cross-Compilation** | All recipes cross-compile via Yocto toolchain |

---

## Troubleshooting

### "layer.conf not found"
Make sure `meta-bharath` is at the same level as `meta`, `meta-poky`, etc. inside the Poky directory.

### Build fails with disk space error
You need ~100 GB free. Check with `df -h`.

### "do_fetch: no SRC_URI"
Ensure the `files/` directories contain all source files and the recipe `SRC_URI` paths match the filenames exactly.

### QEMU won't start
```bash
# Make sure you sourced the build environment first
source oe-init-build-env build
runqemu qemux86-64 nographic
```

---

## License

MIT

---

## Author

Bharath — *Embedded Linux & Yocto Project*
