# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Configure project: `cmake -S . -B build`
- Build project: `cmake --build build --config Debug`
- Build examples: `cmake -DBUILD_EXAMPLES=ON -S . -B build`
- Run all tests: `cmake --build build --target DenOfIzGraphics-Tests && cd build && ctest`
- Run single test: `cd build && ./Graphics/Tests/DenOfIzGraphics-Tests --gtest_filter=TestName*`
- Clean build: `cmake --build build --target clean`

## Lint & Format
- Format code: `clang-format -i path/to/file.cpp`
- Check format: `clang-format --dry-run --Werror path/to/file.cpp`

## Code Style Guidelines
- Follow Microsoft C++ style with braces on new lines (BraceWrapping)
- Use tabs for indentation (4 spaces width)
- Include order: 1) Standard headers 2) Project headers 3) Other headers
- Max line length: 180 characters
- Use namespaces for organization (namespace DenOfIz)
- Naming: camelCase for variables/members, PascalCase for types/classes
- Always use braces for control statements
- Error handling via return codes or exceptions based on context
- Use spaces in parentheses and between operators
- Align consecutive assignments, declarations and trailing comments
- Break constructor initializers after colon
- Use custom spaces in parentheses including conditional statements

## Font System Overview

The font rendering system uses a component-based architecture with clear separation of concerns:

1. **FontLoader**: Handles loading font files using FreeType
2. **FontLayoutManager**: Manages text layout with HarfBuzz
3. **MsdfGenerator**: Generates Multi-channel Signed Distance Fields for glyphs
4. **GlyphVertexGenerator**: Generates vertex data for rendering
5. **FontCache**: Manages the atlas texture and glyph storage
6. **FontManager**: Pure coordinator with minimal logic
7. **FontRenderer**: Renders text using graphics APIs

The design emphasizes proper dependency injection and follows single responsibility principle. Components can be used directly or through the FontManager, which acts as a facade to simplify common operations.