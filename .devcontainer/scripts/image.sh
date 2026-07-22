#!/usr/bin/env bash
#
# image.sh — build / ensure / clean the ISIX QEMU contributor image (Docker or Podman).
#
# Used by both the CLI and devcontainer.json initializeCommand so IDE and host
# share one tagged image and the same cleanup rules.
#
# Usage:
#   .devcontainer/scripts/image.sh ensure             # build if missing (IDE entry)
#   .devcontainer/scripts/image.sh ensure --force     # rebuild even if present
#   .devcontainer/scripts/image.sh build              # always rebuild + cleanup
#   .devcontainer/scripts/image.sh build --tag gcc16
#   .devcontainer/scripts/image.sh prune              # dangling images / stopped ctrs
#   .devcontainer/scripts/image.sh prune --all        # + unused images + build cache
#   .devcontainer/scripts/image.sh info               # show kept image + labels
#
# Engine: auto-detects docker or podman (override with CONTAINER_ENGINE=podman|docker).
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT}"

IMAGE_NAME="${IMAGE_NAME:-isixrtos-qemu-dev}"
IMAGE_TAG="${IMAGE_TAG:-dev}"
DOCKERFILE="${DOCKERFILE:-.devcontainer/Dockerfile}"

die() { echo "error: $*" >&2; exit 1; }

usage() {
	cat <<EOF
Usage: $(basename "$0") <command> [options]

Commands:
  ensure [--tag TAG] [--force]        For Dev Containers: reuse the tagged image if
                                      present, otherwise build. Always drops dangling
                                      layers afterward. --force rebuilds.
  build [--tag TAG] [--prune-cache]   Always rebuild and tag, then drop dangling layers.
  prune [--all]                       Free disk after a failed / interrupted build.
                                      Default: dangling images + stopped containers.
                                      --all also removes unused images and build cache.
  info [--tag TAG]                    Show the tagged image and its OCI labels.

Environment:
  CONTAINER_ENGINE   docker | podman (default: auto)
  IMAGE_NAME         repository name (default: ${IMAGE_NAME})
  IMAGE_TAG          default tag when --tag is omitted (default: ${IMAGE_TAG})
  ISIX_IMAGE_REBUILD=1                Same as ensure --force (handy for IDE rebuilds)

Examples:
  $(basename "$0") ensure
  $(basename "$0") ensure --force
  $(basename "$0") build
  CONTAINER_ENGINE=podman $(basename "$0") build --tag gcc16
  $(basename "$0") prune
  $(basename "$0") prune --all
EOF
}

engine_ready() {
	local name="$1"
	command -v "${name}" >/dev/null 2>&1 || return 1
	# Prefer a reachable daemon/socket; fall through if the CLI exists but is idle.
	"${name}" info >/dev/null 2>&1
}

resolve_engine() {
	local eng="${CONTAINER_ENGINE:-}"
	if [[ -n "${eng}" ]]; then
		case "${eng}" in
			docker|podman) ;;
			*) die "CONTAINER_ENGINE must be docker or podman (got: ${eng})" ;;
		esac
		command -v "${eng}" >/dev/null 2>&1 || die "CONTAINER_ENGINE=${eng} not found in PATH"
		printf '%s\n' "${eng}"
		return
	fi
	# Prefer a working engine; Docker first (VS Code / Cursor Dev Containers), then Podman.
	if engine_ready docker; then
		printf '%s\n' docker
		return
	fi
	if engine_ready podman; then
		printf '%s\n' podman
		return
	fi
	# Last resort: CLI present without a running service (build may still work rootless).
	if command -v docker >/dev/null 2>&1; then
		printf '%s\n' docker
		return
	fi
	if command -v podman >/dev/null 2>&1; then
		printf '%s\n' podman
		return
	fi
	die "neither docker nor podman is available (install one, or set CONTAINER_ENGINE=...)"
}

ENGINE=""

ensure_engine() {
	[[ -n "${ENGINE}" ]] && return
	ENGINE="$(resolve_engine)"
}

ctr() {
	ensure_engine
	"${ENGINE}" "$@"
}

full_ref() {
	printf '%s:%s\n' "${IMAGE_NAME}" "${IMAGE_TAG}"
}

image_exists() {
	local ref="$1"
	ctr image inspect "${ref}" >/dev/null 2>&1
}

prune_dangling() {
	echo "→ removing dangling images"
	ctr image prune -f >/dev/null 2>&1 || true
}

