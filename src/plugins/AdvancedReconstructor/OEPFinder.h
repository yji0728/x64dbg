#ifndef OEP_FINDER_H
#define OEP_FINDER_H

#include <Windows.h>
#include <vector>

namespace OEPFinder
{
    struct OEPCandidate
    {
        duint address;
        int confidence;     // 0-100
        const char* reason;
    };
    
    // Initialize the OEP finder
    void Initialize();
    
    // Cleanup resources
    void Cleanup();
    
    // Find OEP candidates
    void FindOEP();
    
    // Get list of OEP candidates
    std::vector<OEPCandidate> GetCandidates();
    
    // Show OEP candidates UI
    void ShowCandidates();
}

#endif // OEP_FINDER_H
