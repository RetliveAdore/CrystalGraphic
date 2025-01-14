INCLUDE_PATHS := .\\include
INCLUDE_PATHS += .\\depends\\CrystalCoreRelease\\include

INNER_INCLUDE_PATHS := .\\src\\headers

LIBRARY_PATHS := .\\depends\\CrystalCoreRelease\\Windows

LIBRARIES := CrystalCore
LIBRARIES += vulkan-1

INCLUDE_PATHS := $(foreach item,$(INCLUDE_PATHS),-I$(item))
INNER_INCLUDE_PATHS := $(foreach item,$(INNER_INCLUDE_PATHS),-I$(item))
LIBRARY_PATHS := $(foreach item,$(LIBRARY_PATHS),-L$(item))
LIBRARIES := $(foreach item,$(LIBRARIES),-l$(item))

C_FILES := $(shell powershell Get-ChildItem -Recurse -Filter "*.c" -Path . -Name)
C_FILES_MOD := $(filter src\\%.c,$(C_FILES))
C_FILES_CORE := $(filter core\\%.c,$(C_FILES))
OBJ_FILES_MOD_DBG := $(patsubst src\\%.c,out\objs\debug\src\\%.o,$(C_FILES_MOD))
OBJ_FILES_MOD_REL := $(patsubst src\\%.c,out\objs\release\src\\%.o,$(C_FILES_MOD))
OBJ_FILES_CORE_DBG := $(patsubst core\\%.c,out\objs\debug\core\\%.o,$(C_FILES_CORE))
OBJ_FILES_CORE_REL := $(patsubst core\\%.c,out\objs\release\core\\%.o,$(C_FILES_CORE))

VERTSHADER_FILES := $(shell powershell Get-ChildItem -Recurse -Filter "*.vert" -Path . -Name)
VERTSPV_SHADER_FILES := $(patsubst %.vert,out\objs\\%.vert.spv,$(VERTSHADER_FILES))
VERTOBJ_SHADER_FILES := $(patsubst %.spv,%.ov,$(VERTSPV_SHADER_FILES))

FRAGSHADER_FILES := $(shell powershell Get-ChildItem -Recurse -Filter "*.frag" -Path . -Name)
FRAGSPV_SHADER_FILES := $(patsubst %.frag,out\objs\\%.frag.spv,$(FRAGSHADER_FILES))
FRAGOBJ_SHADER_FILES := $(patsubst %.spv,%.of,$(FRAGSPV_SHADER_FILES))

cleandbg:
	@if not exist out\\ (mkdir out)
	@if not exist out\\objs\\shader (mkdir out\\objs\\shader)
	@if exist out\\Windows\\debug\\ (rmdir /s/q out\\Windows\\debug)
	@if exist out\\objs\\debug\\ (rmdir /s/q out\\objs\\debug)
	@mkdir out\\Windows\\debug
	@mkdir out\\objs\\debug\\core
	@mkdir out\\objs\\debug\\src
	@mkdir out\\objs\\debug\\src\\vk
	@mkdir out\\objs\\debug\\src\\gl

cleanrel:
	@if not exist out\\ (mkdir out)
	@if not exist out\\objs\\shader (mkdir out\\objs\\shader)
	@if exist out\\Windows\\release\\ (rmdir /s/q out\\Windows\\release)
	@if exist out\\objs\\release\\ (rmdir /s/q out\\objs\\release)
	@mkdir out\\Windows\\release
	@mkdir out\\objs\\release\\core
	@mkdir out\\objs\\release\\src
	@mkdir out\\objs\\release\\src\\vk
	@mkdir out\\objs\\release\\src\\gl

cleanall: cleandbg cleanrel

out\objs\shader\\%.vert.spv : shader\\%.vert
	@depends\\glslc.exe $^ -o $@

out\objs\shader\\%.frag.spv : shader\\%.frag
	@depends\\glslc.exe $^ -o $@

out\objs\shader\\%.vert.ov : out\objs\shader\\%.vert.spv
	@ld -r -b binary $^ -o $@

