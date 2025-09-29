#include "AutoAnalysisReportPass.h"
#include "_global.h"
#include "console.h"
#include "memory.h"
#include "module.h"
#include "disasm_helper.h"
#include "symbolinfo.h"
#include <regex>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

AutoAnalysisReportPass::AutoAnalysisReportPass(duint VirtualStart, duint VirtualEnd, BBlockArray & MainBlocks)
    : AnalysisPass(VirtualStart, VirtualEnd, MainBlocks)
{
    mReport.baseAddress = VirtualStart;
    mReport.size = VirtualEnd - VirtualStart;
    mReport.analysisTime = GetCurrentTimeString();
    
    // Get module name
    SHARED_ACQUIRE(LockModules);
    auto modInfo = ModInfoFromAddr(VirtualStart);
    if(modInfo)
        mReport.moduleName = modInfo->name;
    else
        mReport.moduleName = "Unknown";
}

const char* AutoAnalysisReportPass::GetName()
{
    return "Automated Analysis Report";
}

bool AutoAnalysisReportPass::Analyse()
{
    dprintf(QT_TRANSLATE_NOOP("DBG", "Starting automated analysis report for module: %s\n"), mReport.moduleName.c_str());
    
    try
    {
        // Clear previous results
        mReport.strings.clear();
        mReport.iocs.clear();
        mReport.functionCalls.clear();
        
        // Perform analysis
        AnalyzeStrings();
        AnalyzeIOCs();  
        AnalyzeFunctionCalls();
        GatherStatistics();
        
        dprintf(QT_TRANSLATE_NOOP("DBG", "Analysis complete: %zu strings, %zu IOCs, %zu function calls found\n"), 
                mReport.totalStrings, mReport.totalIOCs, mReport.totalCalls);
        
        return true;
    }
    catch(const std::exception& e)
    {
        dprintf(QT_TRANSLATE_NOOP("DBG", "Analysis failed: %s\n"), e.what());
        return false;
    }
}

void AutoAnalysisReportPass::AnalyzeStrings()
{
    dprintf(QT_TRANSLATE_NOOP("DBG", "Analyzing strings...\n"));
    
    const size_t MIN_STRING_LENGTH = 4;
    const size_t MAX_STRING_LENGTH = 1024;
    
    for(duint addr = m_VirtualStart; addr < m_VirtualEnd - MIN_STRING_LENGTH; addr++)
    {
        if(!ValidateAddress(addr))
            continue;
            
        const unsigned char* data = TranslateAddress(addr);
        if(!data)
            continue;
            
        // Check for ASCII strings
        bool isWide = false;
        if(IsStringCandidate(data, MAX_STRING_LENGTH, isWide))
        {
            std::string value;
            size_t length = 0;
            
            if(isWide)
            {
                // Wide string (UTF-16)
                const wchar_t* wstr = reinterpret_cast<const wchar_t*>(data);
                for(size_t i = 0; i < MAX_STRING_LENGTH / 2 && wstr[i] != 0; i++)
                {
                    if(wstr[i] < 128)  // Basic ASCII range
                        value += static_cast<char>(wstr[i]);
                    else
                        value += '?';  // Non-ASCII placeholder
                    length = i + 1;
                }
            }
            else
            {
                // ASCII string
                const char* str = reinterpret_cast<const char*>(data);
                for(size_t i = 0; i < MAX_STRING_LENGTH && str[i] != 0; i++)
                {
                    value += str[i];
                    length = i + 1;
                }
            }
            
            if(length >= MIN_STRING_LENGTH && IsPrintableString(value))
            {
                StringFinding finding;
                finding.address = addr;
                finding.value = value;
                finding.length = length;
                finding.isWide = isWide;
                finding.isPrintable = true;
                
                mReport.strings.push_back(finding);
                
                // Skip ahead to avoid overlapping strings
                addr += length * (isWide ? 2 : 1) - 1;
            }
        }
    }
}

void AutoAnalysisReportPass::AnalyzeIOCs()
{
    dprintf(QT_TRANSLATE_NOOP("DBG", "Analyzing IOCs...\n"));
    
    // Search for IOCs in the found strings
    for(const auto& stringFinding : mReport.strings)
    {
        std::string iocType;
        if(IsIOCCandidate(stringFinding.value, iocType))
        {
            IOCFinding ioc;
            ioc.address = stringFinding.address;
            ioc.type = iocType;
            ioc.value = stringFinding.value;
            ioc.context = "String at " + std::to_string(stringFinding.address);
            
            mReport.iocs.push_back(ioc);
        }
    }
}

