@copy .\platform\windows\makefile .\makefile
@set in_label=%1
@if not defined %in_label (set in_label=debug)
@echo building test
@if %in_label%==release (mingw32-make testrel) else (mingw32-make testdbg)
@if %in_label%==release (.\out\Windows\release\Test1.exe) else (.\out\Windows\debug\Test1.exe)