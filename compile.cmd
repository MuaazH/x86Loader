@echo off
cls
g++ -Wall -I..\mlib ..\x86assm32\x86_assm.cpp -DSYSTEM_C_NO_NETWORK ..\mlib\system.c main.cpp -static-libgcc -static-libstdc++ -o loader.exe
