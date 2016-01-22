
#include "ColorReplacerShader.h"

const static char replacer_shader[] = 
	"uniform sampler2D texture;\n"
	"uniform vec3 color1;\n"
	"uniform vec3 replace1;\n"
	"uniform float eps;\n"
	"void main() {\n"
	"	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy); \n"
	"	if(pixel.r > color1.r - eps && pixel.r < color1.r + eps && pixel.g > color1.g - eps && pixel.g < color1.g + eps && pixel.b > color1.b - eps && pixel.b < color1.b + eps ){\n"
	"		pixel = vec4(replace1, pixel.a );\n"
	"    }\n"
	"   pixel.r = gl_Color.r * pixel.r;\n"
	"   pixel.g = gl_Color.g * pixel.g;\n"
	"   pixel.b = gl_Color.b * pixel.b;\n"
	"   pixel.a = gl_Color.a * pixel.a;\n"    
	"	gl_FragColor = pixel;\n"
	"}\n";

bool ColorReplacerShader::init(){
	return load( replacer_shader );
}

void ColorReplacerShader::updateUniforms(){
	float fromcol[]={ from_color.r, from_color.g, from_color.b};
	GLuint uniid1=glGetUniformLocation(program,"color1");
	glUniform3fv( uniid1,1,fromcol);

	float tocol[]={ to_color.r, to_color.g, to_color.b };
	GLuint uniid2=glGetUniformLocation(program,"replace1");
	glUniform3fv( uniid2,1,tocol);

	GLuint uniid3=glGetUniformLocation(program,"texture");
	glUniform1i(uniid3, 0 );

	GLuint uniid4=glGetUniformLocation(program,"eps");
	glUniform1f(uniid4, epsilon );

}