out\objs\shader\\%.frag.of : out\objs\shader\\%.frag.spv
	@ld -r -b binary $^ -o $@

out\objs\release\src\\%.o : src\\%.c
	@gcc -fexec-charset=GBK -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

out\objs\release\core\\%.o : core\\%.c
	@gcc -fexec-charset=GBK -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

out\objs\debug\src\\%.o : src\\%.c
	@gcc -DCR_BUILD_DEBUG -fexec-charset=GBK -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

out\objs\debug\core\\%.o : core\\%.c
	@gcc -DCR_BUILD_DEBUG -fexec-charset=GBK -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS) $(INNER_INCLUDE_PATHS)

PUB_DEPENDS := $(INNER_INCLUDE_PATHS)
PUB_DEPENDS += $(INCLUDE_PATHS)
PUB_DEPENDS += $(LIBRARY_PATHS)
PUB_DEPENDS += $(LIBRARIES)

REL_BUILDS := $(OBJ_FILES_MOD_REL)
REL_BUILDS += $(FRAGOBJ_SHADER_FILES)
REL_BUILDS += $(VERTOBJ_SHADER_FILES)

build: cleanrel $(REL_BUILDS) $(OBJ_FILES_CORE_REL)
	@ar rcs out/Windows/release/libCrystalGraphic.a $(OBJ_FILES_CORE_REL)
	@gcc -fexec-charset=GBK -shared -o out/Windows/release/CrystalGraphic.so $(REL_BUILDS) $(PUB_DEPENDS)

DBG_BUILDS := $(OBJ_FILES_MOD_DBG)
DBG_BUILDS += $(FRAGOBJ_SHADER_FILES)
DBG_BUILDS += $(VERTOBJ_SHADER_FILES)

debug: cleandbg $(DBG_BUILDS) $(OBJ_FILES_CORE_DBG)
	@ar rcs out/Windows/debug/libCrystalGraphic.a $(OBJ_FILES_CORE_DBG)
	@gcc -DCR_BUILD_DEBUG -fexec-charset=GBK -shared -o out/Windows/debug/CrystalGraphic.so $(DBG_BUILDS) $(PUB_DEPENDS)

C_FILES_TEST := $(filter test\\%.c,$(C_FILES))

TEST_LIBRARY_PATHS_DBG := .\\depends\\CrystalCoreRelease\\Windows
TEST_LIBRARY_PATHS_DBG += .\\out\\Windows\\debug

TEST_LIBRARY_PATHS_REL += .\\depends\\CrystalCoreRelease\\Windows
TEST_LIBRARY_PATHS_REL += .\\out\\Windows\\release

TEST_LIBRARIES := CrystalCore
TEST_LIBRARIES += CrystalGraphic

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
	@if exist out\\Windows\\debug\\Test1.exe (rm out\\Windows\\debug\\Test1.exe)
	@if exist out\\Windows\\release\\Test1.exe (rm out\\Windows\\release\\Test1.exe)

testdbg:
	@if not exist out\\Windows\\debug\\CrystalGraphic.so (echo you have to "mingw32-make debug" first) \
	else (gcc -fexec-charset=GBK -o out\\Windows\\debug\\Test1.exe $(TEST_DEPENDS_DBG))
	@if exist out\\Windows\\debug\\Test1.exe (copy depends\\CrystalCoreRelease\\Windows\\CrystalCore.so out\\Windows\\debug\\CrystalCore.so)

testrel:
	@if not exist out\\Windows\\release\\CrystalGraphic.so (echo you have to "mingw32-make build" first) \
	else (gcc -mwindows -fexec-charset=GBK -o out\\Windows\\release\\Test1.exe $(TEST_DEPENDS_REL))
	@if exist out\\Windows\\release\\Test1.exe (copy depends\\CrystalCoreRelease\\Windows\\CrystalCore.so out\\Windows\\release\\CrystalCore.so)

.PHONY: build debug testdbg testrel cleantest cleandbg cleanrel cleanall