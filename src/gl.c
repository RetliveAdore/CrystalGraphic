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

static void _inner_load_glapi_(CR_GL* pgl)
{
    pgl->glGetString = crGetProcAddress("glGetString");
}

#ifdef CR_WINDOWS

#elif defined CR_LINUX
CR_GL* _inner_create_cr_gl_(Display* pDisplay, XVisualInfo* vi, Window win)
{
    CR_GL *pgl = CRAlloc(NULL, sizeof(CR_GL));
    if (!pgl)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return NULL;
    }
    //创建上下文
    pgl->dpy = pDisplay;
    pgl->wd = win;
    pgl->context = glXCreateContext(pgl->dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(pgl->dpy, pgl->wd, pgl->context);
    //创建完上下文之后才能顺利执行
    pgl->glGetString = crGetProcAddress("glGetString");
    return pgl;
}
#endif

void _inner_delete_cr_gl_(CR_GL* pgl)
{
    glXMakeCurrent(pgl->dpy, None, NULL);
    glXDestroyContext(pgl->dpy, pgl->context);
    CRAlloc(pgl, 0);
}