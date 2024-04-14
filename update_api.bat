del /s /f /q include
del /s /f /q lib
mkdir lib
mkdir include

git submodule update --recursive --remote

cd SevenKewp

python plugin_api.py ..\include
del /s /f /q build
mkdir build
cd build
cmake -A Win32 -DBUILD_SERVER_ONLY=ON ..
cmake --build . --config Release
copy /b/v/y dlls\Release\sevenkewp.lib ..\..\lib\

cd ..\..

pause