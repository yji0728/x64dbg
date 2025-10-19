# Advanced Reconstructor Plugin for x64dbg

## Overview

Advanced Reconstructor is an advanced unpacking plugin for x64dbg that assists in analyzing and unpacking packed/obfuscated Windows PE files.

## Features (MVP Implementation)

This is a Minimum Viable Product (MVP) implementation that provides:

### 1. PackDetector (패킹 감지 엔진)
- Static analysis of PE headers and sections
- Detection of suspicious section characteristics
- Basic entropy and import table analysis
- Packing suspicion scoring

### 2. RuntimeInstrumentor (런타임 계측기)
- Debug event monitoring
- Exception handling and logging
- Memory protection change tracking (W->X transitions)
- Foundation for API call tracing

### 3. OEPFinder (OEP 탐지 엔진)
- Original Entry Point (OEP) detection using multiple heuristics
- Analysis of control flow patterns
- Function prologue pattern matching
- Confidence scoring for OEP candidates

### 4. Dumper & Rebuilder (덤퍼 및 재구성기)
- Memory dumping at detected OEP
- PE file reconstruction
- Entry point fixing in PE headers
- Basic IAT reconstruction framework

### 5. MultiLayerController (다중 레이어 제어기)
- Framework for multi-layer packed file handling
- Configurable maximum unpacking depth

### 6. User Interface
- Menu integration with x64dbg
- Start Auto-Analysis command
- OEP Candidates viewer
- Rebuild IAT command
- Dump & Rebuild PE command

## Usage

1. Load a packed executable in x64dbg
2. From the Plugins menu, select "Advanced Reconstructor"
3. Click "Start Auto-Analysis" to begin analysis
4. Review OEP candidates with "OEP Candidates"
5. Rebuild the IAT with "Rebuild IAT"
6. Dump the unpacked file with "Dump & Rebuild PE"

## Building

This plugin is built as part of the x64dbg project. To build:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

The plugin will be output to `bin/plugins/` directory.

## Legal Notice

This plugin is intended for **legitimate reverse engineering purposes only**, including:
- Security research
- Educational purposes
- Analysis of your own software
- Authorized malware analysis

**Prohibited uses:**
- DRM circumvention
- Software piracy
- Unauthorized cracking
- Any illegal activities

## Architecture

The plugin follows a modular architecture with 6 core components:

1. **PackDetector**: Detects packing through static and dynamic analysis
2. **RuntimeInstrumentor**: Monitors runtime behavior and API calls
3. **OEPFinder**: Locates the original entry point using multiple heuristics
4. **Dumper**: Dumps memory and reconstructs PE files
5. **MultiLayerController**: Handles multi-layer packed files
6. **UI Framework**: Provides user interface and command integration

## Implementation Status

This is an MVP (Minimum Viable Product) implementation as defined in section 8 of the specification (spec.md). 

### Implemented:
- [x] Basic plugin infrastructure
- [x] Static packing detection
- [x] Core API tracing framework
- [x] Basic OEP heuristics
- [x] Memory dump functionality
- [x] PE entry point fixing
- [x] Menu integration

### Future Enhancements (Beta/v1.0):
- [ ] Advanced W->X transition tracking
- [ ] Full API call tracing (VirtualAlloc, VirtualProtect, etc.)
- [ ] YARA signature-based packer identification
- [ ] Complete IAT reconstruction
- [ ] TLS/Relocation/Resource restoration
- [ ] Scylla/ScyllaHide integration
- [ ] Unpack Wizard UI
- [ ] HTML/JSON reporting
- [ ] User-defined rules and scripting

## License

Part of the x64dbg project. See main project LICENSE for details.
