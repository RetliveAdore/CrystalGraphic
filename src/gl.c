/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-07-08 12:33:11
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-07-10 23:32:33
 * @FilePath: \CrystalGraphic\src\gl.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include "crgl.h"

#define CRGL_RATIO 1.0f

#ifdef CR_WINDOWS
HMODULE libgl = NULL;
#elif defined CR_LINUX
void* libgl = NULL;
#endif

//供init.c中启动函数使用
void _inner_init_gl_()
{
    #ifdef CR_WINDOWS
    libgl = LoadLibraryA("opengl32.dll");
    #elif defined CR_LINUX
    libgl = dlopen("libOpenGL.so", RTLD_LAZY);
    #endif
    if (!libgl) CR_LOG_ERR("auto", "Failed load OpenGL library");
}

//供init.c中清理函数使用
void _inner_uninit_gl_()
{
    if (libgl)
    {
        #ifdef CR_WIDNOWS
        FreeLibrary(libgl);
        #elif defined CR_LINUX
        dlclose(libgl);
        #endif
    }
    libgl = NULL;
}

#ifdef CR_WINDOWS
static void* crGetProcAddress(const char* name)
{
    PROC ret = wglGetProcAddress(name);
    if (ret == NULL)
    {
        ret = GetProcAddress(libgl, name);
    }
    if (!ret) CR_LOG_WAR("auto", "Unable to load: %s", name);
    return ret;
}
#elif defined CR_LINUX
static void* crGetProcAddress(const char* name)
{
    void* ret = (void*)glXGetProcAddress((const GLubyte*)name);
    if (ret == NULL)
    {
        ret = (void*)dlsym(libgl, name);
    }
    if (!ret) CR_LOG_WAR("auto", "Unable to load: %s", name);
    return ret;
}
#endif

#define _INNER_LOAD_(name) pgl->gl##name = crGetProcAddress("gl" #name)

static void _inner_load_glapi_(CR_GL* pgl)
{
    _INNER_LOAD_(GetString);
    _INNER_LOAD_(ClearColor);
    _INNER_LOAD_(Clear);
    _INNER_LOAD_(LoadIdentity);
    _INNER_LOAD_(Viewport);
    _INNER_LOAD_(Ortho);
    _INNER_LOAD_(Disable);
    _INNER_LOAD_(Enable);
    _INNER_LOAD_(BlendFunc);
    _INNER_LOAD_(Begin);
    _INNER_LOAD_(End);
    _INNER_LOAD_(Color3f);
    _INNER_LOAD_(Color4f);
    _INNER_LOAD_(Vertex3f);
    _INNER_LOAD_(GenVertexArrays);
    _INNER_LOAD_(DeleteVertexArrays);
    _INNER_LOAD_(GenBuffers);
    _INNER_LOAD_(DeleteBuffers);
    _INNER_LOAD_(GenTextures);
    _INNER_LOAD_(DeleteTextures);
    _INNER_LOAD_(BindVertexArray);
    _INNER_LOAD_(BindBuffer);
    _INNER_LOAD_(BindTexture);
    _INNER_LOAD_(VertexAttribPointer);
    _INNER_LOAD_(BufferData);
    _INNER_LOAD_(TexImage2D);
    _INNER_LOAD_(CreateShader);
    _INNER_LOAD_(DeleteShader);
    _INNER_LOAD_(ShaderSource);
    _INNER_LOAD_(CompileShader);
    _INNER_LOAD_(AttachShader);
    _INNER_LOAD_(DetachShader);
    _INNER_LOAD_(CreateProgram);
    _INNER_LOAD_(DeleteProgram);
    _INNER_LOAD_(LinkProgram);
    _INNER_LOAD_(UseProgram);
    _INNER_LOAD_(EnableVertexAttribArray);
    _INNER_LOAD_(DisableVertexAttribArray);
    _INNER_LOAD_(DrawArrays);
    _INNER_LOAD_(DrawElements);
    _INNER_LOAD_(PolygonMode);
    _INNER_LOAD_(Uniform1i);
    _INNER_LOAD_(Uniform2f);
    _INNER_LOAD_(Uniform4f);
    _INNER_LOAD_(GetUniformLocation);
    _INNER_LOAD_(TexParameteri);
    _INNER_LOAD_(TexParameterfv);
    _INNER_LOAD_(GenerateMipmap);
    _INNER_LOAD_(ActiveTexture);
}

#ifdef CR_WINDOWS
//像素格式
static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
};

static void _inner_setup_pixel_format_(HDC hDc)
{
    int nPixelFormat;
    nPixelFormat = ChoosePixelFormat(hDc, &pfd);
    SetPixelFormat(hDc, nPixelFormat, &pfd);
}
#endif

