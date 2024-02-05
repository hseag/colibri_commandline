
set CMAKE="cmake.EXE"
set PWD=%CD%

rmdir /s /q out
mkdir out
mkdir out\x32
mkdir out\x64

rmdir /s /q build32
mkdir build32
%CMAKE% --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S%PWD% -B%PWD%\build32 -G "Visual Studio 16 2019" -T host=x86 -A win32
%CMAKE% --build %PWD%\build32 --config Release --target ALL_BUILD -j 10 --
xcopy build32\Release\colibri.exe %PWD%\out\x32 /s /e /y
xcopy build32\Release\libcolibri.dll %PWD%\out\x32 /s /e /y


rmdir /s /q build64
mkdir build64
%CMAKE% --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S%PWD%\ -B%PWD%\build64 -G "Visual Studio 16 2019" -T host=x86 -A x64
%CMAKE% --build %PWD%\build64 --config Release --target ALL_BUILD -j 10 --
xcopy build64\Release\colibri.exe %PWD%\out\x64 /s /e /y
xcopy build64\Release\libcolibri.dll %PWD%\out\x64 /s /e /y

set dt=%DATE:~6,4%%DATE:~3,2%%DATE:~0,2%_%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%
set dt=%dt: =0%

cd out
7z a "Colibri %VERSION%-%dt%.zip" *
cd %PWD%
