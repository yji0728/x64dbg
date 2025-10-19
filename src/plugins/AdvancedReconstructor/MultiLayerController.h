#ifndef MULTI_LAYER_CONTROLLER_H
#define MULTI_LAYER_CONTROLLER_H

#include <Windows.h>

namespace MultiLayerController
{
    // Initialize the multi-layer controller
    void Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Start multi-layer unpacking process
    void StartUnpacking();
    
    // Get current unpacking layer
    int GetCurrentLayer();
    
    // Get maximum layers to unpack
    int GetMaxLayers();
    
    // Set maximum layers
    void SetMaxLayers(int maxLayers);
}

#endif // MULTI_LAYER_CONTROLLER_H
