#!/bin/bash
KERNEL_VERSION=$(uname -r)
MODULE_NAME="immortal_proc.ko"

# Sign the module
/usr/src/linux-headers-${KERNEL_VERSION}/scripts/sign-file \
  sha256 \
  keys/immortal.priv \
  keys/immortal.der \
  src/${MODULE_NAME}

echo "Module signed successfully"
