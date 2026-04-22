const { spawn, spawnSync } = require("node:child_process");
const ffprobe = require("ffprobe-static").path;
const ffmpeg = require("ffmpeg-static");
const fs = require("node:fs");

class Frame {
  constructor(buffer, width, height) {
    this.buffer = buffer;
    this.width = width;
    this.height = height;
  }

  pixel(x, y) {
    const offset = (y * this.width + x) * 3;
    return {
      r: this.buffer[offset],
      g: this.buffer[offset + 1],
      b: this.buffer[offset + 2],
    };
  }
}

class Converter {
  constructor() {
    this.frames = [];
    this.inputPath = null;
    this.width = 0;
    this.height = 0;
  }

  async getDims() {
    const res = spawnSync(ffprobe, [
      "-v",
      "error",
      "-select_streams",
      "v:0",
      "-show_entries",
      "stream=width,height",
      "-of",
      "json",
      this.inputPath,
    ]);
    const json = JSON.parse(res.stdout.toString());
    return { width: json.streams[0].width, height: json.streams[0].height };
  }

  read(width, height) {
    const frameSize = width * height * 3;
    return new Promise((resolve, reject) => {
      const ff = spawn(ffmpeg, [
        "-i",
        this.inputPath,
        "-f",
        "rawvideo",
        "-pix_fmt",
        "rgb24",
        "-",
      ]);

      let buffer = Buffer.alloc(0);
      ff.stdout.on("data", (chunk) => {
        buffer = Buffer.concat([buffer, chunk]);
        while (buffer.length >= frameSize) {
          this.frames.push(buffer.slice(0, frameSize));
          buffer = buffer.slice(frameSize);
        }
      });

      ff.on("close", (code) => (code === 0 ? resolve() : reject()));
    });
  }

  async load(inputPath) {
    this.inputPath = inputPath;
    const { width, height } = await this.getDims();
    this.width = width;
    this.height = height;
    console.log(`Dimensions: ${width}x${height}`);
    await this.read(this.width, this.height);
  }

  process(outputFileName = "BadApple.bad") {
    console.log(
      `Converting ${this.frames.length} frames into .bad format...`,
    );

    const pixelsPerFrame = this.width * this.height;
    const rleBuffer = Buffer.allocUnsafe(this.frames.length * pixelsPerFrame);
    let rleIndex = 0;

    let currentColor = -1;
    let runLength = 0;

    for (let f = 0; f < this.frames.length; f++) {
      const frameBuffer = this.frames[f];

      for (let i = 0; i < pixelsPerFrame; i++) {
        const pixelOffset = i * 3;
        const r = frameBuffer[pixelOffset];
        const g = frameBuffer[pixelOffset + 1];
        const b = frameBuffer[pixelOffset + 2];

        const isBright = (r + g + b) / 3 >= 128 ? 1 : 0;

        if (currentColor === -1) {
          currentColor = isBright;
          runLength = 1;
        } else if (currentColor === isBright && runLength < 127) {
          runLength++;
        } else {
          rleBuffer[rleIndex++] = (currentColor << 7) | runLength;
          currentColor = isBright;
          runLength = 1;
        }
      }

      if (f % 100 === 0) console.log(`Processed frame ${f}...`);
    }

    if (runLength > 0) {
      rleBuffer[rleIndex++] = (currentColor << 7) | runLength;
    }

    const headerSize = 12;
    const finalSize = headerSize + rleIndex;
    const buffer = Buffer.alloc(finalSize);

    // magic
    buffer.write("BAD\0", 0, "ascii");

    // dimensions
    buffer.writeUInt16LE(this.width, 4);
    buffer.writeUInt16LE(this.height, 6);

    // frame count
    buffer.writeUInt32LE(this.frames.length, 8);

    rleBuffer.copy(buffer, headerSize, 0, rleIndex);

    fs.writeFileSync(outputFileName, buffer);
    console.log(`Success! ${outputFileName} created.`);
    console.log(
      `Original size: ~${Math.round((this.frames.length * pixelsPerFrame * 3) / 1024 / 1024)} MB`,
    );
    console.log(`Converted size: ${Math.round(finalSize / 1024)} KB`);
  }
}

(async () => {
  const conv = new Converter();
  try {
    await conv.load("Bad Apple!!.mp4");
    conv.process("video.bad");
  } catch (e) {
    console.error(
      "Failed to process video. Check if the file exists and ffmpeg is installed.",
    );
    console.error(e);
  }
})();
