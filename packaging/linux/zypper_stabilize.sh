#!/usr/bin/env bash
# Stabilize zypper on openSUSE Tumbleweed by fixing the flaky openh264 repo.
# - Prefer HTTPS for codecs.opensuse.org
# - Retry refresh with GPG auto-import
# - If it still fails, disable the openh264 repo and continue

set -euo pipefail

echo "[zypper-stabilize] Starting repository stabilization"

# If not an openSUSE system or zypper missing, exit quietly
if ! command -v zypper >/dev/null 2>&1; then
  echo "[zypper-stabilize] zypper not found; skipping"
  exit 0
fi

# Patch HTTP to HTTPS for codecs.opensuse.org if repo files exist
if ls /etc/zypp/repos.d/*.repo >/dev/null 2>&1; then
  echo "[zypper-stabilize] Ensuring HTTPS for codecs.opensuse.org"
  # shellcheck disable=SC2016
  sudo sed -i 's|http://codecs.opensuse.org|https://codecs.opensuse.org|g' /etc/zypp/repos.d/*.repo || true
fi

refresh_with_retry() {
  local tries=${1:-3}
  local delay=${2:-5}
  local n=1
  set +e
  while (( n <= tries )); do
    echo "[zypper-stabilize] Refresh attempt $n/$tries"
    if sudo zypper -n --gpg-auto-import-keys ref; then
      set -e
      return 0
    fi
    (( n++ ))
    sleep "$delay"
  done
  set -e
  return 1
}

if refresh_with_retry 3 5; then
  echo "[zypper-stabilize] Repository refresh succeeded"
  exit 0
fi

echo "[zypper-stabilize] Refresh still failing; attempting to disable openh264 repo"

# Try to disable only the openh264 repo (identified by URL)
set +e
openh264_aliases=$(sudo zypper -n lr | awk '/codecs\.opensuse\.org\/openh264/ {print $3}')
set -e

if [[ -n "${openh264_aliases:-}" ]]; then
  for alias in $openh264_aliases; do
    echo "[zypper-stabilize] Disabling repo: $alias"
    sudo zypper -n mr -d "$alias" || true
  done
else
  echo "[zypper-stabilize] No explicit openh264 repo alias found; proceeding"
fi

if sudo zypper -n --gpg-auto-import-keys ref; then
  echo "[zypper-stabilize] Repository refresh succeeded after disabling openh264"
  exit 0
fi

echo "[zypper-stabilize] Repository refresh still failing; continuing (non-fatal)"
exit 0
