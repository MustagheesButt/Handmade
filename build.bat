@echo off

set CommonCompilerFlags=-MT -Gm- -nologo -GR- -EHa -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -FC -Z7 -Fm
set CommonLinkerFlags=-opt:ref user32.lib gdi32.lib

mkdir build
pushd build
REM cl %CommonCompilerFlags% h:\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
cl %CommonCompilerFlags% h:\win32_handmade.cpp /link %CommonLinkerFlags%
popd