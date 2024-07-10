@if not exist CrystalCore\ (git submodule add https://github.com/RetliveAdore/CrystalCore)
@if not exist CrystalAlgorithms\ (git submodule add https://github.com/RetliveAdore/CrystalAlgorithms)
@if not exist CrystalThread\ (git submodule add https://github.com/RetliveAdore/CrystalThread)
@git submodule update --init
@git submodule update --remote
@copy .\platform\windows\makefile .\makefile
@mingw32-make build