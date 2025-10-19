#include "RuntimeInstrumentor.h"
#include "_plugins.h"
#include <map>
#include <vector>

namespace RuntimeInstrumentor
{
    static bool initialized = false;
    static bool tracing = false;
    static std::map<duint, DWORD> memoryProtections;
    static std::vector<duint> writableToExecutable;
    
    void Initialize()
    {
        initialized = true;
        tracing = false;
        _plugin_logputs("[RuntimeInstrumentor] Initialized");
    }
    
    void Cleanup()
    {
        StopTracing();
        memoryProtections.clear();
        writableToExecutable.clear();
        initialized = false;
        _plugin_logputs("[RuntimeInstrumentor] Cleaned up");
    }
    
    void StartTracing()
    {
        if (!initialized)
            return;
            
        tracing = true;
        _plugin_logputs("[RuntimeInstrumentor] Started API tracing");
        
        // In a full implementation, we would:
        // - Set breakpoints on VirtualAlloc, VirtualProtect, etc.
        // - Hook LoadLibrary, GetProcAddress
        // - Monitor memory page protection changes
    }
    
    void StopTracing()
    {
        if (!tracing)
            return;
            
        tracing = false;
        _plugin_logputs("[RuntimeInstrumentor] Stopped API tracing");
    }
    
    void OnDebugEvent(PLUG_CB_DEBUGEVENT* info)
    {
        if (!tracing)
            return;
            
        // Handle various debug events
        // In a full implementation, we would track:
        // - Process creation/exit
        // - Thread creation/exit
        // - DLL load/unload
        // - Breakpoint hits (especially on hooked APIs)
    }
    
    void OnException(PLUG_CB_EXCEPTION* info)
    {
        if (!tracing)
            return;
            
        // Log exception information
        _plugin_logprintf("[RuntimeInstrumentor] Exception at 0x%p, code: 0x%x\n",
            (void*)info->Exception->ExceptionAddress,
            info->Exception->ExceptionCode);
    }
    
    bool CheckMemoryTransition(duint address)
    {
        // Check if this memory region changed from writable to executable
        // This is a key indicator of unpacking
        
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(DbgGetProcessHandle(), (LPCVOID)address, &mbi, sizeof(mbi)))
        {
            DWORD oldProtect = 0;
            if (memoryProtections.find(address) != memoryProtections.end())
            {
                oldProtect = memoryProtections[address];
            }
            
            // Check for W->X transition
            bool wasWritable = (oldProtect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
            bool isExecutable = (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
            bool wasNotExecutable = (oldProtect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) == 0;
            
            if (wasWritable && wasNotExecutable && isExecutable)
            {
                _plugin_logprintf("[RuntimeInstrumentor] W->X transition detected at 0x%p\n", (void*)address);
                writableToExecutable.push_back(address);
                return true;
            }
            
            // Update stored protection
            memoryProtections[address] = mbi.Protect;
        }
        
        return false;
    }
}
