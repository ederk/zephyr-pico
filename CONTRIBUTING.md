# Contributing to Zephyr Pico

Thank you for your interest in contributing to this project! We welcome contributions from the community.

## How to Contribute

### Reporting Issues

If you find a bug or have a suggestion for improvement:

1. Check if the issue already exists in the [Issues](https://github.com/ederk/zephyr-pico/issues) section
2. If not, create a new issue with a clear title and description
3. Include steps to reproduce (for bugs) or use cases (for features)
4. Add relevant labels to help categorize the issue

### Submitting Changes

1. **Fork the repository** and create a new branch from `main`
2. **Make your changes** following the project's coding standards
3. **Test your changes** thoroughly on relevant hardware
4. **Commit your changes** with clear, descriptive commit messages
5. **Push to your fork** and submit a pull request

### Pull Request Guidelines

- Keep changes focused and atomic (one feature/fix per PR)
- Update documentation to reflect any changes
- Follow the existing code style and conventions
- Add or update tests as needed
- Ensure your code builds without errors
- Reference any related issues in your PR description

## Development Setup

### Prerequisites

- Zephyr RTOS development environment
- West tool
- Supported hardware (RP2040, Pico 2, or similar)

### Building and Testing

```bash
# Navigate to the application directory
cd application

# Build for your target board
west build -b <board_name>

# Flash to hardware
west flash
```

## Code Standards

### General Guidelines

- Follow Zephyr RTOS coding conventions
- Keep hardware-specific code in board files
- Maintain portability in the application layer
- Use meaningful variable and function names
- Comment complex logic

### File Organization

- **Application logic**: `src/` and `inc/` directories
- **Board-specific**: `boards/` directory
- **Configuration**: `prj.conf` and board-specific `.conf` files
- **DeviceTree**: `.overlay` files in `boards/`

### Commit Messages

Write clear commit messages following this format:

```
<type>: <short summary>

<detailed description if needed>

Fixes #<issue_number>
```

Types: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`

## Adding Support for New Boards

To add support for a new microcontroller/board:

1. Create `boards/<board_name>.overlay` with hardware definitions
2. Create `boards/<board_name>.conf` with Kconfig settings
3. Test the build and functionality
4. Update documentation with the new board
5. Submit a PR with your changes

## Questions?

If you have questions about contributing:

- Open a discussion in the repository
- Check existing documentation
- Reach out to maintainers through issues

## License

By contributing to this project, you agree that your contributions will be licensed under the Apache License 2.0.
