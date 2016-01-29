#include "cumino.h"
#include "client.h"

#include "FragmentShader.h"

#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE)

static bool shaderCompile(GLuint shader, const char *src){
	GLsizei len = strlen(src), size;
	GLint compiled;

	glShaderSource(shader, 1, (const GLchar**) & src, (GLint*)&len );

	// シェーダのコンパイル
	glCompileShader(shader);
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

	if ( compiled == GL_FALSE ){
		print( "couldn't compile shader: %s \n ", src );
		glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &size );
		if ( size > 0 ){
			GLchar *buf;
			buf = (char *)MALLOC(size);
			glGetShaderInfoLog( shader, size, &len, buf);
			printf("%s",buf);
			FREE(buf);
		}
		return false;
	}
	return true;
}

static bool link( GLuint prog ){
	GLsizei size, len;
	GLint linked;
	char *infoLog ;

	glLinkProgram( prog );

	glGetProgramiv( prog, GL_LINK_STATUS, &linked );

	if ( linked == GL_FALSE ){
		print("couldnt link shader:\n");

		glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &size );
		if ( size > 0 ){
			infoLog = (char *)MALLOC(size);
			glGetProgramInfoLog( prog, size, &len, infoLog );
			printf("%s",infoLog);
			FREE(infoLog);
		}
		return false;
	}
	return true;
}

#endif

bool FragmentShader::load( const char *src) {

#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE)    
	GLuint shader = glCreateShader( GL_FRAGMENT_SHADER );
	if(!shaderCompile( shader, src)){
		glDeleteShader(shader);
		return false;
	}

	program = glCreateProgram();
	glAttachShader( program, shader );
	glDeleteShader(shader);

	if(!link( program ) ){
		return false;
	}
#endif    
	return true;
}


