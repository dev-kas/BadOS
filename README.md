# BadOS 

**An x86_64 operating system built entirely from scratch with exactly one purpose: to play the "Bad Apple!!" music video.**

BadOS (based on the [meat kernel](https://github.com/dev-kas/meat)) is an operating system featuring a custom monolithic kernel, a custom executable format (`KEX`), a tar-based ramdisk, and Ring 3 userland support. It was built to be as lightweight as possible, successfully booting and playing smooth video on just **31MB of RAM**.

## Features

* **x86_64 Architecture:** Boots via the Limine bootloader (supports both BIOS and UEFI).
* **Ring 3 Userland:** Fully functional userland environment with system calls (`int 0x80`).
* **Multitasking:** Preemptive multitasking and scheduling via PIT (IRQ0).
* **Custom Executable Format (KEX):** Replaces standard ELF files for userland. Includes a custom linker (`kex-ld`).
* **Custom Libc:** A lightweight standard library implementation (`printf`, etc.).
* **Ultra-Low Memory Footprint:** Thanks to Run-Length Encoding (RLE) and on-the-fly decoding, the entire OS runs on just 31MB of RAM.

## How it Works

1. **Video Conversion (`misc/index.js`):** A Node.js script uses FFmpeg to extract frames from `Bad Apple!!.mp4`. It converts the frames to black and white (1bpp) and compresses them using Run-Length Encoding (RLE). The result is packed into a custom `.bad` file format alongside a header containing resolution and frame count.
2. **Booting & Ramdisk:** Limine boots the OS and loads a Tar archive as a ramdisk. The kernel initializes memory (PMM/VMM), sets up the GDT/IDT, and mounts the ramdisk.
3. **The `KEX` Loader:** The kernel finds `init.kex` (the userland video player) in the ramdisk, maps it into memory, and drops into Ring 3 user mode.
4. **Playback:** The `init` program reads `video.bad` directly from the kernel-mapped memory. It decodes the RLE stream on-the-fly into a fixed-size frame buffer and issues a `draw_frame` system call to render it to the Limine framebuffer. The PIT timer keeps playback synced to 30 FPS.

## Building & Running

### Prerequisites

You will need a standard OS development environment:
* `x86_64-elf-gcc` cross-compiler suite (`gcc`, `as`, `ar`)
* `make` and `7z` (p7zip)
* `xorriso` (for building the ISO)
* `qemu-system-x86_64` (for testing)
* `nodejs` and `npm` (for the video converter)
* `g++` and OpenSSL headers (`libssl-dev`) to compile the custom `kex-ld` linker

### Step 1: Prepare the Video

Place a copy of the Bad Apple music video named `Bad Apple!!.mp4` inside the `misc/` directory.

```bash
cd misc
npm install
node index.js
```
This will generate `video.bad`. Move this file into the `ramdisk/` directory at the root of the project.

### Step 2: Compile the `kex-ld` Linker
The userland Makefile expects `kex-ld` to be available.
```bash
cd kex-ld
g++ kex-ld.cpp -o kex-ld -lssl -lcrypto
# Note: Ensure the path to this binary matches KEXLD in userland/Makefile
```

### Step 3: Build the OS
Return to the root directory and run the ISO build script. This will compile the libc, kernel, and userland, pack the ramdisk, and generate `bados.iso`.

```bash
./iso.sh
```

### Step 4: Run in QEMU
```bash
./qemu.sh
```

## Tested Environments

BadOS has been heavily optimized for both emulators and real hardware (bare-metal). It has been successfully tested on:

* **QEMU** (`qemu-system-x86_64` v10.2.0) on Arch WSL (Windows) - *31MB RAM*
* **VirtualBox** (v7.2.0) on Windows - *31MB RAM*
* **VMware Workstation Pro** (25h2) on Windows 11 - *32MB RAM*
* **Bare-metal** on AMD Ryzen 7 7800X3D - *32GB RAM*

## Version History

* **v1.0.3 (Latest):** 
  * Massive memory reduction (down to 31MB minimum RAM).
  * Replaced raw 1bpp frame storage with compact RLE encoding.
  * Implemented on-the-fly RLE decoding in userland into a fixed-size frame buffer.
* **v1.0.2:** 
  * Reduced RAM requirement to 154MB.
  * Optimized boot sequence (direct mapped physical pages for `video.bad`).
  * Fixed PMM bitmap type mismatch (crucial for bare-metal stability).
* **v1.0.1:** 
  * Reduced PIT frequency from 1000Hz to 100Hz to fix slow bare-metal playback.
  * Implemented back-buffering and dirty-drawing for smooth rendering.
  * Added video looping.
* **v1.0.0:** 
  * Initial stable release. Transitioned to x86\_64 with graphics, implemented KEX userland, multitasking, and Limine bootloader.

## License

This project is licensed under the MIT License. See the video converter's `package.json` for details. *(Note: The "Bad Apple!!" song and animation belong to their respective original creators).*

> [!NOTE]
> Portions of this `README.md` were generated with help from a LLM, but I have manually verified most of the info for accuracy.
