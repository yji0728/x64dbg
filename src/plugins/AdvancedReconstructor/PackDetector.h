#ifndef PACK_DETECTOR_H
#define PACK_DETECTOR_H

#include <Windows.h>

namespace PackDetector
{
    // Initialize the pack detector
    void Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Analyze the currently loaded module for packing
    void AnalyzeCurrentModule();
    
    // Check if a module is packed
    bool IsModulePacked(duint moduleBase);
    
    // Get packing suspicion score (0-100)
    int GetSuspicionScore(duint moduleBase);
    
    // Get estimated packer type
    const char* GetPackerType(duint moduleBase);
}

#endif // PACK_DETECTOR_H
