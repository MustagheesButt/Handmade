MACOS_LD_FLAGS="-framework AppKit"
mkdir build

pushd build
clang -g $MACOS_LD_FLAGS -o handmade ../macos_handmade.mm
popd
