# Doom RPG RE

## Building on macOS

1. Install Homebrew if you don't have it:
   ```sh
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```
2. Install dependencies:
   ```sh
   brew install cmake sdl2 sdl2_mixer fluid-synth pkg-config
   ```
3. Build the project:
   ```sh
   ./build-macos.sh build
   ```
   The compiled binary will be located at `build/src/DoomRPG`.

## Building with Docker

1. Build the Docker image:
   ```sh
   docker build -t doom-rpg-re .
   ```
2. Extract the compiled binary to your host machine:
   ```sh
   docker run --rm -v $(pwd)/output:/output doom-rpg-re sh -c "cp /app/build/src/DoomRPG /output/"
   ```
   The binary will be available in the `output` directory on your host.

## Building for Web (Emscripten)

1. Install Emscripten by following the instructions at https://emscripten.org/docs/getting_started/downloads.html. (or use brew)
2. Build the project for Web:
   ```sh
   ./build-web.sh
   http-server ./build-web/src/
   ```