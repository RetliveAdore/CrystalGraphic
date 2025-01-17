#include "head.h"

static void _inner_auto_speedup_(float dt)
{
    status.oldvX = status.vX;
    status.oldvY = status.vY;
    if (status.vX > 0.0f)
    {
        status.vX += dt * SPEED_UP_RATIO;
        if (status.vX > SPEED_F_MAX)
            status.vX = SPEED_F_MAX;
    }
    else if (status.vX < 0.0f)
    {
        status.vX -= dt * SPEED_UP_RATIO;
        if (status.vX < -SPEED_F_MAX)
            status.vX = -SPEED_F_MAX;
    }
    if (status.vY > 0.0f)
    {
        status.vY += dt * SPEED_UP_RATIO;
        if (status.vY > SPEED_F_MAX)
            status.vY = SPEED_F_MAX;
    }
    else if (status.vY < 0.0f)
    {
        status.vY -= dt * SPEED_UP_RATIO;
        if (status.vY < -SPEED_F_MAX)
            status.vY = -SPEED_F_MAX;
    }
}

static void _inner_setup_speed_()
{
    if (status.pl)
    {
        if (!status.pr)
        {
            if (status.vX == 0.0f)
                status.vX = SPEED_F;
        }
        else
            status.vX = 0.0f;
    }
    else if (status.pr)
    {
        if (status.vX == 0.0f)
            status.vX = -SPEED_F;
    }
    else status.vX = 0.0f;
    if (status.pu)
    {
        if (!status.pd)
        {
            if (status.vY == 0.0f)
                status.vY = SPEED_F;
        }
        else
            status.vY = 0.0f;
    }
    else if (status.pd)
    {
        if (status.vY == 0.0f)
            status.vY = -SPEED_F;
    }
    else status.vY = 0.0f;
}

void mainloop()
{
    //
    float dt = CRTimerMark(&status.timer);
    
    //更新速度状态
    //自动加速
    _inner_auto_speedup_(dt);
    //自动初始化速度
    _inner_setup_speed_();
    //更新坐标状态
    float halfdx = (status.vX - status.oldvX) / 2;
    float halfdy = (status.vY - status.oldvY) / 2;
    status.posX += dt * (status.vX + halfdx);
    status.posY += dt * (status.vY + halfdy);
    //更新摄像机状态
    game.camera.x = status.posX;
    game.camera.y = status.posY;
    game.camera.zoom = status.zoom;
    game.camera.angle = status.angle;
    CRSetGlobalProp(game.windowProperty.window, &game.camera);
}