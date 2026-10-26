@echo off
rem Run from a VS x64 developer prompt. Static runtime so it loads under Proton.
setlocal
set ROOT=%~dp0
if not exist "%ROOT%build" mkdir "%ROOT%build"
cl /nologo /O2 /MT /LD /std:c++17 /EHsc /I"%ROOT%third_party\include" /Fo"%ROOT%build\\" /Fe"%ROOT%build\StealthDiva.dll" "%ROOT%src\dllmain.cpp" "%ROOT%src\diva.cpp" "%ROOT%src\scene_skip.cpp" "%ROOT%src\loading_bg.cpp" /link /LIBPATH:"%ROOT%third_party\lib" detours.lib user32.lib

if errorlevel 1 exit /b 1
if not exist "%ROOT%release\StealthDiva" mkdir "%ROOT%release\StealthDiva"
copy /y "%ROOT%build\StealthDiva.dll" "%ROOT%release\StealthDiva\" >nul
copy /y "%ROOT%config.toml" "%ROOT%release\StealthDiva\" >nul
