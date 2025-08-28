#pragma once

#include <windows.h>
#include <stdint.h>

#pragma pack(push, 1)

// Global.Constant.Structure.Declaration:
// Engine.External:
#define UE_STRUCT_TITAN_ENGINE_CONTEXT 16

#define UE_ACCESS_READ 0
//#define UE_ACCESS_WRITE 1
#define UE_ACCESS_ALL 2

#define UE_HIDE_PEBONLY 0

#define UE_ENGINE_SET_DEBUG_PRIVILEGE 9
#define UE_ENGINE_SAFE_ATTACH 10
#define UE_ENGINE_MEMBP_ALT 11
#define UE_ENGINE_DISABLE_ASLR 12
#define UE_ENGINE_SAFE_STEP 13

#define UE_OPTION_REMOVEALL 1

#define UE_CH_CREATETHREAD 14
#define UE_CH_EXITTHREAD 15
#define UE_CH_CREATEPROCESS 16
#define UE_CH_EXITPROCESS 17
#define UE_CH_LOADDLL 18
#define UE_CH_UNLOADDLL 19
#define UE_CH_OUTPUTDEBUGSTRING 20
#define UE_CH_SYSTEMBREAKPOINT 23
#define UE_CH_UNHANDLEDEXCEPTION 24
#define UE_CH_DEBUGEVENT 26

#define UE_BREAKPOINT_INT3 1
#define UE_BREAKPOINT_LONG_INT3 2
#define UE_BREAKPOINT_UD2 3

#define UE_BREAKPOINT 0
#define UE_SINGLESHOOT 1
#define UE_HARDWARE 2
#define UE_MEMORY 3
#define UE_MEMORY_READ 4
#define UE_MEMORY_WRITE 5
#define UE_MEMORY_EXECUTE 6
#define UE_BREAKPOINT_TYPE_INT3 0x10000000
#define UE_BREAKPOINT_TYPE_LONG_INT3 0x20000000
#define UE_BREAKPOINT_TYPE_UD2 0x30000000

#define UE_HARDWARE_EXECUTE 4
#define UE_HARDWARE_WRITE 5
#define UE_HARDWARE_READWRITE 6

#define UE_HARDWARE_SIZE_1 7
#define UE_HARDWARE_SIZE_2 8
#define UE_HARDWARE_SIZE_4 9
#define UE_HARDWARE_SIZE_8 10

#define UE_EAX 1
#define UE_EBX 2
#define UE_ECX 3
#define UE_EDX 4
#define UE_EDI 5
#define UE_ESI 6
#define UE_EBP 7
#define UE_ESP 8
#define UE_EIP 9
#define UE_EFLAGS 10
#define UE_DR0 11
#define UE_DR1 12
#define UE_DR2 13
#define UE_DR3 14
#define UE_DR6 15
#define UE_DR7 16
#define UE_RAX 17
#define UE_RBX 18
#define UE_RCX 19
#define UE_RDX 20
#define UE_RDI 21
#define UE_RSI 22
#define UE_RBP 23
#define UE_RSP 24
#define UE_RIP 25
#define UE_RFLAGS 26
#define UE_R8 27
#define UE_R9 28
#define UE_R10 29
#define UE_R11 30
#define UE_R12 31
#define UE_R13 32
#define UE_R14 33
#define UE_R15 34
#define UE_CIP 35
#define UE_CSP 36
#ifdef _WIN64
#define UE_CFLAGS UE_RFLAGS
#else
#define UE_CFLAGS UE_EFLAGS
#endif
#define UE_SEG_GS 37
#define UE_SEG_FS 38
#define UE_SEG_ES 39
#define UE_SEG_DS 40
#define UE_SEG_CS 41
#define UE_SEG_SS 42
#define UE_x87_r0 43
#define UE_x87_r1 44
#define UE_x87_r2 45
#define UE_x87_r3 46
#define UE_x87_r4 47
#define UE_x87_r5 48
#define UE_x87_r6 49
#define UE_x87_r7 50
#define UE_X87_STATUSWORD 51
#define UE_X87_CONTROLWORD 52
#define UE_X87_TAGWORD 53
#define UE_MXCSR 54
#define UE_MMX0 55
#define UE_MMX1 56
#define UE_MMX2 57
#define UE_MMX3 58
#define UE_MMX4 59
#define UE_MMX5 60
#define UE_MMX6 61
#define UE_MMX7 62
#define UE_XMM0 63
#define UE_XMM1 64
#define UE_XMM2 65
#define UE_XMM3 66
#define UE_XMM4 67
#define UE_XMM5 68
#define UE_XMM6 69
#define UE_XMM7 70
#define UE_XMM8 71
#define UE_XMM9 72
#define UE_XMM10 73
#define UE_XMM11 74
#define UE_XMM12 75
#define UE_XMM13 76
#define UE_XMM14 77
#define UE_XMM15 78
#define UE_x87_ST0 79
#define UE_x87_ST1 80
#define UE_x87_ST2 81
#define UE_x87_ST3 82
#define UE_x87_ST4 83
#define UE_x87_ST5 84
#define UE_x87_ST6 85
#define UE_x87_ST7 86
#define UE_YMM0 87
#define UE_YMM1 88
#define UE_YMM2 89
#define UE_YMM3 90
#define UE_YMM4 91
#define UE_YMM5 92
#define UE_YMM6 93
#define UE_YMM7 94
#define UE_YMM8 95
#define UE_YMM9 96
#define UE_YMM10 97
#define UE_YMM11 98
#define UE_YMM12 99
#define UE_YMM13 100
#define UE_YMM14 101
#define UE_YMM15 102

