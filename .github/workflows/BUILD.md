# Build Workflows Documentation (macOS & Windows only)

This repository provides build documentation and workflows for macOS and Windows targets only.

## Supported Platforms
- macOS (Apple Silicon and Intel)
- Windows (x64 and ARM64)

## Manual Builds
- macOS workflows: see `.github/workflows/` for macOS-specific README files.
- Windows workflows: see `.github/workflows/` for Windows-specific README files.

## Automated Builds
Automated builds run on push for Windows; macOS builds are manual-trigger only to allow architecture selection.

## How to Trigger Builds
- Use the GitHub Actions UI to run macOS or Windows workflows documented in `.github/workflows/`.

## Downloading Artifacts
Artifacts produced by the workflows are available from the Actions run page or from Releases when created by the release workflow.

## Troubleshooting
Check the workflow logs in GitHub Actions and the README files under `.github/workflows/` for platform-specific instructions.

**Last Updated:** 2025-12-19
