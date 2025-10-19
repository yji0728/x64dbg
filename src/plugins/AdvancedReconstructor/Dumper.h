#ifndef DUMPER_H
#define DUMPER_H

#include <Windows.h>

namespace Dumper
{
    // Initialize the dumper
    void Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Rebuild Import Address Table
    void RebuildIAT();
    
    // Dump and rebuild PE file
    void DumpAndRebuild();
    
    // Dump memory to file
    bool DumpMemory(duint moduleBase, const char* outputPath);
    
    // Fix PE headers
    bool FixPEHeaders(const char* filePath, duint oepRVA);
}

#endif // DUMPER_H
