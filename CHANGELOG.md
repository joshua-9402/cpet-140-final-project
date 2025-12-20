## What's Changed

- macOS: added ad-hoc code signing for the `.app` artifact and a CI step to apply ad-hoc signing to macOS builds to reduce Gatekeeper friction.


## Supported Platforms

### macOS
- Apple Silicon (ARM64) - macOS 11.0+
- Intel (x64) - macOS 15.0+

### Windows
- x64 and ARM64


## Build Information
- Build Type: Release
- Optimizations: Enabled (LTO)
- Compiler Flags: -O3 (macOS), /O2 (Windows)

## Requirements

### macOS
You need to execute this in the terminal before launching the app as macOS shows this error because the app is not code-signed.
macOS Gatekeeper blocks unsigned apps downloaded from the internet by setting a "quarantine" attribute.
```bash
xattr -cr <location-of-the-.app>
```

### Windows
No additional dependencies required.

## Support

For issues, questions, or feature requests, please visit the [Issues](https://github.com/joshua-9402/cpet-140-final-project/issues) page.

## Documentation

See the [README](README.md) for full documentation and usage instructions.
