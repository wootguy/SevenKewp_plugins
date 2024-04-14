@echo off
cls

if not exist include\ (
  call update_api.bat
)

mkdir build
cd build
cmake -A Win32 ..
cmake --build . --config Release

mkdir dlls
copy "maps\pizza_ya_san\Release\*.dll" "dlls\*"

echo.
pause