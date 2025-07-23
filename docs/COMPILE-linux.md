## Prerequisites
```sh
sudo apt update
sudo apt install cmake ninja-build wine
```

## Install MSVC
Download https://github.com/mstorsjo/msvc-wine
```sh
cd msvc-wine
./vsdownload.py --accept-license --dest ~/opt/msvc Microsoft.VisualStudio.Workload.VCTools Microsoft.VisualStudio.Component.VC.ATL
./install.sh ~/opt/msvc
```

## Build
```sh
cd x64dbg
export MSVC_BIN_DIR=~/opt/msvc/bin/x86
export QT_BIN_DIR=~/src/x64dbg/build32/_deps/qt5-src/bin
cmake -B build32 -DCMAKE_TOOLCHAIN_FILE=cmake/msvc-wine.cmake -G Ninja
cmake --build build32 -j4
```

## Issues
```LINK : fatal error LNK1158: cannot run 'rc.exe'```
Fix: winecfg -> Drives -> Remove non default drives (stay only: C and Z drives).

