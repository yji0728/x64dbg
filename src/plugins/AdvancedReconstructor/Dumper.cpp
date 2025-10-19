#include "Dumper.h"
#include "_plugins.h"
#include "OEPFinder.h"
#include <fstream>

namespace Dumper
{
    static bool initialized = false;
    
    void Initialize()
    {
        initialized = true;
        _plugin_logputs("[Dumper] Initialized");
    }
    
    void Cleanup()
    {
        initialized = false;
        _plugin_logputs("[Dumper] Cleaned up");
    }
    
    void RebuildIAT()
    {
        if (!initialized)
            return;
            
        _plugin_logputs("[Dumper] Rebuilding IAT...");
        
        duint moduleBase = Script::Module::GetMainModuleBase();
        if (!moduleBase)
        {
            _plugin_logputs("[Dumper] No module loaded");
            return;
        }
        
        // In a full implementation, we would:
        // 1. Trace LoadLibrary/GetProcAddress calls to build address->API map
        // 2. Scan memory for API addresses
        // 3. Analyze code references to find IAT entries
        // 4. Optionally integrate with Scylla plugin
        
        _plugin_logputs("[Dumper] IAT reconstruction: This is a stub implementation");
        _plugin_logputs("[Dumper] In production, would trace API calls and rebuild IAT table");
    }
    
    void DumpAndRebuild()
    {
        if (!initialized)
            return;
            
        _plugin_logputs("[Dumper] Dumping and rebuilding PE...");
        
        duint moduleBase = Script::Module::GetMainModuleBase();
        if (!moduleBase)
        {
            _plugin_logputs("[Dumper] No module loaded");
            return;
        }
        
        // Get module path
        char modulePath[MAX_PATH];
        if (!Script::Module::GetMainModulePath(modulePath))
        {
            _plugin_logputs("[Dumper] Failed to get module path");
            return;
        }
        
        // Create output filename
        char outputPath[MAX_PATH];
        strcpy_s(outputPath, modulePath);
        strcat_s(outputPath, "_unpacked.exe");
        
        _plugin_logprintf("[Dumper] Output file: %s\n", outputPath);
        
        // Dump memory
        if (!DumpMemory(moduleBase, outputPath))
        {
            _plugin_logputs("[Dumper] Failed to dump memory");
            return;
        }
        
        // Get best OEP candidate
        auto candidates = OEPFinder::GetCandidates();
        duint oep = 0;
        if (!candidates.empty())
        {
            oep = candidates[0].address - moduleBase;  // Convert to RVA
            _plugin_logprintf("[Dumper] Using OEP RVA: 0x%x\n", (int)oep);
        }
        
        // Fix PE headers
        if (oep && FixPEHeaders(outputPath, oep))
        {
            _plugin_logputs("[Dumper] Successfully dumped and rebuilt PE!");
            _plugin_logprintf("[Dumper] Output saved to: %s\n", outputPath);
        }
        else
        {
            _plugin_logputs("[Dumper] Warning: PE headers may need manual fixing");
        }
    }
    
    bool DumpMemory(duint moduleBase, const char* outputPath)
    {
        duint moduleSize = Script::Module::GetMainModuleSize();
        if (!moduleSize)
            return false;
            
        _plugin_logprintf("[Dumper] Dumping 0x%x bytes from 0x%p\n", 
            (int)moduleSize, (void*)moduleBase);
        
        // Allocate buffer
        unsigned char* buffer = new unsigned char[moduleSize];
        if (!buffer)
            return false;
        
        // Read memory
        if (!Script::Memory::Read(moduleBase, buffer, moduleSize, nullptr))
        {
            delete[] buffer;
            return false;
        }
        
        // Write to file
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile)
        {
            delete[] buffer;
            return false;
        }
        
        outFile.write((char*)buffer, moduleSize);
        outFile.close();
        
        delete[] buffer;
        return true;
    }
    
    bool FixPEHeaders(const char* filePath, duint oepRVA)
    {
        // In a full implementation, we would:
        // 1. Parse PE headers
        // 2. Update AddressOfEntryPoint to oepRVA
        // 3. Fix section alignments
        // 4. Rebuild import directory
        // 5. Fix relocations if needed
        // 6. Update checksums
        
        _plugin_logprintf("[Dumper] Fixing PE headers (OEP RVA: 0x%x)...\n", (int)oepRVA);
        
        std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
        if (!file)
            return false;
        
        // Read DOS header
        IMAGE_DOS_HEADER dosHeader;
        file.read((char*)&dosHeader, sizeof(dosHeader));
        
        if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
            return false;
        
        // Read NT headers
        file.seekg(dosHeader.e_lfanew);
        IMAGE_NT_HEADERS ntHeaders;
        file.read((char*)&ntHeaders, sizeof(ntHeaders));
        
        if (ntHeaders.Signature != IMAGE_NT_SIGNATURE)
            return false;
        
        // Update entry point
        ntHeaders.OptionalHeader.AddressOfEntryPoint = (DWORD)oepRVA;
        
        // Write back NT headers
        file.seekp(dosHeader.e_lfanew);
        file.write((char*)&ntHeaders, sizeof(ntHeaders));
        
        file.close();
        
        _plugin_logputs("[Dumper] PE headers updated");
        return true;
    }
}
