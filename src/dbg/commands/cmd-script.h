#pragma once

#include "command.h"

bool cbScriptLoad(int argc, char* argv[]);
bool cbScriptMsg(int argc, char* argv[]);
bool cbScriptMsgyn(int argc, char* argv[]);
bool cbScriptCmd(int argc, char* argv[]);
bool cbScriptRun(int argc, char* argv[]);
bool cbScriptExec(int argc, char* argv[]);
bool cbInstrLog(int argc, char* argv[]);
bool cbInstrHtmlLog(int argc, char* argv[]);
bool cbInstrPrintStack(int argc, char* argv[]);