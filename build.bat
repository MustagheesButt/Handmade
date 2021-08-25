@echo off

mkdir build
pushd build
cl -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -FC -Zi h:\win32_handmade.cpp user32.lib gdi32.lib
popd