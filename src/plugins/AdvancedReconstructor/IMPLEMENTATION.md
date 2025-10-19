# Advanced Reconstructor Plugin - Implementation Guide

## Overview

This document explains how the Advanced Reconstructor plugin implementation aligns with the specification in `spec.md`.

## Specification Reference

The plugin is based on the Korean-language specification document `spec.md` which describes an "Advanced Reconstructor" plugin for x64dbg. The spec defines a comprehensive unpacking plugin with the following goals:

### Core Goals (From Spec Section 1)
- Support analysis and semi-automatic/automatic unpacking of packed/obfuscated Windows PE files (x64 focus, x86 optional)
- Implement essential unpacking steps (OEP detection, IAT reconstruction, section restoration) with stability focus
- Provide transparent anti-debugging countermeasures
- Support multi-layer unpacking
- Offer both expert interface and beginner-friendly "Unpack Wizard"

## Implementation Status

This is an **MVP (Minimum Viable Product)** implementation as defined in Section 8 of the specification.

### What's Implemented

#### 1. Core Architecture (Spec Section 4)
✅ All 6 core modules defined in the spec are implemented as separate C++ modules:

- **PackDetector** (`PackDetector.cpp/h`)
- **RuntimeInstrumentor** (`RuntimeInstrumentor.cpp/h`)
- **OEPFinder** (`OEPFinder.cpp/h`)
- **Dumper & Rebuilder** (`Dumper.cpp/h`)
- **MultiLayerController** (`MultiLayerController.cpp/h`)
- **Plugin Core** (`plugin.cpp/h`)

#### 2. Module Details

##### PackDetector (Spec Section 5.1)
**Spec Requirements:**
- Static analysis: PE header anomalies, high entropy sections, abnormal section flags, sparse import table
- Dynamic analysis: Memory allocation patterns, W->X transitions
- Signature-based detection: YARA rules for known packers
- Output: Suspicion score, estimated packer type, recommended workflow

**MVP Implementation:**
- ✅ Module structure and initialization
- ✅ Basic framework for static analysis
- ⏳ Detailed PE parsing (future)
- ⏳ Entropy calculation (future)
- ⏳ YARA integration (future)

##### RuntimeInstrumentor (Spec Section 5.2)
**Spec Requirements:**
- API tracing: VirtualAlloc, VirtualProtect, WriteProcessMemory, LoadLibrary, GetProcAddress
- W->X transition monitoring using PAGE_GUARD or memory breakpoints
- Anti-anti-debugging: Optionally hook IsDebuggerPresent, CheckRemoteDebuggerPresent, etc.
- All manipulations must be transparently logged

**MVP Implementation:**
- ✅ Module structure and event handling framework
- ✅ Debug event callback infrastructure
- ✅ Exception callback infrastructure
- ⏳ API hooking implementation (future)
- ⏳ Memory transition tracking (future)
- ⏳ Anti-anti-debugging features (future)

##### OEPFinder (Spec Section 5.3)
**Spec Requirements:**
- Multiple heuristics: Control flow analysis, W->X memory changes, code characteristics, import usage
- Candidate scoring with confidence levels
- Present ranked candidates to user

**MVP Implementation:**
- ✅ Module structure and candidate management
- ✅ Framework for multiple heuristics
- ✅ Confidence scoring system
- ⏳ Control flow analysis (future)
- ⏳ Prologue pattern matching (future)
- ⏳ Memory transition analysis (future)

##### Dumper & Rebuilder (Spec Section 5.4)
**Spec Requirements:**
- Intelligent memory dumping at OEP
- PE reconstruction: Entry point fixing, section alignment, IAT reconstruction
- IAT reconstruction: Dynamic tracing, static scanning, optional Scylla integration
- Data directory restoration: Relocations, TLS, Resources, Load Config
- Output: Unpacked PE file and detailed analysis report (JSON/HTML)

**MVP Implementation:**
- ✅ Module structure
- ✅ Framework for memory dumping
- ✅ Framework for PE reconstruction
- ⏳ Actual memory dumping (future)
- ⏳ PE header fixing (future)
- ⏳ IAT reconstruction (future)
- ⏳ Scylla integration (future)

##### MultiLayerController (Spec Section 5.5)
**Spec Requirements:**
- Automate "dump -> reload -> analyze" workflow
- Support configurable max layers and timeouts

**MVP Implementation:**
- ✅ Module structure
- ✅ Layer tracking
- ✅ Configurable max layers
- ⏳ Automatic iteration (future)

##### UI and Workflow (Spec Section 6)
**Spec Requirements:**
- Dock panel with multiple tabs (Overview, Events Timeline, OEP Candidates, Rebuild Panel)
- "Unpack Wizard" for beginners
- Workflow: Start Auto-Analysis -> Review OEP Candidates -> Rebuild IAT -> Dump & Rebuild PE

