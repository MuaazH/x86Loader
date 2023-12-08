@echo off
cls
g++ -c -m32 -Wall ..\c_lib\memory.c                              -o obj\memory.o
g++ -c -m32 -Wall ..\c_lib\c_strings.c                           -o obj\c_strings.o
g++ -c -m32 -Wall ..\c_lib\io.c                                  -o obj\io.o
g++ -c -m32 -Wall -I..\mlib -I..\c_lib ..\x86assm32\x86_assm.cpp -o obj\x86_asm.o
g++ -c -m32 -Wall -I..\mlib -I..\c_lib main.cpp                  -o obj\main.o
g++ -m32 -static-libgcc -static-libstdc++ obj\main.o obj\memory.o obj\c_strings.o obj\io.o obj\x86_asm.o -o loader.exe