typedef void(*TITANCALLBACKARG)(const void*);
typedef void(*TITANCALLBACK)();

typedef TITANCALLBACK TITANCBCH;
typedef TITANCALLBACK TITANCBSTEP;
typedef TITANCALLBACK TITANCBSOFTBP;
typedef TITANCALLBACKARG TITANCBHWBP;
typedef TITANCALLBACKARG TITANCBMEMBP;

typedef struct DECLSPEC_ALIGN(16) _XmmRegister_t
{
    ULONGLONG Low;
    LONGLONG High;
} XmmRegister_t;

typedef struct
{
    XmmRegister_t Low; //XMM/SSE part
    XmmRegister_t High; //AVX part
} YmmRegister_t;

typedef struct
{
    YmmRegister_t Low; //AVX part
    YmmRegister_t High; //AVX-512 part
} ZmmRegister_t;

typedef struct
{
    BYTE    data[10];
    int     st_value;
    int     tag;
} x87FPURegister_t;

typedef struct
{
    WORD   ControlWord;
    WORD   StatusWord;
    WORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    DWORD   Cr0NpxState;
} x87FPU_t;

typedef struct
{
    ULONG_PTR cax;
    ULONG_PTR ccx;
    ULONG_PTR cdx;
    ULONG_PTR cbx;
    ULONG_PTR csp;
    ULONG_PTR cbp;
    ULONG_PTR csi;
    ULONG_PTR cdi;
#ifdef _WIN64
    ULONG_PTR r8;
    ULONG_PTR r9;
    ULONG_PTR r10;
    ULONG_PTR r11;
    ULONG_PTR r12;
    ULONG_PTR r13;
    ULONG_PTR r14;
    ULONG_PTR r15;
#endif //_WIN64
    ULONG_PTR cip;
    ULONG_PTR eflags;
    unsigned short gs;
    unsigned short fs;
    unsigned short es;
    unsigned short ds;
    unsigned short cs;
    unsigned short ss;
    ULONG_PTR dr0;
    ULONG_PTR dr1;
    ULONG_PTR dr2;
    ULONG_PTR dr3;
    ULONG_PTR dr6;
    ULONG_PTR dr7;
    BYTE RegisterArea[80];
    x87FPU_t x87fpu;
    DWORD MxCsr;
#ifdef _WIN64
    XmmRegister_t XmmRegisters[16];
    YmmRegister_t YmmRegisters[16];
#else // x86
    XmmRegister_t XmmRegisters[8];
    YmmRegister_t YmmRegisters[8];
#endif
} TITAN_ENGINE_CONTEXT_t;

typedef struct
{
#ifdef _WIN64
    ZmmRegister_t ZmmRegisters[32];
#else // x86
    ZmmRegister_t ZmmRegisters[8];
#endif
    ULONGLONG Opmask[8];
} TITAN_ENGINE_CONTEXT_AVX512_t;

