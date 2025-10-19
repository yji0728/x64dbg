#include "OEPFinder.h"
#include "_plugins.h"
#include <algorithm>

namespace OEPFinder
{
    static bool initialized = false;
    static std::vector<OEPCandidate> candidates;
    
    void Initialize()
    {
        initialized = true;
        candidates.clear();
        _plugin_logputs("[OEPFinder] Initialized");
    }
    
    void Cleanup()
    {
        candidates.clear();
        initialized = false;
        _plugin_logputs("[OEPFinder] Cleaned up");
    }
    
    void FindOEP()
    {
        if (!initialized)
            return;
            
        candidates.clear();
        
        duint moduleBase = Script::Module::GetMainModuleBase();
        if (!moduleBase)
        {
            _plugin_logputs("[OEPFinder] No module loaded");
            return;
        }
        
        _plugin_logputs("[OEPFinder] Searching for OEP candidates...");
        
        // Strategy 1: Check for far jumps from packer stub to code section
        duint codeBase = 0;
        duint codeSize = 0;
        
        duint sectionCount = Script::Module::GetSectionCount(moduleBase);
        for (duint i = 0; i < sectionCount; i++)
        {
            char sectionName[9] = {0};
            Script::Module::GetSectionName(moduleBase, i, sectionName);
            
            // Look for .text section
            if (strcmp(sectionName, ".text") == 0)
            {
                codeBase = Script::Module::GetSectionAddr(moduleBase, i);
                codeSize = Script::Module::GetSectionSize(moduleBase, i);
                break;
            }
        }
        
        if (codeBase && codeSize)
        {
            _plugin_logprintf("[OEPFinder] .text section at 0x%p, size 0x%x\n", 
                (void*)codeBase, (int)codeSize);
            
            // Add the start of .text as a candidate
            OEPCandidate candidate;
            candidate.address = codeBase;
            candidate.confidence = 60;
            candidate.reason = "Start of .text section";
            candidates.push_back(candidate);
        }
        
        // Strategy 2: Look for common function prologues
        // In x64: push rbp; mov rbp, rsp
        // In x86: push ebp; mov ebp, esp
        
        if (codeBase && codeSize)
        {
            // Scan for function prologues (simplified)
            unsigned char* codeData = new unsigned char[codeSize];
            if (Script::Memory::Read(codeBase, codeData, codeSize, nullptr))
            {
                for (duint offset = 0; offset < codeSize - 10; offset += 1)
                {
                    // x64 prologue: 55 48 8B EC (push rbp; mov rbp, rsp)
                    if (codeData[offset] == 0x55 && codeData[offset+1] == 0x48 && 
                        codeData[offset+2] == 0x8B && codeData[offset+3] == 0xEC)
                    {
                        OEPCandidate candidate;
                        candidate.address = codeBase + offset;
                        candidate.confidence = 70;
                        candidate.reason = "Function prologue pattern";
                        candidates.push_back(candidate);
                        
                        // Only add first few to avoid too many candidates
                        if (candidates.size() >= 5)
                            break;
                    }
                }
            }
            delete[] codeData;
        }
        
        // Sort by confidence
        std::sort(candidates.begin(), candidates.end(), 
            [](const OEPCandidate& a, const OEPCandidate& b) { 
                return a.confidence > b.confidence; 
            });
        
        _plugin_logprintf("[OEPFinder] Found %d OEP candidates\n", (int)candidates.size());
    }
    
    std::vector<OEPCandidate> GetCandidates()
    {
        return candidates;
    }
    
    void ShowCandidates()
    {
        if (candidates.empty())
        {
            _plugin_logputs("[OEPFinder] No OEP candidates found. Run auto-analysis first.");
            return;
        }
        
        _plugin_logputs("[OEPFinder] OEP Candidates:");
        for (size_t i = 0; i < candidates.size(); i++)
        {
            _plugin_logprintf("  [%d] Address: 0x%p, Confidence: %d%%, Reason: %s\n",
                (int)i + 1,
                (void*)candidates[i].address,
                candidates[i].confidence,
                candidates[i].reason);
        }
        
        if (!candidates.empty())
        {
            _plugin_logprintf("[OEPFinder] Best candidate: 0x%p (confidence %d%%)\n",
                (void*)candidates[0].address,
                candidates[0].confidence);
        }
    }
}
