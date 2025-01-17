#include "head.h"

CRCODE mouseCallback(PCRWINDOWMSG msg)
{
    static CRINT64 oldX, oldY;
    if (msg->status & CRUI_STAT_MOVE)
    {
        if (status.pressed)
        {
            float dx, dy;
            _delta_pixel_to_float_(msg->x - oldX, msg->y - oldY, &dx, &dy);
            status.posX = status.oldX + dx;
            status.posY = status.oldY + dy;
        }
    }
    else if (msg->status & CRUI_STAT_DOWN)
    {
        oldX = msg->x;
        oldY = msg->y;
        status.oldX = status.posX;
        status.oldY = status.posY;
        status.pressed = CRTRUE;
    }
    else if (msg->status & CRUI_STAT_UP)
    {
        status.pressed = CRFALSE;
    }
    else if ((msg->status & CRUI_STAT_MIDD) && (msg->status & CRUI_STAT_SCROLL))
    {
        status.zoom += 0.001f * (float)msg->z;
        if (status.zoom < 0.05f) status.zoom = 0.05f;
        else if (status.zoom > 1.5f) status.zoom = 1.5f;
    }
    return 0;
}

CRCODE focusCallback(PCRWINDOWMSG msg)
{
    if (msg->status & CRUI_STAT_DOWN)
    {
        status.pressed = CRFALSE;
    }
}

CRCODE keyCallback(PCRWINDOWMSG msg)
{
    if (msg->status & CRUI_STAT_DOWN)
    {
        if (msg->keycode == 'W')
            status.pu = CRTRUE;
        else if (msg->keycode == 'A')
            status.pl = CRTRUE;
        else if (msg->keycode == 'S')
            status.pd = CRTRUE;
        else if (msg->keycode == 'D')
            status.pr = CRTRUE;
        else if (msg->keycode == CR_VK_ESC)
            CRCloseWindow(game.windowProperty.window);
    }
    else if (msg->status & CRUI_STAT_UP)
    {
        if (msg->keycode == 'W')
            status.pu = CRFALSE;
        else if (msg->keycode == 'A')
            status.pl = CRFALSE;
        else if (msg->keycode == 'S')
            status.pd = CRFALSE;
        else if (msg->keycode == 'D')
            status.pr = CRFALSE;
    }
}
