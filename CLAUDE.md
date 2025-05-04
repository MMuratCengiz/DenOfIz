# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Build project: `cmake --build build --config Debug`
- Build examples: Set `BUILD_EXAMPLES=ON` in CMake configuration
- Run tests: `cmake --build build --target DenOfIzGraphics-Tests && cd build && ctest`
- Run single test: `cd build && ./Graphics/Tests/DenOfIzGraphics-Tests --gtest_filter=TestName*`

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

## Font System Overview

The font rendering system uses a component-based architecture with clear separation of concerns:

1. **FontLoader**: Handles loading font files using FreeType
   - Loads font files and extracts metrics
   - Creates FreeType faces
   - Provides access to font data

2. **FontLayoutManager**: Manages text layout with HarfBuzz
   - Text shaping with proper script support
   - Handles proper glyph positioning
   - Supports different text directions and scripts

3. **MsdfGenerator**: Generates Multi-channel Signed Distance Fields for glyphs
   - Creates MSDF representations of glyphs
   - Handles edge coloring and distance field generation
   - Provides sharp, scalable glyphs

4. **GlyphVertexGenerator**: Generates vertex data for rendering
   - Creates quads for each glyph
   - Positions glyphs based on layout information
   - Handles text alignment

5. **FontCache**: Manages the atlas texture and glyph storage
   - Stores glyphs in a texture atlas
   - Tracks glyph positions in the atlas
   - Manages glyph metrics

6. **FontManager**: Pure coordinator with minimal logic
   - Provides access to specialized components
   - Manages font cache
   - Delegates operations to appropriate components
   - Does not duplicate functionality from other components

7. **FontRenderer**: Renders text using graphics APIs
   - Creates and manages GPU resources
   - Handles shader program setup
   - Renders batched text efficiently
   - Works directly with specialized components when needed

The design emphasizes proper dependency injection and follows single responsibility principle. Components can be used directly or through the FontManager, which acts as a facade to simplify common operations.
