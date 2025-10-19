#ifndef RUNTIME_INSTRUMENTOR_H
#define RUNTIME_INSTRUMENTOR_H

#include "_plugin_types.h"
#include <Windows.h>

namespace RuntimeInstrumentor
{
    // Initialize the runtime instrumentor
    void Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Start tracing API calls and memory changes
    void StartTracing();
    
    // Stop tracing
    void StopTracing();
    
    // Handle debug events
    void OnDebugEvent(PLUG_CB_DEBUGEVENT* info);
    
    // Handle exceptions
    void OnException(PLUG_CB_EXCEPTION* info);
    
    // Check if a memory region transitioned from W to X
    bool CheckMemoryTransition(duint address);
}

#endif // RUNTIME_INSTRUMENTOR_H
