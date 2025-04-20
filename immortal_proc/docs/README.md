# Immortal Process Linux Kernel Module

A Linux kernel module for preventing process termination, even from SIGKILL.

## Features

- Protect critical processes from all signals, including SIGKILL
- Simple management via /proc interface
- Compatible with Secure Boot environments
- Works across major Linux distributions

## Installation

### Building from source

Requirements:
- Linux kernel headers
- Build tools (gcc, make)
- OpenSSL for module signing

```bash
# Clone repository
git clone https://github.com/yourusername/immortal_proc.git
cd immortal_proc

# Build module
make

# Sign module (for Secure Boot)
make sign

# Install module
sudo make install
