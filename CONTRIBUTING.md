# Contributing to dtuGateway

Thank you for your interest in contributing to dtuGateway! We welcome contributions in many forms.

## Ways to Contribute

### 🧪 Testing & Feedback
- **Download snapshot releases** and test new features before they're released as stable
- **Report bugs** in [GitHub Issues](https://github.com/ohAnd/dtuGateway/issues) with detailed information
- **Share feedback** in [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions)
- **Help other users** answer questions and share your experience

### 💻 Code Contributions
We welcome pull requests for bug fixes, features, and improvements:

1. **Create or Find an Issue**: Start with a [GitHub Issue](https://github.com/ohAnd/dtuGateway/issues) describing the feature or bug
2. **Fork** the repository to your GitHub account
3. **Create a Feature Branch**:
   - Use GitHub's "Create a branch" button on the issue page
   - Branch from latest `develop` (not `main`)
   - Use descriptive names: `feature/issue-number-description` or `bugfix/issue-number-description`
4. **Develop & Test**:
   - Make changes in your feature branch
   - Test locally with PlatformIO: `pio run -e esp32`
   - Ensure code follows project conventions
5. **Pull Request**:
   - Submit PR from your feature branch → `develop` branch
   - Reference the original issue in PR description
   - Include testing details and any breaking changes
6. **Review Process**: Maintainer review, community feedback, automated testing
7. **Merge**: After approval, your branch gets merged into `develop` and triggers automatic snapshot build

### 💝 Financial Support
If you find this project useful but don't have time to contribute code, you can also support the project by [becoming a sponsor](https://github.com/sponsors/ohAnd).

Your support helps:
- Keep the project active and maintained
- Enable faster development and bug fixes
- Support hardware testing with various setups
- Improve documentation and user experience

## Development Setup

### Prerequisites
```bash
# Install PlatformIO
pip install platformio

# Clone repository
git clone https://github.com/ohAnd/dtuGateway.git
cd dtuGateway
```

### Building Locally
```bash
# Create version file (required for local builds)
echo "localDev" > include/buildnumber.txt

# Build for ESP32
pio run -e esp32

# Upload to device
pio run -e esp32 -t upload

# Monitor serial output
pio device monitor
```

## Reporting Issues

When reporting bugs, please include:
- **Device info**: ESP32 model, display type
- **Firmware version**: Stable or snapshot with version number
- **Serial logs**: Connect via USB and capture debug output (115200 baud)
- **Steps to reproduce**: Detailed description of the issue
- **Configuration**: Relevant settings (remove sensitive data like WiFi passwords)

## Code Standards

- **Follow existing code style** in the repository
- **Add comments** for complex logic
- **Test your changes** before submitting a PR
- **Update documentation** if your change affects user-facing features

## Support Channels

- **Documentation**: [README.md](README.md) and inline code comments
- **Community**: [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions)
- **Bug Reports**: [GitHub Issues](https://github.com/ohAnd/dtuGateway/issues)
- **Development**: Pull requests welcome with tests

Thanks for contributing! 🌞⚡
