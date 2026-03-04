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
    console.log(`Converting ${this.frames.length} frames into .bad format...`);

    const header = Buffer.alloc(8);
    header.write("BAD\0", 0, "ascii");
    header.writeUInt16LE(this.width, 4);
    header.writeUInt16LE(this.height, 6);

    const pixelsPerFrame = this.width * this.height;
    const bytesPerFrame = Math.ceil(pixelsPerFrame / 8);

    const payload = Buffer.alloc(this.frames.length * bytesPerFrame);

    for (let f = 0; f < this.frames.length; f++) {
      const frameBuffer = this.frames[f];
      const frameStartByte = f * bytesPerFrame;

      for (let i = 0; i < pixelsPerFrame; i++) {
        const x = i % this.width;
        const y = Math.floor(i / this.width);

        const offset = (y * this.width + x) * 3;
        const r = frameBuffer[offset];
        const g = frameBuffer[offset + 1];
        const b = frameBuffer[offset + 2];

        const brightness = (r + g + b) / 3;
        const isBright = brightness >= 128;

        if (isBright) {
          const byteOffset = frameStartByte + Math.floor(i / 8);
          const bitOffset = i % 8;
          payload[byteOffset] |= 1 << bitOffset;
        }
      }

      if (f % 100 === 0) console.log(`Processed frame ${f}...`);
    }

    const finalBuffer = Buffer.concat([header, payload]);
    fs.writeFileSync(outputFileName, finalBuffer);

    console.log(`Success! ${outputFileName} created.`);
    console.log(
      `Original size: ~${Math.round((this.frames.length * this.width * this.height * 3) / 1024 / 1024)} MB`,
    );
    console.log(`Converted size: ${Math.round(finalBuffer.length / 1024)} KB`);
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
