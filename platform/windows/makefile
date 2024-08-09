INCLUDE_PATHS := .\include
INCLUDE_PATHS += .\shader
INCLUDE_PATHS += .\CrystalCore\include
INCLUDE_PATHS += .\CrystalAlgorithms\include
INCLUDE_PATHS += .\CrystalThread\include

LIBRARIES := winmm
LIBRARIES += opengl32
LIBRARIES += gdi32

INCLUDE_PATHS := $(foreach item,$(INCLUDE_PATHS),-I$(item))
LIBRARIES := $(foreach item,$(LIBRARIES),-l$(item))

C_FILES := $(shell powershell Get-ChildItem -Recurse -Filter "*.c" -Path . -Name)
C_FILES_MOD := $(filter src\\%.c,$(C_FILES))
C_FILES_CORE := $(filter core\\%.c,$(C_FILES))
OBJ_FILES_MOD := $(patsubst src\\%.c,out\objs\src\\%.o,$(C_FILES_MOD))
OBJ_FILES_CORE := $(patsubst core\\%.c,out\objs\core\\%.o,$(C_FILES_CORE))

SHADER_FILES := $(shell powershell Get-ChildItem -Recurse -Filter "*.glsl" -Path . -Name)
OBJ_SHADER_FILES := $(patsubst shader\\%.glsl,out\objs\shader\\%.o,$(SHADER_FILES))

clean:
	@if not exist out\ (mkdir .\out)
	@if exist out\Windows\ (rmdir /s/q .\out\Windows)
	@if exist out\objs\ (rmdir /s/q .\out\objs)
	@mkdir .\out\Windows
	@mkdir .\out\objs\core
	@mkdir .\out\objs\src
	@mkdir .\out\objs\shader

copy:

out\objs\shader\\%.o : shader\\%.glsl
	@ld -r -b binary -z noexecstack $^ -o $@

out\objs\src\\%.o : src\\%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

out\objs\core\\%.o : core\\%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

build: clean $(OBJ_FILES_MOD) $(OBJ_FILES_CORE) $(OBJ_SHADER_FILES) copy
	@ar rcs out/Windows/libCrystalGraphic.a $(OBJ_FILES_CORE)
	@gcc -shared -o out/Windows/CrystalGraphic.so $(OBJ_FILES_MOD) $(OBJ_SHADER_FILES) $(INCLUDE_PATHS) $(LIBRARIES)

debug:
	@echo $(C_FILES)
	@echo $(C_FILES_MOD)
	@echo $(C_FILES_CORE)

.PHONY: build clean copy debug