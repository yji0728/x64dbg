@echo off

cd /d "%~dp0"

set PORTABLE_PYTHON=%~dp0python-2.7.18.amd64.portable
if not exist "%PORTABLE_PYTHON%\python.exe" (
	echo Downloading portable Python...
	curl.exe -L -O https://github.com/x64dbg/docs/releases/download/python27-portable/python-2.7.18.amd64.portable.7z
	7z x python-2.7.18.amd64.portable.7z -opython-2.7.18.amd64.portable
)

if not exist "%~dp0hhc.exe" (
	echo Downloading HTML Help Workshop...
	pause
	curl.exe -L -O https://github.com/x64dbg/deps/releases/download/dependencies/hhc-4.74.8702.7z
	7z x hhc-4.74.8702.7z
)

echo Building Help Project
call make htmlhelp
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
echo Applying CHM hacks
copy theme.js .\_build\htmlhelp\_static\js\theme.js
type hacks.css >> .\_build\htmlhelp\_static\css\theme.css
echo Building CHM File
hhc.exe .\_build\htmlhelp\x64dbgdoc.hhp
copy /Y .\_build\htmlhelp\x64dbgdoc.chm x64dbg.chm
if not exist "x64dbg.chm" (
	echo Failed to build CHM file!
	exit /b 1
)
echo Finished