cmd_ensure() {
	local force=0
	while [[ $# -gt 0 ]]; do
		case "$1" in
			--tag) IMAGE_TAG="${2:?}"; shift 2 ;;
			--force) force=1; shift ;;
			-h|--help) usage; return 0 ;;
			*) die "unknown ensure option: $1" ;;
		esac
	done
	if [[ "${ISIX_IMAGE_REBUILD:-0}" == "1" ]]; then
		force=1
	fi

	local ref
	ref="$(full_ref)"
	ensure_engine
	echo "Using ${ENGINE}"

	if [[ "${force}" -eq 0 ]] && image_exists "${ref}"; then
		echo "Image ${ref} already present (skip build)."
		echo "Rebuild with: $(basename "$0") ensure --force   or   ISIX_IMAGE_REBUILD=1"
		prune_dangling
		cmd_info
		return 0
	fi

	if [[ "${force}" -eq 1 ]]; then
		echo "Force rebuild of ${ref}"
	else
		echo "Image ${ref} missing — building (this can take a long time: GCC + QEMU)."
	fi
	cmd_build
}

cmd_prune() {
	local all=0
	while [[ $# -gt 0 ]]; do
		case "$1" in
			--all) all=1; shift ;;
			-h|--help) usage; return 0 ;;
			*) die "unknown prune option: $1" ;;
		esac
	done

	ensure_engine
	echo "Using ${ENGINE}"
	echo "Disk before:"
	ctr system df 2>/dev/null || true

	echo
	echo "→ stopped containers"
	ctr container prune -f 2>/dev/null || true

	echo
	echo "→ dangling images (<none>)"
	ctr image prune -f

	if [[ "${all}" -eq 1 ]]; then
		echo
		echo "→ unused images (not used by any container)"
		ctr image prune -a -f
		echo
		echo "→ build cache"
		if [[ "${ENGINE}" == docker ]]; then
			ctr builder prune -f 2>/dev/null || ctr system prune -f
		else
			# Podman: build cache lives in system storage
			ctr system prune -f 2>/dev/null || true
		fi
	fi

	echo
	echo "Disk after:"
	ctr system df 2>/dev/null || true
	echo
	echo "Tagged ${IMAGE_NAME} images:"
	ctr images "${IMAGE_NAME}" 2>/dev/null || echo "(none)"
}

cmd_info() {
	while [[ $# -gt 0 ]]; do
		case "$1" in
			--tag) IMAGE_TAG="${2:?}"; shift 2 ;;
			-h|--help) usage; return 0 ;;
			*) die "unknown info option: $1" ;;
		esac
	done
	local ref
	ref="$(full_ref)"
	ensure_engine
	echo "Using ${ENGINE}"
	ctr image inspect "${ref}" >/dev/null 2>&1 || die "image not found: ${ref} (run: $(basename "$0") build)"
	# CreatedSince is Docker-oriented; fall back to a plain listing for older Podman.
	if ! ctr images --format 'table {{.Repository}}\t{{.Tag}}\t{{.ID}}\t{{.Size}}' "${IMAGE_NAME}" 2>/dev/null; then
		ctr images "${IMAGE_NAME}"
	fi
	echo
	ctr image inspect "${ref}" --format \
		'title: {{index .Config.Labels "org.opencontainers.image.title"}}
description: {{index .Config.Labels "org.opencontainers.image.description"}}
stage: {{index .Config.Labels "org.opencontainers.image.stage"}}
created: {{index .Config.Labels "org.opencontainers.image.created"}}' 2>/dev/null \
		|| echo "(OCI labels unavailable for ${ref})"
}

cmd_build() {
	local prune_cache=0
	while [[ $# -gt 0 ]]; do
		case "$1" in
			--tag) IMAGE_TAG="${2:?}"; shift 2 ;;
			--prune-cache) prune_cache=1; shift ;;
			-h|--help) usage; return 0 ;;
			*) die "unknown build option: $1" ;;
		esac
	done

	local ref
	ref="$(full_ref)"
	ensure_engine
	echo "Using ${ENGINE}"
	echo "Building ${ref}"
	echo "Dockerfile: ${DOCKERFILE}"

	local failed=0
	set +e
	ctr build \
		-f "${DOCKERFILE}" \
		-t "${ref}" \
		--label "org.opencontainers.image.created=$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
		--label "org.opencontainers.image.ref.name=${ref}" \
		.
	failed=$?
	set -e

	echo
	echo "→ removing dangling images from this build"
	prune_dangling

	if [[ "${failed}" -ne 0 ]]; then
		die "build failed (exit ${failed}); dangling layers pruned"
	fi

	if [[ "${prune_cache}" -eq 1 ]]; then
		echo "→ pruning build cache (--prune-cache)"
		if [[ "${ENGINE}" == docker ]]; then
			ctr builder prune -f >/dev/null 2>&1 || true
		else
			ctr system prune -f >/dev/null 2>&1 || true
		fi
	fi

	echo
	echo "Kept:"
	cmd_info
}

main() {
	local cmd="${1:-}"
	[[ -n "${cmd}" ]] || { usage; exit 1; }
	shift || true
	case "${cmd}" in
		ensure) cmd_ensure "$@" ;;
		build)  cmd_build "$@" ;;
		prune)  cmd_prune "$@" ;;
		info)   cmd_info "$@" ;;
		-h|--help|help) usage ;;
		*) die "unknown command: ${cmd} (try: ensure | build | prune | info)" ;;
	esac
}

main "$@"
