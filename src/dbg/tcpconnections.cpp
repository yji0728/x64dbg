#include "tcpconnections.h"
#include <WS2tcpip.h>

#if (_WIN32_WINNT >= 0x0600)
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#endif // _WIN32_WINNT >= 0x0600

static const char* TcpStateToString(unsigned int State)
{
    switch(State)
    {
    case MIB_TCP_STATE_CLOSED:
        return "CLOSED";
    case MIB_TCP_STATE_LISTEN:
        return "LISTEN";
    case MIB_TCP_STATE_SYN_SENT:
        return "SYN-SENT";
    case MIB_TCP_STATE_SYN_RCVD:
        return "SYN-RECEIVED";
    case MIB_TCP_STATE_ESTAB:
        return "ESTABLISHED";
    case MIB_TCP_STATE_FIN_WAIT1:
        return "FIN-WAIT-1";
    case MIB_TCP_STATE_FIN_WAIT2:
        return "FIN-WAIT-2";
    case MIB_TCP_STATE_CLOSE_WAIT:
        return "CLOSE-WAIT";
    case MIB_TCP_STATE_CLOSING:
        return "CLOSING";
    case MIB_TCP_STATE_LAST_ACK:
        return "LAST-ACK";
    case MIB_TCP_STATE_TIME_WAIT:
        return "TIME-WAIT";
    case MIB_TCP_STATE_DELETE_TCB:
        return "DELETE-TCB";
    default:
        return "UNKNOWN";
    }
}

bool TcpEnumConnections(duint pid, std::vector<TCPCONNECTIONINFO> & connections)
{
#if (_WIN32_WINNT >= 0x0600)
    TCPCONNECTIONINFO info;
    wchar_t AddrBuffer[TCP_ADDR_SIZE] = L"";
    ULONG ulSize = 0;
    // Make an initial call to GetTcpTable2 to get the necessary size into the ulSize variable
    if(GetTcpTable2(nullptr, &ulSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
    {
        Memory<MIB_TCPTABLE2*> pTcpTable(ulSize);
        // Make a second call to GetTcpTable2 to get the actual data we require
        if(GetTcpTable2(pTcpTable(), &ulSize, TRUE) == NO_ERROR)
        {
            for(auto i = 0; i < int(pTcpTable()->dwNumEntries); i++)
            {
                auto & entry = pTcpTable()->table[i];
                if(entry.dwOwningPid != pid)
                    continue;

                info.State = entry.dwState;
                strcpy_s(info.StateText, TcpStateToString(info.State));

                struct in_addr IpAddr;
                IpAddr.S_un.S_addr = u_long(entry.dwLocalAddr);
                InetNtopW(AF_INET, &IpAddr, AddrBuffer, TCP_ADDR_SIZE);
                strcpy_s(info.LocalAddress, StringUtils::Utf16ToUtf8(AddrBuffer).c_str());
                info.LocalPort = ntohs(u_short(entry.dwLocalPort));

                IpAddr.S_un.S_addr = u_long(entry.dwRemoteAddr);
                InetNtopW(AF_INET, &IpAddr, AddrBuffer, TCP_ADDR_SIZE);
                strcpy_s(info.RemoteAddress, StringUtils::Utf16ToUtf8(AddrBuffer).c_str());
                info.RemotePort = ntohs(u_short(entry.dwRemotePort));

                connections.push_back(info);
            }
        }
    }
    ulSize = 0;
    // Make an initial call to GetTcp6Table2 to get the necessary size into the ulSize variable
    if(GetTcp6Table2(nullptr, &ulSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
    {
        Memory<MIB_TCP6TABLE2*> pTcp6Table(ulSize);
        // Make a second call to GetTcpTable2 to get the actual data we require
        if(GetTcp6Table2(pTcp6Table(), &ulSize, TRUE) == NO_ERROR)
        {
            for(auto i = 0; i < int(pTcp6Table()->dwNumEntries); i++)
            {
                auto & entry = pTcp6Table()->table[i];
                if(entry.dwOwningPid != pid)
                    continue;

                info.State = entry.State;
                strcpy_s(info.StateText, TcpStateToString(info.State));

                InetNtopW(AF_INET6, &entry.LocalAddr, AddrBuffer, TCP_ADDR_SIZE);
                sprintf_s(info.LocalAddress, "[%s]", StringUtils::Utf16ToUtf8(AddrBuffer).c_str());
                info.LocalPort = ntohs(u_short(entry.dwLocalPort));

                InetNtopW(AF_INET6, &entry.RemoteAddr, AddrBuffer, TCP_ADDR_SIZE);
                sprintf_s(info.RemoteAddress, "[%s]", StringUtils::Utf16ToUtf8(AddrBuffer).c_str());
                info.RemotePort = ntohs(u_short(entry.dwRemotePort));

                connections.push_back(info);
            }
        }
    }
#else
    // This feature requires Windows Vista or greater, so don't compile on Windows XP.
#endif // _WIN32_WINNT >= 0x0600
    return true;
}