void AutoAnalysisReportPass::AnalyzeFunctionCalls()
{
    dprintf(QT_TRANSLATE_NOOP("DBG", "Analyzing function calls...\n"));
    
    std::set<duint> processedAddresses;  // Avoid duplicates
    
    // If we have basic blocks from a previous analysis, use them
    if(!m_MainBlocks.empty())
    {
        for(const auto& block : m_MainBlocks)
        {
            if(block.GetFlag(BASIC_BLOCK_FLAG_CALL))
            {
                duint callAddr = block.VirtualStart;
                duint targetAddr = block.Target;
                
                if(processedAddresses.find(callAddr) != processedAddresses.end())
                    continue;
                    
                processedAddresses.insert(callAddr);
                
                FunctionCallFinding call;
                call.address = callAddr;
                call.targetAddress = targetAddr;
                call.functionName = GetFunctionName(targetAddr);
                call.moduleName = GetModuleName(targetAddr);
                call.isImport = (call.moduleName != mReport.moduleName);
                call.isDynamic = block.GetFlag(BASIC_BLOCK_FLAG_INDIRPTR);
                
                mReport.functionCalls.push_back(call);
            }
        }
    }
    else
    {
        // Fallback: scan memory for call instructions manually
        // This is a simple heuristic approach
        for(duint addr = m_VirtualStart; addr < m_VirtualEnd - 5; addr++)
        {
            if(!ValidateAddress(addr))
                continue;
                
            const unsigned char* data = TranslateAddress(addr);
            if(!data)
                continue;
            
            // Look for common call patterns
            // E8 xx xx xx xx - direct call (32-bit relative)
            // FF 15 xx xx xx xx - call dword ptr [imm32] (indirect call)
            // FF D0-D7 - call reg (register indirect)
            // FF 10-17, 50-57, 90-97, D0-D7 - call [reg+offset] patterns
            
            if(data[0] == 0xE8)  // Direct call
            {
                if(processedAddresses.find(addr) != processedAddresses.end())
                {
                    addr += 4;  // Skip the call instruction
                    continue;
                }
                
                // Calculate target address
                int32_t offset = *reinterpret_cast<const int32_t*>(data + 1);
                duint targetAddr = addr + 5 + offset;
                
                FunctionCallFinding call;
                call.address = addr;
                call.targetAddress = targetAddr;
                call.functionName = GetFunctionName(targetAddr);
                call.moduleName = GetModuleName(targetAddr);
                call.isImport = (call.moduleName != mReport.moduleName);
                call.isDynamic = false;
                
                mReport.functionCalls.push_back(call);
                processedAddresses.insert(addr);
                addr += 4;  // Skip the call instruction
            }
            else if(data[0] == 0xFF && (data[1] & 0xF8) == 0x10)  // call [reg+offset]
            {
                if(processedAddresses.find(addr) == processedAddresses.end())
                {
                    FunctionCallFinding call;
                    call.address = addr;
                    call.targetAddress = 0;  // Unknown for indirect calls
                    call.functionName = "indirect_call";
                    call.moduleName = "Unknown";
                    call.isImport = false;
                    call.isDynamic = true;
                    
                    mReport.functionCalls.push_back(call);
                    processedAddresses.insert(addr);
                }
            }
        }
    }
}

void AutoAnalysisReportPass::GatherStatistics()
{
    mReport.totalStrings = mReport.strings.size();
    mReport.totalIOCs = mReport.iocs.size();  
    mReport.totalCalls = mReport.functionCalls.size();
    mReport.totalBlocks = m_MainBlocks.size();
    
    // Count functions and instructions
    std::set<duint> uniqueFunctions;
    mReport.totalInstructions = 0;
    
    for(const auto& block : m_MainBlocks)
    {
        mReport.totalInstructions += block.InstrCount;
        
        // Simple heuristic: blocks starting functions
        if(block.GetFlag(BASIC_BLOCK_FLAG_CALL))
            uniqueFunctions.insert(block.Target);
    }
    
    mReport.totalFunctions = uniqueFunctions.size();
}

