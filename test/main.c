/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-15 19:24:44
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-15 21:37:25
 * @FilePath: \PixelCrystal\src\main.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include "head.h"
#include <string.h>

GAMESTRUCT game;

CRBOOL init(char *argv_0)
{
    game.core = CRImport("CrystalCore.so", CRCoreFunList, argv_0);
    if (!game.core)
        return CRFALSE;
    CRSetLogFile("PixelCrystal.log");
    CRLogDefault("file", 0);
    CR_LOG_IFO("auto", "core imported");
    CRMemSetup(MAX_MEM);
    CRThreadInit();
    game.graphic = CRImport("CrystalGraphic.so", CRGraphicFunList, argv_0);
    if (!game.graphic)
    {
        CR_LOG_ERR("auto", "failed to import graphic");
        CRThreadUninit();
        CRMemClear();
        CRUnload(game.core);
        return CRFALSE;
    }
    CR_LOG_IFO("auto", "graphic imported");
    memset(&game, 0, sizeof(game));
    memset(&status, 0, sizeof(status));
    return CRTRUE;
}

void uninit()
{
    CRUnload(game.graphic);
    CRThreadUninit();
    CRMemClear();
    CRUnload(game.core);
}

int main(int argc, char **argv)
{
    if (!init(argv[0]))
        return 1;
    //创建窗口
    if (!initWindow())
        goto End;
    if (!initResources(argv[0]))
    {
        CRCloseWindow(game.windowProperty.window);
        goto FailedtoInitResources;
    }
    //
FailedtoInitResources:
    CRTimerMark(&status.timer);
    while(CRWindowCounter())
    {
        mainloop();
        CRSleep(1);
    }
    //
End:
    uninit();
    return 0;
}