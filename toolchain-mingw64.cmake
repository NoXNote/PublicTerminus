# Use MinGW64 from MSYS2
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER /mingw64/bin/gcc)
set(CMAKE_CXX_COMPILER /mingw64/bin/g++)
set(CMAKE_RC_COMPILER /mingw64/bin/windres)

# Specify the flags to ensure we link against MinGW runtime
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Adjust search paths
set(CMAKE_FIND_ROOT_PATH /mingw64)
