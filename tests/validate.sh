#!/usr/bin/env bash
set -Eeuo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${repo_dir}"

required_files=(
  repository.yaml
  spotconnect/config.yaml
  spotconnect/Dockerfile
  spotconnect/patch-mdns.py
  spotconnect/icon.png
  spotconnect/logo.png
  spotconnect/DOCS.md
  spotconnect/CHANGELOG.md
  spotconnect/apparmor.txt
  spotconnect/rootfs/etc/services.d/spotconnect/run
  spotconnect/rootfs/etc/services.d/spotconnect/finish
)

for file in "${required_files[@]}"; do
  [[ -f "${file}" ]] || { echo "Missing required file: ${file}" >&2; exit 1; }
done

bash -n spotconnect/rootfs/etc/services.d/spotconnect/run
bash -n spotconnect/rootfs/etc/services.d/spotconnect/finish
PYTHONPYCACHEPREFIX="${TMPDIR:-/tmp}/ha-spotconnect-pycache" \
  python3 -m py_compile spotconnect/patch-mdns.py

upstream="$(tr -d '[:space:]' < .upstream-version)"
app_version="$(sed -n 's/^version: "\([^"]*\)"/\1/p' spotconnect/config.yaml)"
docker_version="$(sed -n 's/^ARG SPOTCONNECT_VERSION=//p' spotconnect/Dockerfile)"
checksum="$(sed -n 's/^ARG SPOTCONNECT_SHA256=//p' spotconnect/Dockerfile)"

[[ "${upstream}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || { echo "Invalid upstream version" >&2; exit 1; }
[[ "${app_version}" =~ ^${upstream//./\.}-[1-9][0-9]*$ ]] || { echo "App version does not match upstream version" >&2; exit 1; }
[[ "${docker_version}" == "${upstream}" ]] || { echo "Dockerfile version does not match .upstream-version" >&2; exit 1; }
[[ "${checksum}" =~ ^[0-9a-f]{64}$ ]] || { echo "Invalid SpotConnect archive checksum" >&2; exit 1; }

grep -q '^host_network: true$' spotconnect/config.yaml
grep -q '^  airplay_enabled: true$' spotconnect/config.yaml
grep -q '^  upnp_enabled: false$' spotconnect/config.yaml
grep -q 'ghcr.io/allcrafter1/ha-spotconnect' spotconnect/config.yaml

echo "Validation passed for SpotConnect ${app_version}"
