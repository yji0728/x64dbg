#include "plugin.h"
#include "_plugins.h"
#include "PackDetector.h"
#include "RuntimeInstrumentor.h"
#include "OEPFinder.h"
#include "Dumper.h"
#include "MultiLayerController.h"

// Global variables
int hMenu = 0;
int hMenuDisasm = 0;
int hMenuDump = 0;
int pluginHandle = 0;
HWND hwndDlg = 0;

// Plugin initialization
extern "C" __declspec(dllexport) bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = PLUGIN_VERSION;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy_s(initStruct->pluginName, PLUGIN_NAME);
    pluginHandle = initStruct->pluginHandle;

    // Initialize modules
    PackDetector::Initialize();
    RuntimeInstrumentor::Initialize();
    OEPFinder::Initialize();
    Dumper::Initialize();
    MultiLayerController::Initialize();

    _plugin_logputs("[AdvancedReconstructor] Plugin initialized");
    return true;
}

// Plugin stop
extern "C" __declspec(dllexport) bool plugstop()
{
    // Cleanup modules
    MultiLayerController::Cleanup();
    Dumper::Cleanup();
    OEPFinder::Cleanup();
    RuntimeInstrumentor::Cleanup();
    PackDetector::Cleanup();

    _plugin_logputs("[AdvancedReconstructor] Plugin stopped");
    return true;
}

// Plugin setup
extern "C" __declspec(dllexport) void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    hwndDlg = setupStruct->hwndDlg;
    
    // Add menu items
    _plugin_menuaddentry(setupStruct->hMenu, 0, "&Start Auto-Analysis");
    _plugin_menuaddentry(setupStruct->hMenu, 1, "&OEP Candidates");
    _plugin_menuaddentry(setupStruct->hMenu, 2, "&Rebuild IAT");
    _plugin_menuaddentry(setupStruct->hMenu, 3, "&Dump && Rebuild PE");
    _plugin_menuaddseparator(setupStruct->hMenu);
    _plugin_menuaddentry(setupStruct->hMenu, 4, "A&bout");

    _plugin_logputs("[AdvancedReconstructor] Plugin setup complete");
}

// Menu entry callback
extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch(info->hEntry)
    {
    case 0: // Start Auto-Analysis
        _plugin_logputs("[AdvancedReconstructor] Starting auto-analysis...");
        PackDetector::AnalyzeCurrentModule();
        RuntimeInstrumentor::StartTracing();
        OEPFinder::FindOEP();
        break;
        
    case 1: // OEP Candidates
        _plugin_logputs("[AdvancedReconstructor] Showing OEP candidates...");
        OEPFinder::ShowCandidates();
        break;
        
    case 2: // Rebuild IAT
        _plugin_logputs("[AdvancedReconstructor] Rebuilding IAT...");
        Dumper::RebuildIAT();
        break;
        
    case 3: // Dump & Rebuild PE
        _plugin_logputs("[AdvancedReconstructor] Dumping and rebuilding PE...");
        Dumper::DumpAndRebuild();
        break;
        
    case 4: // About
        _plugin_logputs("[AdvancedReconstructor] Advanced Reconstructor v1.0 - Advanced unpacking plugin for x64dbg");
        MessageBoxA(hwndDlg, 
            "Advanced Reconstructor v1.0\n\n"
            "An advanced unpacking plugin for x64dbg that provides:\n"
            "- Packing detection (static/dynamic)\n"
            "- Runtime instrumentation and API tracing\n"
            "- OEP (Original Entry Point) detection\n"
            "- Memory dumping and PE reconstruction\n"
            "- Multi-layer unpacking support\n\n"
            "For legitimate reverse engineering purposes only.",
            "About Advanced Reconstructor",
            MB_ICONINFORMATION);
        break;
    }
}

// Debug event callback
extern "C" __declspec(dllexport) void CBDEBUGEVENT(CBTYPE cbType, PLUG_CB_DEBUGEVENT* info)
{
    RuntimeInstrumentor::OnDebugEvent(info);
}

// Exception callback
extern "C" __declspec(dllexport) void CBEXCEPTION(CBTYPE cbType, PLUG_CB_EXCEPTION* info)
{
    RuntimeInstrumentor::OnException(info);
}
