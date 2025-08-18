@echo off

cl /nologo /O2 /I ..\include example.c ..\src\enc.c ..\src\dec.c /Fe:fox.exe
