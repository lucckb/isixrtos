# Devcontainer quick start (Phase 1)

QEMU-focused Dev Container for isolated ISIX development.

Base image: **Debian 13 (trixie)** slim. Runtime tag: **`isixrtos-qemu-dev:dev`**.

## What this includes

- Meson (>= 1.2) + Ninja
- `arm-none-eabi` toolchain built from source (GCC 16.1, newlib, gdb)
- Patched QEMU (`v10.1.2`) for STM32 timer fixes required by ISIX tests
- System **clangd** (`/usr/bin/clangd`) for navigation
- Helper scripts for first setup and QEMU execution

Phase 1 is **QEMU only**. Hardware USB flashing/debugging (OpenOCD/ST-Link) is later.

## Quick start

1. Open the repository in Cursor / VS Code (Docker Desktop, or Podman with a Docker-compatible API).
2. **Dev Containers: Reopen in Container**.
3. First open runs `image.sh ensure` on the **host** — building GCC + QEMU can take a long time.
   Dangling layers from interrupted builds are pruned automatically.
4. Wait for `post-create` (configure + editor configs). Extensions should install from
   `devcontainer.json`; if not, open Extensions → `@recommended` and install.
5. Build and run tests:

```bash
meson compile -C build
bash .devcontainer/scripts/qemu-run.sh
```

(`post-create` already runs `./configure build qemu` when `build/` is missing.)

### Rebuild the image (toolchain / Dockerfile changed)

IDE **Rebuild Container** alone does **not** rebuild the Dockerfile (the config uses a
tagged local image). Rebuild explicitly, then reopen:

```bash
.devcontainer/scripts/image.sh ensure --force
# then: Dev Containers → Rebuild Container
```

### Disk full after a failed / cancelled build

```bash
.devcontainer/scripts/image.sh prune        # dangling layers + stopped containers
.devcontainer/scripts/image.sh prune --all  # also unused images + build cache
```

## Debug in VS Code / Cursor (QEMU)

On create, `post-create.sh` copies QEMU-only configs from `.devcontainer/vscode/` into `.vscode/`.

### Required extensions (from `devcontainer.json`)

| Extension | ID | Role |
|-----------|-----|------|
| **clangd** | `llvm-vs-code-extensions.vscode-clangd` | Go to definition / references |
| **Meson** | `mesonbuild.mesonbuild` | Meson syntax / tasks |
| **Native Debug** | `webfreak.debug` | GDB launch configs, asm stepping |
| **C/C++ Debug** | `kylinideteam.cppdebug` | `cppdbg` + Disassembly View |

`ms-vscode.cpptools` is not available in Cursor; use the extensions above.

If clangd shows errors about a missing binary, settings must use **`/usr/bin/clangd`**
(image package) — not `~/.local/bin/clangd`.

### Start debugging

1. **Run and Debug** (Ctrl+Shift+D)
2. Choose one of:
   - **QEMU: debug isixtests (start QEMU)** — builds, starts QEMU paused, attaches GDB (recommended)
   - **QEMU: attach isixtests** — attach when QEMU already runs (`qemu-gdb-server.sh`)
   - **QEMU: GDB start QEMU (Native Debug)** — same via Native Debug extension
3. GDB stops at reset; press **Continue** (F5) once to reach `main`
4. Set breakpoints in `tests/libisix/*.cpp` or `libfoundation/src/sys/tiny_printf.c`

GDB connects to QEMU on port **1234** (forwarded by the devcontainer).

### Disassembly view (asm + C)

During an active debug session:

1. **Command Palette** → `Debug: Open Disassembly View`
2. Or use **Step Into** (`F11`) — with `disassemble-next-line on`, GDB shows asm next to source
3. For libgcc helpers (`__aeabi_uldivmod` / `bpabi.S`), use **Step Instruction** in the Disassembly view

### Troubleshooting (debug)

| Symptom | Cause / fix |
|---------|-------------|
| `TargetArchitecture not detected, assuming x86_64` | Harmless host-side warning; configs set `targetArchitecture: arm` |
| Stops at `crt0.c` then `exited with code 0` | Tests finished (`SYS_EXIT`). Break before second Continue, or stop at `main` |
| GDB cannot connect | Start QEMU with `-S -gdb tcp::1234` (`qemu-gdb-server.sh`) |
| Session ends when tests complete | Expected with `-semihosting`; restart QEMU and attach again |

Hardware/OpenOCD configs remain in `extras/scripts/vscode_tpl/` (Phase 2).

## C/C++ navigation (clangd)

1. Ensure `build/compile_commands.json` exists (`./configure build qemu` / `post-create`).
2. **Trust the workspace** if prompted.
3. Reload once after create: **Developer: Reload Window**.
4. Wait for indexing (status: `clangd: idle`), then F12 / Shift+F12.

If navigation is empty:

```bash
meson compile -C build
```

Then **Developer: Reload Window**.

| Symptom | Fix |
|---------|-----|
| `clangd.path` / binary not found | Use `/usr/bin/clangd` (see `.devcontainer/vscode/settings.json`) |
| Extension missing | Extensions → `@recommended` → install clangd |
| Wrong / empty results | Rebuild `compile_commands.json`, reload window |

## Run with GDB wait (manual)

```bash
.devcontainer/scripts/qemu-gdb-server.sh
```

Or:

```bash
qemu-system-arm -M olimex-stm32-h405 -semihosting \
  -kernel build/tests/libisix/isixtests.binary -nographic -S -gdb tcp::1234
```

## Notes for host operating systems

- Linux: Docker or Podman (Docker-compatible socket for the IDE).
- Windows (WSL2): Docker integrated with WSL2.
- macOS: Docker Desktop or Podman.

## Keep host workflow unchanged

Dev Container support is optional:

```bash
./configure build qemu
meson compile -C build
```

## Container image (Docker or Podman)

One helper — `.devcontainer/scripts/image.sh` — is used by **both** the IDE
(`initializeCommand` → `ensure`) and the CLI.

```bash
.devcontainer/scripts/image.sh ensure             # first open / reuse tagged image
.devcontainer/scripts/image.sh ensure --force     # rebuild GCC/QEMU image
.devcontainer/scripts/image.sh build              # same as force rebuild
CONTAINER_ENGINE=podman .devcontainer/scripts/image.sh build

.devcontainer/scripts/image.sh prune
.devcontainer/scripts/image.sh prune --all
.devcontainer/scripts/image.sh info
```

Default tag: `isixrtos-qemu-dev:dev` (OCI title/description labels).
Multi-stage leftovers are pruned after success or failure.

> Cursor/VS Code need a Docker-compatible API. On Fedora/RHEL use `podman-docker`
> or point `DOCKER_HOST` at the Podman socket. CLI-only builds can set
> `CONTAINER_ENGINE=podman` directly.

### Fully remove the environment

- Delete the Dev Container in the IDE
- `.devcontainer/scripts/image.sh prune --all`
- Remove leftover `vsc-*` images if the IDE left any
- Remove `build/` for a clean repo tree
