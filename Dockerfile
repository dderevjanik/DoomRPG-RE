# Dockerfile for building DoomRPG-RE
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install base dependencies and tools for building CMake
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    pkg-config \
    libsdl2-dev \
    libsdl2-mixer-dev \
    zlib1g-dev \
    libfluidsynth-dev \
    wget \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install CMake 3.25+ from source (Ubuntu 22.04 only has 3.22.1)
ARG CMAKE_VERSION=3.28.3
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-aarch64.tar.gz && \
    tar -xzf cmake-${CMAKE_VERSION}-linux-aarch64.tar.gz -C /opt && \
    rm cmake-${CMAKE_VERSION}-linux-aarch64.tar.gz && \
    ln -s /opt/cmake-${CMAKE_VERSION}-linux-aarch64/bin/cmake /usr/local/bin/cmake && \
    ln -s /opt/cmake-${CMAKE_VERSION}-linux-aarch64/bin/ctest /usr/local/bin/ctest && \
    ln -s /opt/cmake-${CMAKE_VERSION}-linux-aarch64/bin/cpack /usr/local/bin/cpack

# Set working directory
WORKDIR /app

# Copy project files
COPY . /app

# Create build directory
RUN mkdir -p build

# Configure with CMake
WORKDIR /app/build
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release

# Build the project
RUN cmake --build . --config Release -j$(nproc)

# The binary will be in /app/build/src/DoomRPG
# Set the working directory back to build for running
WORKDIR /app/build

# Default command - list the built files
CMD ["sh", "-c", "echo 'DoomRPG build complete!' && ls -lah /app/build/src/ && echo '\nTo run DoomRPG, execute: ./src/DoomRPG'"]