**MVP Implementation:**
- ✅ Menu integration with x64dbg
- ✅ Commands: Start Auto-Analysis, OEP Candidates, Rebuild IAT, Dump & Rebuild PE
- ✅ About dialog
- ⏳ Dock panel UI (future)
- ⏳ Unpack Wizard (future)
- ⏳ Events timeline (future)

#### 3. Build System
✅ CMake configuration for plugin compilation
✅ Integration with main x64dbg build system via cmake.toml
✅ Separate plugin directory structure

#### 4. Documentation
✅ README.md with usage instructions
✅ This implementation guide
✅ Legal notice (legitimate use only)

### What's NOT Implemented (Future Versions)

According to Spec Section 8, these features are planned for Beta and v1.0:

#### Beta Features
- Advanced W->X transition tracking
- TLS/Reloc/Resource restoration
- Multi-layer unpacking loop
- Unpack Wizard UI
- Scylla/ScyllaHide integration
- Signature database updates
- Full dock panel UI

#### v1.0 Features
- Improved OEP confidence scoring algorithm
- User-defined rules and scripting
- Performance optimization
- Detailed HTML/JSON reports
- YARA signature integration
- Complete IAT reconstruction with multiple strategies

## Design Decisions

### 1. Modular Architecture
Each component is implemented as a separate namespace with clear initialization and cleanup functions, following the specification's recommended architecture.

### 2. Placeholder Implementations
Rather than partial implementations that might not work correctly, the MVP uses placeholder implementations that:
- Log what they would do in a full implementation
- Provide the correct structure for future development
- Allow the plugin to compile and load without errors

### 3. Safe Defaults
Following the spec's requirement (Section 5.2), anti-debugging features are disabled by default and would require explicit user activation.

### 4. Transparent Logging
All operations are logged using `_plugin_logputs` and `_plugin_logprintf` to maintain transparency, as required by the spec.

## Usage

### Installation
1. Build the x64dbg project with the plugin included
2. The plugin will be output to `bin/plugins/`
3. x64dbg will automatically load it on startup

### Basic Workflow
1. Load a packed executable in x64dbg
2. Open Plugins menu → Advanced Reconstructor
3. Click "Start Auto-Analysis" to begin analysis
4. Review OEP candidates with "OEP Candidates"
5. Use "Rebuild IAT" to reconstruct imports
6. Use "Dump & Rebuild PE" to create unpacked file

### Current Behavior (MVP)
The plugin will:
- Load successfully and add menu items
- Log all operations to x64dbg's log window
- Explain what each module would do in a full implementation
- Demonstrate the command flow and module interaction

## Legal and Ethical Notice

As specified in the spec (Section 1), this plugin is intended for:
- ✅ Legitimate reverse engineering
- ✅ Security research
- ✅ Educational purposes
- ✅ Analysis of your own software
- ✅ Authorized malware analysis

Prohibited uses:
- ❌ DRM circumvention
- ❌ Software piracy
- ❌ Unauthorized cracking
- ❌ Any illegal activities

## Future Development Roadmap

Following the spec's release roadmap (Section 8):

### Phase 1: Complete MVP Features
- Implement actual PE parsing and section analysis
- Add basic entropy calculation
- Implement function prologue scanning

### Phase 2: Beta Release
- Implement API tracing hooks
- Add W->X transition monitoring
- Implement memory dumping
- Add basic IAT reconstruction
- Create simple UI panel

### Phase 3: v1.0 Release
- YARA signature integration
- Complete IAT reconstruction with multiple strategies
- Multi-layer unpacking automation
- Full Unpack Wizard
- HTML/JSON report generation
- Scylla integration
- Performance optimization

## Technical Notes

### Dependencies
- x64dbg plugin SDK
- Windows API
- C++ Standard Library

### Build Requirements
- CMake 3.15+
- Visual Studio 2019+ (for Windows builds)
- Qt5 (for GUI components in future versions)

### API Usage
The plugin uses x64dbg's bridge API:
- `_plugin_logprintf` / `_plugin_logputs` for logging
- `_plugin_menuaddentry` for menu integration
- Callback system for debug events

## Contributing

When extending this plugin, please:
1. Maintain the modular architecture
2. Follow the specification document
3. Add comprehensive logging
4. Test with various packers (UPX, ASPack, etc.)
5. Document new features
6. Respect the legal and ethical guidelines

## References

- Main specification: `spec.md` (Korean)
- x64dbg plugin documentation: `docs/developers/plugins/basics.md`
- x64dbg repository: https://github.com/x64dbg/x64dbg

## Version History

- **v0.1 (MVP)**: Initial implementation with module structure and basic framework
  - All 6 core modules implemented as placeholders
  - Menu integration complete
  - Build system configured
  - Documentation created
