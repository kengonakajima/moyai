#include <stdio.h>

#include "GLemu.h"



#define WARN fprintf(stderr,"GL func called: %s:%d", __FILE__, __LINE__ )

GLAPI void GLAPIENTRY glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {WARN;}
GLAPI void GLAPIENTRY glDepthMask( GLboolean flag ) {}
GLAPI void GLAPIENTRY glEnable( GLenum cap ){}
GLAPI void GLAPIENTRY glAlphaFunc( GLenum func, GLclampf ref ){}
GLAPI void GLAPIENTRY glDisable( GLenum cap ){}
GLAPI void GLAPIENTRY glCullFace( GLenum mode ){}
GLAPI void GLAPIENTRY glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
GLAPI void GLAPIENTRY glBindTexture( GLenum target, GLuint texture ){}
GLAPI void GLAPIENTRY glEnableClientState( GLenum cap ){}  /* 1.1 */
GLAPI void GLAPIENTRY glDisableClientState( GLenum cap ){}  /* 1.1 */
GLAPI void GLAPIENTRY glVertexPointer( GLint size, GLenum type,GLsizei stride, const GLvoid *ptr ){}
GLAPI void GLAPIENTRY glColorPointer( GLint size, GLenum type,GLsizei stride, const GLvoid *ptr ){}
GLAPI void GLAPIENTRY glNormalPointer( GLenum type, GLsizei stride,const GLvoid *ptr ){}
GLAPI void GLAPIENTRY glLoadIdentity( void ){}
GLAPI void GLAPIENTRY glTexCoordPointer( GLint size, GLenum type,GLsizei stride, const GLvoid *ptr ){}
GLAPI void GLAPIENTRY glLightfv( GLenum light, GLenum pname,const GLfloat *params ){}
GLAPI void GLAPIENTRY glTranslatef( GLfloat x, GLfloat y, GLfloat z ){}
GLAPI void GLAPIENTRY glRotatef( GLfloat angle,GLfloat x, GLfloat y, GLfloat z ){}
GLAPI void GLAPIENTRY glScalef( GLfloat x, GLfloat y, GLfloat z ){}
GLAPI void GLAPIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params ){}
GLAPI void GLAPIENTRY glLineWidth( GLfloat width ){}
GLAPI void GLAPIENTRY glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ){}
GLAPI void GLAPIENTRY glMatrixMode( GLenum mode ){}
GLAPI void GLAPIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor ){}
GLAPI void GLAPIENTRY glGetIntegerv( GLenum pname, GLint *params ){}
GLAPI void GLAPIENTRY glGetDoublev( GLenum pname, GLdouble *params ){}

GLAPI GLint GLAPIENTRY gluProject (GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* winX, GLdouble* winY, GLdouble* winZ){ return 0;}
GLAPI void GLAPIENTRY glReadPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels ){}




GLAPI void GLAPIENTRY gluPerspective (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar){}
GLAPI void GLAPIENTRY gluLookAt (GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ){}
GLAPI GLint GLAPIENTRY gluUnProject (GLdouble winX, GLdouble winY, GLdouble winZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* objX, GLdouble* objY, GLdouble* objZ){ return 0; }

GLAPI void GLAPIENTRY glClear( GLbitfield mask ){}
GLAPI void GLAPIENTRY glFlush( void ){}
GLAPI void GLAPIENTRY glReadBuffer( GLenum mode ){}

GLAPI void GLAPIENTRY glTexParameteri( GLenum target, GLenum pname, GLint param ){}

GLAPI void GLAPIENTRY glGenTextures( GLsizei n, GLuint *textures ){
    static int cnt=1;
    for(int i=0;i<n;i++) textures[i] = ++cnt;
}
GLAPI GLenum GLAPIENTRY glGetError( void ){ return 0; }
GLAPI void GLAPIENTRY glOrtho( GLdouble left, GLdouble right,
                               GLdouble bottom, GLdouble top,
                               GLdouble near_val, GLdouble far_val ){}
GLAPI void GLAPIENTRY glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels ){}