bool AutoAnalysisReportPass::IsStringCandidate(const unsigned char* data, size_t maxLength, bool& isWide) const
{
    if(!data)
        return false;
        
    // Check for ASCII string
    size_t asciiCount = 0;
    for(size_t i = 0; i < maxLength && data[i] != 0; i++)
    {
        if(std::isprint(data[i]) || std::isspace(data[i]))
            asciiCount++;
        else
            break;
    }
    
    if(asciiCount >= 4)
    {
        isWide = false;
        return true;
    }
    
    // Check for wide string (UTF-16)
    const wchar_t* wdata = reinterpret_cast<const wchar_t*>(data);
    size_t wideCount = 0;
    for(size_t i = 0; i < maxLength/2 && wdata[i] != 0; i++)
    {
        if(wdata[i] < 256 && (std::isprint(static_cast<char>(wdata[i])) || std::isspace(static_cast<char>(wdata[i]))))
            wideCount++;
        else
            break;
    }
    
    if(wideCount >= 4)
    {
        isWide = true;
        return true;
    }
    
    return false;
}

bool AutoAnalysisReportPass::IsPrintableString(const std::string& str) const
{
    if(str.length() < 4)
        return false;
        
    size_t printableCount = 0;
    for(char c : str)
    {
        if(std::isprint(c) || std::isspace(c))
            printableCount++;
    }
    
    return (printableCount * 100 / str.length()) >= 80;  // 80% printable
}

bool AutoAnalysisReportPass::IsIOCCandidate(const std::string& str, std::string& type) const
{
    if(IsIPAddress(str))
    {
        type = "ip";
        return true;
    }
    if(IsURL(str))
    {
        type = "url";
        return true;
    }
    if(IsDomainName(str))
    {
        type = "domain";
        return true;
    }
    if(IsRegistryPath(str))
    {
        type = "registry";
        return true;
    }
    if(IsFilePath(str))
    {
        type = "file_path";
        return true;
    }
    
    return false;
}

std::string AutoAnalysisReportPass::GetFunctionName(duint address) const
{
    char labelText[MAX_LABEL_SIZE] = "";
    if(DbgGetLabelAt(address, SEG_DEFAULT, labelText))
        return std::string(labelText);
    
    char symbolName[MAX_LABEL_SIZE] = "";
    if(DbgGetSymbol(address, symbolName, nullptr))
        return std::string(symbolName);
        
    return "sub_" + std::to_string(address);
}

std::string AutoAnalysisReportPass::GetModuleName(duint address) const
{
    char moduleText[MAX_MODULE_SIZE] = "";
    if(DbgGetModuleAt(address, moduleText))
        return std::string(moduleText);
    return "Unknown";
}

