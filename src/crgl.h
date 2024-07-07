#ifndef _INCLUDE_CRGL_H_
#define _INCLUDE_CRGL_H_

#include <GraphicDfs.h>
#include "glDfs.h"

#ifdef CR_WINDOWS
#  include <libloaderapi.h>
#  include <Windows.h>
#  define CR_GLAPI
#elif defined CR_LINUX
#  include <dlfcn.h>
#  include <GL/glx.h>
#  define CR_GLAPI
#endif

typedef struct cr_gl
{
    #ifdef CR_WINDOWS
    HDC _hDc;
    HGLRC _hRc;
    #elif defined CR_LINUX
    Display* dpy;
    Window wd;
    GLXContext context;
    #endif
    //
    const GLubyte* (*glGetString)(GLenum name);
    void (*glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (*glClear)(GLbitfield mask);
    void (*glLoadIdentity)(void);
    void (*glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
    void (*glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    void (*glDisable)(GLenum cap);
    void (*glEnable)(GLenum cap);
    void (*glBlendFunc)(GLenum sfactor, GLenum dfactor);
    void (*glBegin)(GLenum mode);
    void (*glEnd)(void);
    void (*glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
    void (*glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (*glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
    void (*glGenVertexArrays)(GLsizei n, GLuint* arrays);
    void (*glDeleteVertexArrays)(GLsizei n, const GLuint* arrays);
    void (*glGenBuffers)(GLsizei n, GLuint* buffers);
    void (*glDeleteBuffers)(GLsizei n, const GLuint* buffers);
    void (*glGenTextures)(GLsizei n, GLuint* textures);
    void (*glDeleteTextures)(GLsizei n, const GLuint* textures);
    void (*glBindVertexArray)(GLuint arr);
    void (*glBindBuffer)(GLenum target, GLuint buffer);
    void (*glBindTexture)(GLenum target, GLuint texture);
    void (*glVertexAttribPointer)(GLuint index, GLuint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
    void (*glBufferData)(GLenum target, GLsizei ptrSize, const GLvoid* data, GLenum usage);
    void (*glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLint format, GLenum type, const GLvoid* pixels);
    GLuint (*glCreateShader)(GLenum shaderType);
    void (*glDeleteShader)(GLuint shader);
    void (*glShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
    void (*glCompileShader)(GLuint shader);
    void (*glAttachShader)(GLuint program, GLuint shader);
    void (*glDetachShader)(GLuint program, GLuint shader);
    GLuint (*glCreateProgram)(void);
    void (*glDeleteProgram)(GLuint program);
    void (*glLinkProgram)(GLuint program);
    void (*glUseProgram)(GLuint program);
    void (*glEnableVertexAttribArray)(GLuint index);
    void (*glDisableVertexAttribArray)(GLuint index);
    void (*glDrawArrays)(GLenum mode, GLint first, GLsizei count);
    void (*glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indicies);
    void (*glPolygonMode)(GLenum face, GLenum mode);
    void (*glUniform1i)(GLint location, GLint v0);
    void (*glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
    void (*glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    GLuint (*glGetUniformLocation)(GLuint program, const GLchar* name);
    void (*glTexParameteri)(GLenum target, GLenum pname, GLint param);
    void (*glTexParameterfv)(GLenum target, GLenum pname, const GLfloat* params);
    void (*glGenerateMipmap)(GLenum target);
    void (*glActiveTexture)(GLenum texture);
}CR_GL;

#ifdef CR_WINDOWS
CR_GL* _inner_create_cr_gl_();
#elif defined CR_LINUX
CR_GL* _inner_create_cr_gl_(Display* pDisplay, XVisualInfo* vi, Window win);
#endif

void _inner_delete_cr_gl_(CR_GL* pgl);

#endif
