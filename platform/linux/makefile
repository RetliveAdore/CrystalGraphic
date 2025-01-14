INCLUDE_PATHS := ./include
INCLUDE_PATHS += ./depends/CrystalCoreRelease/include

INNER_INCLUDE_PATHS := ./src/headers

LIBRARY_PATHS := ./depends/CrystalCoreRelease/Linux

LIBRARIES := CrystalCore
LIBRARIES += X11
LIBRARIES += vulkan
LIBRARIES += GL

INCLUDE_PATHS := $(foreach item,$(INCLUDE_PATHS),-I$(item))
INNER_INCLUDE_PATHS := $(foreach item,$(INNER_INCLUDE_PATHS),-I$(item))
LIBRARY_PATHS := $(foreach item,$(LIBRARY_PATHS),-L$(item))
LIBRARIES := $(foreach item,$(LIBRARIES),-l$(item))

C_FILES := $(shell find . -name "*.c")
C_FILES_MOD := $(filter ./src/%.c,$(C_FILES))
C_FILES_CORE := $(filter ./core/%.c,$(C_FILES))
OBJ_FILES_MOD_DBG := $(patsubst ./src/%.c,./out/objs/debug/src/%.o,$(C_FILES_MOD))
OBJ_FILES_MOD_REL := $(patsubst ./src/%.c,./out/objs/release/src/%.o,$(C_FILES_MOD))
OBJ_FILES_CORE_DBG := $(patsubst ./core/%.c,./out/objs/debug/core/%.o,$(C_FILES_CORE))
OBJ_FILES_CORE_REL := $(patsubst ./core/%.c,./out/objs/release/core/%.o,$(C_FILES_CORE))

VERTSHADER_FILES := $(shell find . -name "*.vert")
VERTSPV_SHADER_FILES := $(patsubst ./%.vert,./out/objs/%.vert.spv,$(VERTSHADER_FILES))
VERTOBJ_SHADER_FILES := $(patsubst ./%.spv,%.ov,$(VERTSPV_SHADER_FILES))

FRAGSHADER_FILES := $(shell find . -name "*.frag")
FRAGSPV_SHADER_FILES := $(patsubst ./%.frag,./out/objs/%.frag.spv,$(FRAGSHADER_FILES))
FRAGOBJ_SHADER_FILES := $(patsubst ./%.spv,%.of,$(FRAGSPV_SHADER_FILES))

cleandbg:
	@if [ ! -d "./out/" ];then mkdir ./out;fi
	@if [ ! -d "./out/objs/shader" ];then mkdir -p ./out/objs/shader;fi
	@if [ -d "./out/Linux/debug/" ];then rm -r ./out/Linux/debug/;fi
	@if [ -d "./out/objs/debug/" ];then rm -r ./out/objs/debug/;fi
	@mkdir -p ./out/Linux/debug
	@mkdir -p ./out/objs/debug/core
	@mkdir -p ./out/objs/debug/src
	@mkdir -p ./out/objs/debug/src/vk
	@mkdir -p ./out/objs/debug/src/gl

cleanrel:
	@if [ ! -d "./out/" ];then mkdir ./out;fi
	@if [ ! -d "./out/objs/shader" ];then mkdir -p ./out/objs/shader;fi
	@if [ -d "./out/Linux/release/" ];then rm -r ./out/Linux/release/;fi
	@if [ -d "./out/objs/release/" ];then rm -r ./out/objs/release/;fi
	@mkdir -p ./out/Linux/release
	@mkdir -p ./out/objs/release/core
	@mkdir -p ./out/objs/release/src
	@mkdir -p ./out/objs/release/src/vk
	@mkdir -p ./out/objs/release/src/gl

cleanall: cleandbg cleanrel

./out/objs/shader/%.vert.spv : ./shader/%.vert
	@./depends/glslc $^ -o $@

./out/objs/shader/%.frag.spv : ./shader/%.frag
	@./depends/glslc $^ -o $@

./out/objs/shader/%.vert.ov : ./out/objs/shader/%.vert.spv
	@ld -r -b binary $^ -o $@

./out/objs/shader/%.frag.of : ./out/objs/shader/%.frag.spv
	@ld -r -b binary $^ -o $@

./out/objs/release/src/%.o: ./src/%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

./out/objs/release/core/%.o: ./core/%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

