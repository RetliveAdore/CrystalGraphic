INCLUDE_PATHS := ./include
INCLUDE_PATHS += ./shader
INCLUDE_PATHS += ./CrystalCore/include
INCLUDE_PATHS += ./CrystalAlgorithms/include
INCLUDE_PATHS += ./CrystalThread/include

LIBRARIES := X11
LIBRARIES += GL

INCLUDE_PATHS := $(foreach item,$(INCLUDE_PATHS),-I$(item))
LIBRARIES := $(foreach item,$(LIBRARIES),-l$(item))

C_FILES := $(shell find . -name "*.c")
C_FILES_MOD := $(filter ./src/%.c,$(C_FILES))
C_FILES_CORE := $(filter ./core/%.c,$(C_FILES))
OBJ_FILES_MOD := $(patsubst ./src/%.c,./out/objs/src/%.o,$(C_FILES_MOD))
OBJ_FILES_CORE := $(patsubst ./core/%.c,./out/objs/core/%.o,$(C_FILES_CORE))

SHADER_FILES := $(shell find . -name "*.glsl")
OBJ_SHADER_FILES := $(patsubst ./shader/%.glsl,./out/objs/shader/%.o,$(SHADER_FILES))

clean:
	@if [ ! -d "./out/" ];then mkdir ./out;fi
	@if [ -d "./out/Linux/" ];then rm -r ./out/Linux/;fi
	@if [ -d "./out/objs/" ];then rm -r ./out/objs/;fi
	@mkdir -p ./out/Linux
	@mkdir -p ./out/objs/core
	@mkdir -p ./out/objs/src
	@mkdir -p ./out/objs/shader

copy:

./out/objs/shader/%.o : ./shader/%.glsl
	@ld -r -b binary $^ -o $@

./out/objs/src/%.o : ./src/%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

./out/objs/core/%.o : ./core/%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

build: clean $(OBJ_FILES_MOD) $(OBJ_SHADER_FILES) $(OBJ_FILES_CORE) copy
	@ar rcs out/Linux/libCrystalGraphic.a $(OBJ_FILES_CORE)
	@gcc -shared -o out/Linux/CrystalGraphic.so $(OBJ_FILES_MOD) $(OBJ_SHADER_FILES) $(INCLUDE_PATHS) $(LIBRARIES)

debug:
	@echo $(C_FILES)
	@echo $(C_FILES_MOD)
	@echo $(C_FILES_CORE)

.PHONY: build clean copy debug