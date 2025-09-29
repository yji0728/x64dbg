# autoreport / malwarereport

Generate an automated malware analysis report that extracts strings, IOCs (Indicators of Compromise), function calls, and other relevant information from a binary module.

## Arguments

`[arg1]` The base address of the module to analyze. If not specified, the module at the current selection will be used.

`[arg2]` Output format: `text`, `json`, or `html`. Default is `text`.

`[arg3]` Output filename. If not specified, a default filename will be generated based on the module name.

## Examples

```
autoreport                          // Analyze current module, output text report
autoreport 0x400000                 // Analyze module at 0x400000, output text report  
autoreport 0x400000 json            // Analyze module, output JSON report
autoreport 0x400000 html report.html // Analyze module, output HTML report to report.html
malwarereport                       // Alias for autoreport
```

## Report Contents

The automated analysis report includes:

### Statistics
- Total functions found
- Basic blocks analyzed
- Instructions processed
- Strings extracted
- IOCs identified
- Function calls discovered

### Strings
- ASCII and Unicode strings found in the binary
- Address location of each string
- String length and encoding information

### IOCs (Indicators of Compromise)
- IP addresses
- URLs and domain names
- Registry paths
- File paths
- Other suspicious patterns

### Function Calls
- Direct and indirect function calls
- Import function calls
- Target addresses and function names
- Module information for external calls

## Output Formats

### Text Format
Human-readable text format suitable for manual review.

### JSON Format
Structured JSON format suitable for automated processing and integration with other tools.

### HTML Format
Web-friendly HTML format with tables and styling for easy viewing in browsers.

## Use Cases

- Initial malware triage and classification
- Automated extraction of IOCs for threat intelligence
- Documentation of analysis findings
- Integration with malware analysis workflows
- Bulk analysis of multiple samples

## Results

This command does not set any result variables but generates a comprehensive report file in the specified format.

## Notes

- The analysis is performed on the loaded module in memory
- Large modules may take some time to analyze completely
- The quality of results depends on the completeness of the loaded module
- IOC detection uses pattern matching and may have false positives
- Function call detection works best when debugging symbols are available