./out/objs/debug/src/%.o: ./src/%.c
	@gcc -DCR_BUILD_DEBUG -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

./out/objs/debug/core/%.o: ./core/%.c
	@gcc -DCR_BUILD_DEBUG -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

PUB_DEPENDS := $(INNER_INCLUDE_PATHS)
PUB_DEPENDS += $(INCLUDE_PATHS)
PUB_DEPENDS += $(LIBRARY_PATHS)
PUB_DEPENDS += $(LIBRARIES)

REL_BUILDS := $(OBJ_FILES_MOD_REL)
REL_BUILDS += $(FRAGOBJ_SHADER_FILES)
REL_BUILDS += $(VERTOBJ_SHADER_FILES)

build: cleanrel $(REL_BUILDS) $(OBJ_FILES_CORE_REL)
	@ar rcs out/Linux/release/libCrystalGraphic.a $(OBJ_FILES_CORE_REL)
	@gcc -shared -o out/Linux/release/CrystalGraphic.so $(REL_BUILDS) $(PUB_DEPENDS)

DBG_BUILDS := $(OBJ_FILES_MOD_DBG)
DBG_BUILDS += $(FRAGOBJ_SHADER_FILES)
DBG_BUILDS += $(VERTOBJ_SHADER_FILES)

debug: cleandbg $(DBG_BUILDS) $(OBJ_FILES_CORE_DBG)
	@ar rcs out/Linux/debug/libCrystalGraphic.a $(OBJ_FILES_CORE_DBG)
	@gcc -DCR_BUILD_DEBUG -shared -o out/Linux/debug/CrystalGraphic.so $(DBG_BUILDS) $(PUB_DEPENDS)

C_FILES_TEST := $(filter ./test/%.c,$(C_FILES))

TEST_LIBRARY_PATHS_DBG := ./depends/CrystalCoreRelease/Linux
TEST_LIBRARY_PATHS_DBG += ./out/Linux/debug

TEST_LIBRARY_PATHS_REL := ./depends/CrystalCoreRelease/Linux
TEST_LIBRARY_PATHS_REL += ./out/Linux/release

TEST_LIBRARIES := CrystalCore
TEST_LIBRARIES += CrystalGraphic
TEST_LIBRARIES += m

TEST_LIBRARY_PATHS_DBG := $(foreach item,$(TEST_LIBRARY_PATHS_DBG),-L$(item))
TEST_LIBRARY_PATHS_REL := $(foreach item,$(TEST_LIBRARY_PATHS_REL),-L$(item))
TEST_LIBRARIES := $(foreach item,$(TEST_LIBRARIES),-l$(item))

TEST_DEPENDS_DBG := $(C_FILES_TEST)
TEST_DEPENDS_DBG += $(INCLUDE_PATHS)
TEST_DEPENDS_DBG += $(TEST_LIBRARY_PATHS_DBG)
TEST_DEPENDS_DBG += $(TEST_LIBRARIES)

TEST_DEPENDS_REL := $(C_FILES_TEST)
TEST_DEPENDS_REL += $(INCLUDE_PATHS)
TEST_DEPENDS_REL += $(TEST_LIBRARY_PATHS_REL)
TEST_DEPENDS_REL += $(TEST_LIBRARIES)

cleantest:
	@if [ -f "./out/Linux/debug/Test1" ]; then rm ./out/Linux/debug/Test1;fi
	@if [ -f "./out/Linux/release/Test1" ]; then rm ./out/Linux/release/Test1;fi

testdbg:
	@if [ ! -f "./out/Linux/debug/CrystalGraphic.so" ];then echo you have to "make debug" first; \
	else gcc -o ./out/Linux/debug/Test1 $(TEST_DEPENDS_DBG); \
	cp ./depends/CrystalCoreRelease/Linux/CrystalCore.so ./out/Linux/debug/CrystalCore.so; \
	fi

testrel:
	@if [ ! -f "./out/Linux/release/CrystalGraphic.so" ];then echo you have to "make build" first; \
	else gcc -no-pie -o ./out/Linux/release/Test1 $(TEST_DEPENDS_REL); \
	cp ./depends/CrystalCoreRelease/Linux/CrystalCore.so ./out/Linux/release/CrystalCore.so; \
	fi

.PHONY: build debug testdbg testrel cleantest cleandbg clearnrel cleanall