#ifdef __cplusplus
extern "C"
{
#endif

// Global.Function.Declaration:
// TitanEngine.Dumper.functions:
__declspec(dllexport) ULONG_PTR ConvertVAtoFileOffsetEx(ULONG_PTR FileMapVA, DWORD FileSize, ULONG_PTR ImageBase, ULONG_PTR AddressToConvert, bool AddressIsRVA, bool ReturnType); // TODO: remove
__declspec(dllexport) ULONG_PTR ConvertFileOffsetToVA(ULONG_PTR FileMapVA, ULONG_PTR AddressToConvert, bool ReturnType); // TODO: remove
__declspec(dllexport) bool MemoryReadSafe(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);
__declspec(dllexport) bool MemoryWriteSafe(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
// TitanEngine.Hider.functions:
__declspec(dllexport) void* GetPEBLocation(HANDLE hProcess);
__declspec(dllexport) void* GetTEBLocation(HANDLE hThread);
__declspec(dllexport) bool HideDebugger(HANDLE hProcess, DWORD PatchAPILevel); // TODO: remove
// TitanEngine.Debugger.functions:
__declspec(dllexport) void* InitDebugW(const wchar_t* szFileName, const wchar_t* szCommandLine, const wchar_t* szCurrentFolder);
__declspec(dllexport) bool StopDebug();
__declspec(dllexport) void SetBPXOptions(long DefaultBreakPointType);
__declspec(dllexport) bool IsBPXEnabled(ULONG_PTR bpxAddress);
__declspec(dllexport) bool SetBPX(ULONG_PTR bpxAddress, DWORD bpxType, TITANCBSOFTBP bpxCallBack);
__declspec(dllexport) bool DeleteBPX(ULONG_PTR bpxAddress);
__declspec(dllexport) bool SetMemoryBPXEx(ULONG_PTR MemoryStart, SIZE_T SizeOfMemory, DWORD BreakPointType, bool RestoreOnHit, TITANCBMEMBP bpxCallBack);
__declspec(dllexport) bool RemoveMemoryBPX(ULONG_PTR MemoryStart, SIZE_T SizeOfMemory);
__declspec(dllexport) bool GetFullContextDataEx(HANDLE hActiveThread, TITAN_ENGINE_CONTEXT_t* titcontext);
__declspec(dllexport) bool SetFullContextDataEx(HANDLE hActiveThread, TITAN_ENGINE_CONTEXT_t* titcontext);
__declspec(dllexport) ULONG_PTR GetContextDataEx(HANDLE hActiveThread, DWORD IndexOfRegister);
__declspec(dllexport) bool SetContextDataEx(HANDLE hActiveThread, DWORD IndexOfRegister, ULONG_PTR NewRegisterValue);
__declspec(dllexport) bool GetAVXContext(HANDLE hActiveThread, TITAN_ENGINE_CONTEXT_t* titcontext);
__declspec(dllexport) bool SetAVXContext(HANDLE hActiveThread, TITAN_ENGINE_CONTEXT_t* titcontext);
__declspec(dllexport) bool GetAVX512Context(HANDLE hActiveThread, TITAN_ENGINE_CONTEXT_AVX512_t* titcontext);
__declspec(dllexport) bool SetAVX512Context(HANDLE hActiveThread, TITAN_ENGINE_CONTEXT_AVX512_t* titcontext);
__declspec(dllexport) bool Fill(LPVOID MemoryStart, DWORD MemorySize, PBYTE FillByte); // TODO: remove
__declspec(dllexport) void* GetDebugData(); // TODO: remove?
__declspec(dllexport) void SetCustomHandler(DWORD ExceptionId, TITANCBCH CallBack);
__declspec(dllexport) void StepInto(TITANCBSTEP traceCallBack);
__declspec(dllexport) void StepOver(TITANCBSTEP traceCallBack);
__declspec(dllexport) bool GetUnusedHardwareBreakPointRegister(LPDWORD RegisterIndex);
__declspec(dllexport) bool SetHardwareBreakPoint(ULONG_PTR bpxAddress, DWORD IndexOfRegister, DWORD bpxType, DWORD bpxSize, TITANCBHWBP bpxCallBack);
__declspec(dllexport) bool DeleteHardwareBreakPoint(DWORD IndexOfRegister);
__declspec(dllexport) bool RemoveAllBreakPoints(DWORD RemoveOption);
__declspec(dllexport) void DebugLoop();
__declspec(dllexport) void SetNextDbgContinueStatus(DWORD SetDbgCode);
__declspec(dllexport) bool AttachDebugger(DWORD ProcessId, bool KillOnExit, LPVOID DebugInfo, TITANCALLBACK CallBack);
__declspec(dllexport) bool DetachDebuggerEx(DWORD ProcessId);
__declspec(dllexport) bool IsFileBeingDebugged();
// TitanEngine.Process.functions:
__declspec(dllexport) HANDLE TitanOpenProcess(DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwProcessId);
__declspec(dllexport) HANDLE TitanOpenThread(DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwThreadId);
// TitanEngine.StaticUnpacker.functions:
__declspec(dllexport) bool StaticFileLoadW(const wchar_t* szFileName, DWORD DesiredAccess, bool SimulateLoad, LPHANDLE FileHandle, LPDWORD LoadedSize, LPHANDLE FileMap, PULONG_PTR FileMapVA); // TODO: remove
__declspec(dllexport) bool StaticFileUnloadW(const wchar_t* szFileName, bool CommitChanges, HANDLE FileHandle, DWORD LoadedSize, HANDLE FileMap, ULONG_PTR FileMapVA); // TODO: remove
// TitanEngine.Engine.functions:
__declspec(dllexport) void SetEngineVariable(DWORD VariableId, bool VariableSet);
__declspec(dllexport) bool EngineCheckStructAlignment(DWORD StructureType, ULONG_PTR StructureSize);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
