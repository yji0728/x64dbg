# Build Instructions for Advanced Reconstructor Plugin

## Prerequisites

- Windows operating system
- CMake 3.15 or higher
- Visual Studio 2019 or higher
- Qt5 (for x64dbg GUI)

## Building the Plugin

The plugin is integrated into the x64dbg build system and will be built automatically when you build x64dbg.

### Option 1: Build with x64dbg (Recommended)

```bash
# From the x64dbg root directory
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The plugin will be output to:
- `bin/plugins/x64_AdvancedReconstructor.dll` (64-bit)
- `bin/plugins/x32_AdvancedReconstructor.dll` (32-bit)

### Option 2: Build Plugin Only

If you want to build just the plugin:

```bash
cd src/plugins/AdvancedReconstructor
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Installation

After building:

1. Copy the plugin DLL to x64dbg's plugins directory:
   ```
   <x64dbg_install>/x64/plugins/x64_AdvancedReconstructor.dll
   ```

2. Restart x64dbg

3. The plugin should appear in the Plugins menu

## Verification

To verify the plugin loaded correctly:

1. Start x64dbg
2. Check the log window for: `[AdvancedReconstructor] Plugin initialized`
3. Open the Plugins menu
4. Look for "Advanced Reconstructor" submenu with commands:
   - Start Auto-Analysis
   - OEP Candidates
   - Rebuild IAT
   - Dump & Rebuild PE
   - About

## Troubleshooting

### Plugin doesn't load

Check the x64dbg log for error messages. Common issues:
- Missing dependencies (x64dbg bridge DLL)
- Architecture mismatch (32-bit plugin in 64-bit debugger)
- Plugin not in correct directory

### Build errors

If you encounter build errors:

1. Ensure all x64dbg dependencies are built first
2. Check that include paths are correct
3. Verify Visual Studio is properly configured for C++17 or later

### Runtime errors

If the plugin loads but crashes:
- Check x64dbg version compatibility
- Ensure you're using the correct architecture (x64 vs x32)
- Review the log output for specific errors

## Development Setup

For plugin development:

1. Open the x64dbg solution in Visual Studio
2. Add the AdvancedReconstructor project to your workspace
3. Set breakpoints in plugin code
4. Build and debug through Visual Studio

Alternatively, use the x64dbg headless mode for testing:
```bash
x64dbg.exe -a <test_executable.exe>
```

## Testing

Currently, the plugin provides logging output to demonstrate its structure. To test:

1. Load any executable in x64dbg
2. Go to Plugins → Advanced Reconstructor → Start Auto-Analysis
3. Check the log window for output from each module
4. Try other menu commands to see the framework in action

## Clean Build

To perform a clean build:

```bash
# From build directory
cmake --build . --target clean
# Or delete the build directory and rebuild
```

## Notes

- This is an MVP implementation with placeholder functionality
- The plugin compiles and loads correctly but does not perform actual unpacking yet
- Future versions will implement the full functionality described in spec.md
- See IMPLEMENTATION.md for detailed feature roadmap