CR_GL* _inner_create_cr_gl_(
    #ifdef CR_WINDOWS
    HDC hDc
    #elif defined CR_LINUX
    Display* pDisplay,
    XVisualInfo* vi,
    Window win
    #endif
)
{
    CR_GL* pgl = CRAlloc(NULL, sizeof(CR_GL));
    if (!pgl)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    pgl->aspx = 1.0f;
    pgl->aspy = 1.0f;
    pgl->dx = 0.0f;
    pgl->dy = 0.0f;
    pgl->ratio = 0.0f;
    //创建上下文
    #ifdef CR_WINDOWS
    pgl->hdc = hDc;
    _inner_setup_pixel_format_(pgl->hdc);
    pgl->hrc = wglCreateContext(pgl->hdc);
    wglMakeCurrent(pgl->hdc, pgl->hrc);
    #elif defined CR_LINUX
    pgl->dpy = pDisplay;
    pgl->wd = win;
    pgl->context = glXCreateContext(pgl->dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(pgl->dpy, pgl->wd, pgl->context);
    #endif
    //创建完成之后才会有版本信息
    _inner_load_glapi_(pgl);
    //进行一些初始化
    pgl->glDisable(GL_DEPTH_TEST);
    pgl->glDisable(GL_DEPTH);
    pgl->glEnable(GL_BLEND);
    pgl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pgl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    return pgl;
}

void _inner_delete_cr_gl_(CR_GL* pgl)
{
    #ifdef CR_WINDOWS
    wglMakeCurrent(pgl->hdc, NULL);
    wglDeleteContext(pgl->hrc);
    #elif defined CR_LINUX
    glXMakeCurrent(pgl->dpy, None, NULL);
    glXDestroyContext(pgl->dpy, pgl->context);
    #endif
    CRAlloc(pgl, 0);
}

static void _inner_fill_port_(CR_GL* pgl, float r, float g, float b)
{
    pgl->glColor3f(r, g, b);
    pgl->glBegin(GL_QUADS);
    pgl->glVertex3f(1.0f, 1.0f, 0);
    pgl->glVertex3f(1.0f, -1.0f, 0);
    pgl->glVertex3f(-1.0f, -1.0f, 0);
    pgl->glVertex3f(-1.0f, 1.0f, 0);
    pgl->glEnd();
}

static void _inner_draw_titlebar_(CR_GL* pgl)
{
    pgl->glLoadIdentity();
    pgl->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
    //标题栏底色
    pgl->glViewport(0, pgl->h, pgl->w, CRUI_TITLEBAR_PIXEL);
    _inner_fill_port_(pgl, 0.2, 0.3, 0.35);
    //三个按钮
    pgl->glViewport(CRUI_TITLEBAR_PIXEL / 4, pgl->h + CRUI_TITLEBAR_PIXEL / 4, CRUI_TITLEBAR_PIXEL / 2, CRUI_TITLEBAR_PIXEL / 2);
    _inner_fill_port_(pgl, 0.95, 0.45, 0.55);
    //
    pgl->glViewport(CRUI_TITLEBAR_PIXEL + CRUI_TITLEBAR_PIXEL / 4, pgl->h + CRUI_TITLEBAR_PIXEL / 4, CRUI_TITLEBAR_PIXEL / 2, CRUI_TITLEBAR_PIXEL / 2);
    _inner_fill_port_(pgl, 0.55, 0.9, 0.55);
    //
    pgl->glViewport(CRUI_TITLEBAR_PIXEL * 2 + CRUI_TITLEBAR_PIXEL / 4, pgl->h + CRUI_TITLEBAR_PIXEL / 4, CRUI_TITLEBAR_PIXEL / 2, CRUI_TITLEBAR_PIXEL / 2);
    _inner_fill_port_(pgl, 0.7, 0.6, 1.0);
    //
}

static void _inner_ratio_(CR_GL* pgl)
{
    pgl->ratio = CRGL_RATIO / (float)(pgl->w < pgl->h ? pgl->w : pgl->h) * 2;
    if (pgl->w > pgl->h)
    {
        pgl->dx = pgl->w * pgl->ratio / 2;
        pgl->dy = CRGL_RATIO;
        pgl->aspx = (float)pgl->h / (float)pgl->w;
        pgl->aspy = 1.0;
    }
    else
    {
        pgl->dx = CRGL_RATIO;
        pgl->dy = pgl->h * pgl->ratio / 2;
        pgl->aspx = 1.0f;
        pgl->aspy = (float)pgl->w / (float)pgl->h;
    }
}

void _inner_cr_gl_resize_(CR_GL* pgl)
{
    pgl->glViewport(0, 0, pgl->w, pgl->h);
    pgl->glLoadIdentity();
    _inner_ratio_(pgl);
}

void _inner_cr_gl_paint_(CR_GL* pgl)
{
    pgl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _inner_draw_titlebar_(pgl);
    _inner_cr_gl_resize_(pgl);
    //更新缓冲
    #ifdef CR_WINDOWS
    SwapBuffers(pgl->hdc);
    #elif defined CR_LINUX
    glXSwapBuffers(pgl->dpy, pgl->wd);
    #endif
}

void _inner_set_size_(CR_GL* pgl, CRUINT64 w, CRUINT64 h)
{
    pgl->w = w;
    pgl->h = h;
}