std::string AutoAnalysisReportPass::GetCurrentTimeString() const
{
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool AutoAnalysisReportPass::IsIPAddress(const std::string& str) const
{
    std::regex ipRegex(R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    return std::regex_match(str, ipRegex);
}

bool AutoAnalysisReportPass::IsURL(const std::string& str) const
{
    std::regex urlRegex(R"(^https?://[^\s/$.?#].[^\s]*$)", std::regex_constants::icase);
    return std::regex_match(str, urlRegex);
}

bool AutoAnalysisReportPass::IsDomainName(const std::string& str) const
{
    std::regex domainRegex(R"(^[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)+$)");
    return str.length() > 4 && str.find('.') != std::string::npos && std::regex_match(str, domainRegex);
}

bool AutoAnalysisReportPass::IsRegistryPath(const std::string& str) const
{
    return str.find("HKEY_") == 0 || str.find("\\Registry\\") == 0 || 
           str.find("SOFTWARE\\") == 0 || str.find("SYSTEM\\") == 0;
}

bool AutoAnalysisReportPass::IsFilePath(const std::string& str) const
{
    return (str.length() > 3 && 
           ((str[1] == ':' && str[2] == '\\') ||  // C:\path
            (str.find("\\\\") == 0) ||             // \\server\share
            (str.find("%") != std::string::npos && str.find("\\") != std::string::npos)));  // %TEMP%\file
}

bool AutoAnalysisReportPass::ExportToJson(const std::string& filename) const
{
    try
    {
        std::ofstream file(filename);
        if(!file.is_open())
            return false;
            
        file << "{\n";
        file << "  \"module\": \"" << mReport.moduleName << "\",\n";
        file << "  \"baseAddress\": \"0x" << std::hex << mReport.baseAddress << "\",\n";
        file << "  \"size\": " << std::dec << mReport.size << ",\n";
        file << "  \"analysisTime\": \"" << mReport.analysisTime << "\",\n";
        
        // Statistics
        file << "  \"statistics\": {\n";
        file << "    \"totalFunctions\": " << mReport.totalFunctions << ",\n";
        file << "    \"totalBlocks\": " << mReport.totalBlocks << ",\n";
        file << "    \"totalInstructions\": " << mReport.totalInstructions << ",\n";
        file << "    \"totalStrings\": " << mReport.totalStrings << ",\n";
        file << "    \"totalIOCs\": " << mReport.totalIOCs << ",\n";
        file << "    \"totalCalls\": " << mReport.totalCalls << "\n";
        file << "  },\n";
        
        // Strings
        file << "  \"strings\": [\n";
        for(size_t i = 0; i < mReport.strings.size(); i++)
        {
            const auto& s = mReport.strings[i];
            file << "    {\n";
            file << "      \"address\": \"0x" << std::hex << s.address << "\",\n";
            file << "      \"value\": \"" << s.value << "\",\n";
            file << "      \"length\": " << std::dec << s.length << ",\n";
            file << "      \"isWide\": " << (s.isWide ? "true" : "false") << "\n";
            file << "    }" << (i < mReport.strings.size() - 1 ? "," : "") << "\n";
        }
        file << "  ],\n";
        
        // IOCs  
        file << "  \"iocs\": [\n";
        for(size_t i = 0; i < mReport.iocs.size(); i++)
        {
            const auto& ioc = mReport.iocs[i];
            file << "    {\n";
            file << "      \"address\": \"0x" << std::hex << ioc.address << "\",\n";
            file << "      \"type\": \"" << ioc.type << "\",\n";
            file << "      \"value\": \"" << ioc.value << "\",\n";
            file << "      \"context\": \"" << ioc.context << "\"\n";
            file << "    }" << (i < mReport.iocs.size() - 1 ? "," : "") << "\n";
        }
        file << "  ],\n";
        
        // Function calls
        file << "  \"functionCalls\": [\n";
        for(size_t i = 0; i < mReport.functionCalls.size(); i++)
        {
            const auto& call = mReport.functionCalls[i];
            file << "    {\n";
            file << "      \"address\": \"0x" << std::hex << call.address << "\",\n";
            file << "      \"targetAddress\": \"0x" << std::hex << call.targetAddress << "\",\n";
            file << "      \"functionName\": \"" << call.functionName << "\",\n";
            file << "      \"moduleName\": \"" << call.moduleName << "\",\n";
            file << "      \"isImport\": " << (call.isImport ? "true" : "false") << ",\n";
            file << "      \"isDynamic\": " << (call.isDynamic ? "true" : "false") << "\n";
            file << "    }" << (i < mReport.functionCalls.size() - 1 ? "," : "") << "\n";
        }
        file << "  ]\n";
        file << "}\n";
        
        return true;
    }
    catch(const std::exception&)
    {
        return false;
    }
}

bool AutoAnalysisReportPass::ExportToText(const std::string& filename) const
{
    try
    {
        std::ofstream file(filename);
        if(!file.is_open())
            return false;
            
        file << "=== Automated Malware Analysis Report ===\n\n";
        file << "Module: " << mReport.moduleName << "\n";
        file << "Base Address: 0x" << std::hex << mReport.baseAddress << "\n";
        file << "Size: " << std::dec << mReport.size << " bytes\n";
        file << "Analysis Time: " << mReport.analysisTime << "\n\n";
        
        // Statistics
        file << "=== Statistics ===\n";
        file << "Functions: " << mReport.totalFunctions << "\n";
        file << "Basic Blocks: " << mReport.totalBlocks << "\n";
        file << "Instructions: " << mReport.totalInstructions << "\n";
        file << "Strings: " << mReport.totalStrings << "\n";
        file << "IOCs: " << mReport.totalIOCs << "\n";
        file << "Function Calls: " << mReport.totalCalls << "\n\n";
        
        // Strings
        if(!mReport.strings.empty())
        {
            file << "=== Strings Found ===\n";
            for(const auto& s : mReport.strings)
            {
                file << "0x" << std::hex << s.address << " (" << (s.isWide ? "Wide" : "ASCII") 
                     << ", " << std::dec << s.length << "): " << s.value << "\n";
            }
            file << "\n";
        }
        
        // IOCs
        if(!mReport.iocs.empty())
        {
            file << "=== Indicators of Compromise (IOCs) ===\n";
            for(const auto& ioc : mReport.iocs)
            {
                file << "0x" << std::hex << ioc.address << " [" << ioc.type << "]: " 
                     << ioc.value << "\n";
            }
            file << "\n";
        }
        
        // Function calls
        if(!mReport.functionCalls.empty())
        {
            file << "=== Function Calls ===\n";
            for(const auto& call : mReport.functionCalls)
            {
                file << "0x" << std::hex << call.address << " -> " << call.functionName;
                if(call.isImport)
                    file << " [" << call.moduleName << "]";
                if(call.isDynamic)
                    file << " (dynamic)";
                file << "\n";
            }
            file << "\n";
        }
        
        return true;
    }
    catch(const std::exception&)
    {
        return false;
    }
}

bool AutoAnalysisReportPass::ExportToHtml(const std::string& filename) const
{
    try
    {
        std::ofstream file(filename);
        if(!file.is_open())
            return false;
            
        file << "<!DOCTYPE html>\n<html>\n<head>\n";
        file << "<title>Malware Analysis Report - " << mReport.moduleName << "</title>\n";
        file << "<style>\n";
        file << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
        file << "h1, h2 { color: #333; }\n";
        file << "table { border-collapse: collapse; width: 100%; margin: 10px 0; }\n";
        file << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
        file << "th { background-color: #f2f2f2; }\n";
        file << ".address { font-family: monospace; }\n";
        file << ".ioc-ip { color: red; }\n";
        file << ".ioc-url { color: blue; }\n";
        file << ".ioc-domain { color: purple; }\n";
        file << "</style>\n";
        file << "</head>\n<body>\n";
        
        file << "<h1>Automated Malware Analysis Report</h1>\n";
        file << "<p><strong>Module:</strong> " << mReport.moduleName << "</p>\n";
        file << "<p><strong>Base Address:</strong> 0x" << std::hex << mReport.baseAddress << "</p>\n";
        file << "<p><strong>Size:</strong> " << std::dec << mReport.size << " bytes</p>\n";
        file << "<p><strong>Analysis Time:</strong> " << mReport.analysisTime << "</p>\n";
        
        // Statistics table
        file << "<h2>Statistics</h2>\n<table>\n";
        file << "<tr><th>Metric</th><th>Count</th></tr>\n";
        file << "<tr><td>Functions</td><td>" << mReport.totalFunctions << "</td></tr>\n";
        file << "<tr><td>Basic Blocks</td><td>" << mReport.totalBlocks << "</td></tr>\n";
        file << "<tr><td>Instructions</td><td>" << mReport.totalInstructions << "</td></tr>\n";
        file << "<tr><td>Strings</td><td>" << mReport.totalStrings << "</td></tr>\n";
        file << "<tr><td>IOCs</td><td>" << mReport.totalIOCs << "</td></tr>\n";
        file << "<tr><td>Function Calls</td><td>" << mReport.totalCalls << "</td></tr>\n";
        file << "</table>\n";
        
        // IOCs table (most important first)
        if(!mReport.iocs.empty())
        {
            file << "<h2>Indicators of Compromise (IOCs)</h2>\n<table>\n";
            file << "<tr><th>Address</th><th>Type</th><th>Value</th></tr>\n";
            for(const auto& ioc : mReport.iocs)
            {
                file << "<tr><td class=\"address\">0x" << std::hex << ioc.address 
                     << "</td><td>" << ioc.type << "</td><td class=\"ioc-" << ioc.type 
                     << "\">" << ioc.value << "</td></tr>\n";
            }
            file << "</table>\n";
        }
        
        file << "</body>\n</html>\n";
        
        return true;
    }
    catch(const std::exception&)
    {
        return false;
    }
}