# Linux Packaging Removed

Linux installer packaging (.deb/.rpm) and the associated `packaging/linux/postinst.sh` are no longer produced by this repository's release pipeline.

If you need to deploy on Linux, either:
- Build from source (recommended): `cmake -S . -B build && cmake --build build`, or
- Create distribution-specific packages in a separate packaging repository or CI pipeline.

See `doc/LINUX_PACKAGING_REMOVED.md` for guidance.
