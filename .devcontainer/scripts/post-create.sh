#!/usr/bin/env bash
#
# post-create.sh — runs inside the container after first create.
# Checks the toolchain, configures the QEMU build tree, installs editor configs.
#
set -euo pipefail

echo "Checking required tools in devcontainer..."
command -v meson >/dev/null
command -v ninja >/dev/null
command -v arm-none-eabi-gcc >/dev/null
command -v qemu-system-arm >/dev/null
command -v clangd >/dev/null

meson --version
ninja --version
arm-none-eabi-gcc --version | sed -n '1p'
qemu-system-arm --version | sed -n '1p'
clangd --version | sed -n '1p'

if [[ ! -d build ]]; then
	echo "No build directory found. Running initial configure for QEMU..."
	./configure build qemu
else
	echo "Build directory already exists. Skipping initial configure."
fi

# QEMU-only VS Code / Cursor debug + editor config (from .devcontainer/vscode/).
if [[ -d .devcontainer/vscode ]]; then
	mkdir -p .vscode
	cp .devcontainer/vscode/launch.json .vscode/launch.json
	cp .devcontainer/vscode/tasks.json .vscode/tasks.json
	cp .devcontainer/vscode/settings.json .vscode/settings.json
	cp .devcontainer/vscode/extensions.json .vscode/extensions.json
	echo "Installed QEMU debug and editor config in .vscode/"
fi

echo "Devcontainer setup completed."
echo "Tip: install recommended extensions if the IDE skipped them (Extensions view → @recommended)."
