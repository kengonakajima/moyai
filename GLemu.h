#ifndef _GLEMU_H_
#define _GLEMU_H_

// Fake GL funcs, copied from GL/gl.h in linux

typedef unsigned int    GLenum;
typedef unsigned char   GLboolean;
typedef unsigned int    GLbitfield;
typedef void        GLvoid;
typedef signed char GLbyte;     /* 1-byte signed */
typedef short       GLshort;    /* 2-byte signed */
typedef int     GLint;      /* 4-byte signed */
typedef unsigned char   GLubyte;    /* 1-byte unsigned */
typedef unsigned short  GLushort;   /* 2-byte unsigned */
typedef unsigned int    GLuint;     /* 4-byte unsigned */
typedef int     GLsizei;    /* 4-byte signed */
typedef float       GLfloat;    /* single precision float */
typedef float       GLclampf;   /* single precision float in [0,1] */
typedef double      GLdouble;   /* double precision float */
typedef double      GLclampd;   /* double precision float in [0,1] */

#define GLAPI
#define GLAPIENTRY

GLAPI void GLAPIENTRY glDepthMask( GLboolean flag );
GLAPI void GLAPIENTRY glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
GLAPI void GLAPIENTRY glEnable( GLenum cap );
GLAPI void GLAPIENTRY glAlphaFunc( GLenum func, GLclampf ref );
GLAPI void GLAPIENTRY glDisable( GLenum cap );
GLAPI void GLAPIENTRY glCullFace( GLenum mode );
GLAPI void GLAPIENTRY glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
GLAPI void GLAPIENTRY glBindTexture( GLenum target, GLuint texture );
GLAPI void GLAPIENTRY glEnableClientState( GLenum cap );  /* 1.1 */
GLAPI void GLAPIENTRY glDisableClientState( GLenum cap );  /* 1.1 */
GLAPI void GLAPIENTRY glVertexPointer( GLint size, GLenum type,GLsizei stride, const GLvoid *ptr );
GLAPI void GLAPIENTRY glColorPointer( GLint size, GLenum type,GLsizei stride, const GLvoid *ptr );
GLAPI void GLAPIENTRY glNormalPointer( GLenum type, GLsizei stride,const GLvoid *ptr );
GLAPI void GLAPIENTRY glLoadIdentity( void );
GLAPI void GLAPIENTRY glTexCoordPointer( GLint size, GLenum type,GLsizei stride, const GLvoid *ptr );
GLAPI void GLAPIENTRY glLightfv( GLenum light, GLenum pname,const GLfloat *params );
GLAPI void GLAPIENTRY glTranslatef( GLfloat x, GLfloat y, GLfloat z );
GLAPI void GLAPIENTRY glRotatef( GLfloat angle,GLfloat x, GLfloat y, GLfloat z );
GLAPI void GLAPIENTRY glScalef( GLfloat x, GLfloat y, GLfloat z );
GLAPI void GLAPIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY glLineWidth( GLfloat width );
GLAPI void GLAPIENTRY glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
GLAPI void GLAPIENTRY glMatrixMode( GLenum mode );
GLAPI void GLAPIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor );
GLAPI void GLAPIENTRY glGetIntegerv( GLenum pname, GLint *params );
GLAPI void GLAPIENTRY glGetDoublev( GLenum pname, GLdouble *params );

GLAPI GLint GLAPIENTRY gluProject (GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* winX, GLdouble* winY, GLdouble* winZ);
GLAPI void GLAPIENTRY glReadPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels );




GLAPI void GLAPIENTRY gluPerspective (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
GLAPI void GLAPIENTRY gluLookAt (GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ);
GLAPI GLint GLAPIENTRY gluUnProject (GLdouble winX, GLdouble winY, GLdouble winZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* objX, GLdouble* objY, GLdouble* objZ);

GLAPI void GLAPIENTRY glClear( GLbitfield mask );
GLAPI void GLAPIENTRY glFlush( void );
GLAPI void GLAPIENTRY glReadBuffer( GLenum mode );

GLAPI void GLAPIENTRY glTexParameteri( GLenum target, GLenum pname, GLint param );

GLAPI void GLAPIENTRY glGenTextures( GLsizei n, GLuint *textures );
GLAPI GLenum GLAPIENTRY glGetError( void );
GLAPI void GLAPIENTRY glOrtho( GLdouble left, GLdouble right,
                               GLdouble bottom, GLdouble top,
                               GLdouble near_val, GLdouble far_val );
GLAPI void GLAPIENTRY glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels );

#define GL_TRUE                 0x1

#define GL_NEAREST              0x2600

#define GL_RGBA                 0x1908
#define GL_UNSIGNED_BYTE            0x1401
#define GL_TEXTURE_MAG_FILTER           0x2800
#define GL_TEXTURE_MIN_FILTER           0x2801
#define GL_LINEAR               0x2601

#define GL_BLEND                0x0BE2

#define GL_COLOR_BUFFER_BIT         0x00004000
#define GL_DEPTH_BUFFER_BIT         0x00000100

#define GL_DEPTH_COMPONENT          0x1902

#define GL_MODELVIEW_MATRIX         0x0BA6
#define GL_VIEWPORT             0x0BA2
#define GL_ONE                  0x1

#define GL_MODELVIEW                0x1700
#define GL_SRC_ALPHA                0x0302
#define GL_ONE_MINUS_SRC_ALPHA          0x0303
#define GL_PROJECTION               0x1701
#define GL_PROJECTION_MATRIX            0x0BA7

#define GL_LINES                0x0001
#define GL_LINE_STRIP               0x0003
#define GL_UNSIGNED_INT             0x1405
#define GL_DEPTH_TEST               0x0B71

#define GL_FRONT                0x0404

#define GL_DIFFUSE              0x1201
#define GL_SPECULAR             0x1202
#define GL_POSITION             0x1203

#define GL_LIGHTING             0x0B50
#define GL_LIGHT0               0x4000
#define GL_AMBIENT              0x1200

#define GL_TEXTURE_COORD_ARRAY          0x8078
#define GL_FLOAT                0x1406

#define GL_VERTEX_ARRAY             0x8074
#define GL_COLOR_ARRAY              0x8076
#define GL_INDEX_ARRAY              0x8077
#define GL_NORMAL_ARRAY             0x8075

#define GL_TRIANGLES                0x0004

#define GL_TEXTURE_2D               0x0DE1

#define GL_BACK                 0x0405

#define GL_CULL_FACE                0x0B44
#define GL_ALPHA_TEST               0x0BC0
#define GL_GREATER              0x0204

#endif
