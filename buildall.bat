@copy .\platform\windows\makefile .\makefile
@mingw32-make cleanall
@set in_label=%1
@if not defined %in_label (set in_label=debug)
@if %in_label%==release (echo building release) else (echo building debug)
@if %in_label%==release (mingw32-make build) else (mingw32-make debug)
@echo building test
@if %in_label%==release (mingw32-make testrel) else (mingw32-make testdbg)
@if %in_label%==release (.\out\Windows\release\Test1.exe) else (.\out\Windows\debug\Test1.exe)