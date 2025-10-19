#pragma once

#include <functional>
#include <vector>
#include <string>
#include <set>
#include "AnalysisPass.h"
#include "BasicBlock.h"

// Structure to hold string findings
struct StringFinding
{
    duint address;
    std::string value;
    size_t length;
    bool isWide;
    bool isPrintable;
};

// Structure to hold IOC findings  
struct IOCFinding
{
    duint address;
    std::string type;  // "url", "ip", "domain", "registry", "file_path"
    std::string value;
    std::string context;
};

// Structure to hold function call findings
struct FunctionCallFinding
{
    duint address;
    duint targetAddress;
    std::string functionName;
    std::string moduleName;
    bool isImport;
    bool isDynamic;
};

// Structure to hold the complete analysis report
struct AnalysisReport
{
    duint baseAddress;
    duint size;
    std::string moduleName;
    std::string analysisTime;
    
    std::vector<StringFinding> strings;
    std::vector<IOCFinding> iocs;
    std::vector<FunctionCallFinding> functionCalls;
    
    // Statistics
    size_t totalFunctions;
    size_t totalBlocks;
    size_t totalInstructions;
    size_t totalStrings;
    size_t totalIOCs;
    size_t totalCalls;
};

class AutoAnalysisReportPass : public AnalysisPass
{
public:
    AutoAnalysisReportPass(duint VirtualStart, duint VirtualEnd, BBlockArray & MainBlocks);
    
    const char* GetName() override;
    bool Analyse() override;
    
    // Get the analysis report
    const AnalysisReport& GetReport() const { return mReport; }
    
    // Export report to different formats
    bool ExportToJson(const std::string& filename) const;
    bool ExportToHtml(const std::string& filename) const;
    bool ExportToText(const std::string& filename) const;

private:
    AnalysisReport mReport;
    
    // Analysis methods
    void AnalyzeStrings();
    void AnalyzeIOCs();
    void AnalyzeFunctionCalls();
    void GatherStatistics();
    
    // Helper methods
    bool IsStringCandidate(const unsigned char* data, size_t length, bool& isWide) const;
    bool IsPrintableString(const std::string& str) const;
    bool IsIOCCandidate(const std::string& str, std::string& type) const;
    std::string GetFunctionName(duint address) const;
    std::string GetModuleName(duint address) const;
    std::string GetCurrentTimeString() const;
    
    // Pattern matching for IOCs
    bool IsIPAddress(const std::string& str) const;
    bool IsURL(const std::string& str) const;
    bool IsDomainName(const std::string& str) const;
    bool IsRegistryPath(const std::string& str) const;
    bool IsFilePath(const std::string& str) const;
};