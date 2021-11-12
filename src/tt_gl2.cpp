//tt_gl2.cpp - Auxiliary OpenGL functions
//Copyright (C) 2021 Ayman Wagih
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include			"tt.h"
#include			"tt_gl2_vector.h"
#include			<vector>
const char			file[]=__FILE__;

extern const float	_pi=acos(-1.f), _2pi=2*_pi, pi_2=_pi*0.5f, inv_2pi=1/_2pi,
					sqrt2=sqrt(2.f),
					torad=_pi/180, todeg=180/_pi,
					infinity=(float)_HUGE,
					inv255=1.f/255, inv256=1.f/256, inv128=1.f/128;

const char			*GLversion=nullptr;
int					glMajorVer=0, glMinorVer=0;

#ifndef __linux__
GL_State			GL2_state=GL2_NOTHING;
const int 			gl_api_decl_start=__LINE__;//const var is already static
void				(__stdcall *glBlendEquation)(unsigned mode)=nullptr;
//void				(__stdcall *glGenVertexArrays)(int n, unsigned *arrays)=nullptr;//OpenGL 3.0
//void				(__stdcall *glDeleteVertexArrays)(int n, unsigned *arrays)=nullptr;//OpenGL 3.0
void				(__stdcall *glBindVertexArray)(unsigned arr)=nullptr;
void				(__stdcall *glGenBuffers)(int n, unsigned *buffers)=nullptr;
void				(__stdcall *glBindBuffer)(unsigned target, unsigned buffer)=nullptr;
void				(__stdcall *glBufferData)(unsigned target, int size, const void *data, unsigned usage)=nullptr;
void				(__stdcall *glBufferSubData)(unsigned target, int offset, int size, const void *data)=nullptr;
void				(__stdcall *glEnableVertexAttribArray)(unsigned index)=nullptr;
void				(__stdcall *glVertexAttribPointer)(unsigned index, int size, unsigned type, unsigned char normalized, int stride, const void *pointer)=nullptr;
void				(__stdcall *glDisableVertexAttribArray)(unsigned index)=nullptr;
unsigned			(__stdcall *glCreateShader)(unsigned shaderType)=nullptr;
void				(__stdcall *glShaderSource)(unsigned shader, int count, const char **string, const int *length)=nullptr;
void				(__stdcall *glCompileShader)(unsigned shader)=nullptr;
void				(__stdcall *glGetShaderiv)(unsigned shader, unsigned pname, int *params)=nullptr;
void				(__stdcall *glGetShaderInfoLog)(unsigned shader, int maxLength, int *length, char *infoLog)=nullptr;
unsigned			(__stdcall *glCreateProgram)()=nullptr;
void				(__stdcall *glAttachShader)(unsigned program, unsigned shader)=nullptr;
void				(__stdcall *glLinkProgram)(unsigned program)=nullptr;
void				(__stdcall *glGetProgramiv)(unsigned program, unsigned pname, int *params)=nullptr;
void				(__stdcall *glGetProgramInfoLog)(unsigned program, int maxLength, int *length, char *infoLog)=nullptr;
void				(__stdcall *glDetachShader)(unsigned program, unsigned shader)=nullptr;
void				(__stdcall *glDeleteShader)(unsigned shader)=nullptr;
void				(__stdcall *glUseProgram)(unsigned program)=nullptr;
int					(__stdcall *glGetAttribLocation)(unsigned program, const char *name)=nullptr;
void				(__stdcall *glDeleteProgram)(unsigned program)=nullptr;
void				(__stdcall *glDeleteBuffers)(int n, const unsigned *buffers)=nullptr;
int					(__stdcall *glGetUniformLocation)(unsigned program, const char *name)=nullptr;
void				(__stdcall *glUniformMatrix3fv)(int location, int count, unsigned char transpose, const float *value)=nullptr;
void				(__stdcall *glUniformMatrix4fv)(int location, int count, unsigned char transpose, const float *value)=nullptr;
void				(__stdcall *glGetBufferParameteriv)(unsigned target, unsigned value, int *data)=nullptr;
void				(__stdcall *glActiveTexture)(unsigned texture)=nullptr;
void				(__stdcall *glUniform1i)(int location, int v0)=nullptr;
void				(__stdcall *glUniform2i)(int location, int v0, int v1)=nullptr;
void				(__stdcall *glUniform1f)(int location, float v0)=nullptr;
void				(__stdcall *glUniform2f)(int location, float v0, float v1)=nullptr;
void				(__stdcall *glUniform3f)(int location, float v0, float v1, float v2)=nullptr;
void				(__stdcall *glUniform3fv)(int location, int count, const float *value)=nullptr;
void				(__stdcall *glUniform4f)(int location, float v0, float v1, float v2, float v3)=nullptr;
const int 			gl_api_decl_end=__LINE__;
#endif

//error handling
void 				p_check(void (*p)(), const char *file, int line, const char *func_name, int *state)
{
	if(!p)
	{
		log_error(file, line, "Error: %s is 0x%p", func_name, p);
		*state=0;
	}
}
void				my_assert(int success, const char *file, int line, const char *msg)
{
	if(!success)
		log_error(file, line, msg);
}
#define				MY_ASSERT(SUCCESS, MESSAGE)		my_assert((SUCCESS)!=0, file, __LINE__, MESSAGE)
const char*			glerr2str(int error)
{
#define 			EC(x)	case x:a=(const char*)#x;break
	const char *a=nullptr;
	switch(error)
	{
	case 0:a="SUCCESS";break;
	EC(GL_INVALID_ENUM);
	EC(GL_INVALID_VALUE);
	EC(GL_INVALID_OPERATION);
	case 0x0503:a="GL_STACK_OVERFLOW";break;
	case 0x0504:a="GL_STACK_UNDERFLOW";break;
	EC(GL_OUT_OF_MEMORY);
	case 0x0506:a="GL_INVALID_FRAMEBUFFER_OPERATION";break;
	case 0x0507:a="GL_CONTEXT_LOST";break;
	case 0x8031:a="GL_TABLE_TOO_LARGE";break;
	default:a="???";break;
	}
	return a;
#undef				EC
}
void 				gl_check(const char *file, int line)
{
	int err=glGetError();
	if(err)
		log_error(file, line, "GL %d: %s", err, glerr2str(err));
}
void				gl_error(const char *file, int line)
{
	int err=glGetError();
	log_error(file, line, "GL %d: %s", err, glerr2str(err));
}

#ifndef __linux__
void				load_OGL_API()
{
	if(!GL2_state)//load API
	{
		GL2_state=GL2_LOADING_API;
#define	GET_GL_FUNC(glFunc)				glFunc=(decltype(glFunc))wglGetProcAddress(#glFunc), p_check((void(*)())glFunc, file, __LINE__, #glFunc, (int*)&GL2_state)
#define	GET_GL_FUNC_UNCHECKED(glFunc)	glFunc=(decltype(glFunc))wglGetProcAddress(#glFunc)
		const int gl_api_init_start=__LINE__;
		GET_GL_FUNC(glBlendEquation);
	//	GET_GL_FUNC(glGenVertexArrays);//OpenGL 3.0
	//	GET_GL_FUNC(glDeleteVertexArrays);//OpenGL 3.0
		GET_GL_FUNC(glBindVertexArray);
		GET_GL_FUNC(glGenBuffers);
		GET_GL_FUNC(glBindBuffer);
		GET_GL_FUNC(glBufferData);
		GET_GL_FUNC(glBufferSubData);
		GET_GL_FUNC(glEnableVertexAttribArray);
		GET_GL_FUNC(glVertexAttribPointer);
		GET_GL_FUNC(glDisableVertexAttribArray);
		GET_GL_FUNC(glCreateShader);
		GET_GL_FUNC(glShaderSource);
		GET_GL_FUNC(glCompileShader);
		GET_GL_FUNC(glGetShaderiv);
		GET_GL_FUNC(glGetShaderInfoLog);
		GET_GL_FUNC(glCreateProgram);
		GET_GL_FUNC(glAttachShader);
		GET_GL_FUNC(glLinkProgram);
		GET_GL_FUNC(glGetProgramiv);
		GET_GL_FUNC(glGetProgramInfoLog);
		GET_GL_FUNC(glDetachShader);
		GET_GL_FUNC(glDeleteShader);
		GET_GL_FUNC(glUseProgram);
		GET_GL_FUNC(glGetAttribLocation);
		GET_GL_FUNC(glDeleteProgram);
		GET_GL_FUNC(glDeleteBuffers);
		GET_GL_FUNC(glGetUniformLocation);
		GET_GL_FUNC(glUniformMatrix3fv);
		GET_GL_FUNC(glUniformMatrix4fv);
		GET_GL_FUNC(glGetBufferParameteriv);
		GET_GL_FUNC(glActiveTexture);
		GET_GL_FUNC(glUniform1i);
		GET_GL_FUNC(glUniform2i);
		GET_GL_FUNC(glUniform1f);
		GET_GL_FUNC(glUniform2f);
		GET_GL_FUNC(glUniform3f);
		GET_GL_FUNC(glUniform3fv);
		GET_GL_FUNC(glUniform4f);
		const int gl_api_init_end=__LINE__;
#undef	GET_GL_FUNC
#undef	GET_GL_FUNC_UNCHECKED
		const int n_functions=gl_api_decl_end-(gl_api_decl_start+1), n_initialized=gl_api_init_end-(gl_api_init_start+1);
		MY_ASSERT(n_functions==n_initialized, "Number of declared functions not equal to number of initialized functions.");
		if(GL2_state)//I don't know what I'm doing
			GL2_state=GL2_API_LOADED;
	}
}
#endif

//shader API
unsigned			CompileShader(const char *src, unsigned type, const char *programname=nullptr)
{
	unsigned shaderID=glCreateShader(type);
	glShaderSource(shaderID, 1, &src, 0);
	glCompileShader(shaderID);
	int success=0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		int infoLogLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> errorMessage(infoLogLength+1);
		glGetShaderInfoLog(shaderID, infoLogLength, 0, &errorMessage[0]);
		copy_to_clipboard_c(&errorMessage[0], infoLogLength);
		if(programname)
			LOG_ERROR("%s compilation failed. Output copied to cipboard.", programname);
		else
			GL_ERROR();
		return 0;
	}
	return shaderID;
}
unsigned			make_gl_program_impl(const char *vertSrc, const char *fragSrc, const char *programname=nullptr)
{
	unsigned
		vertShaderID=CompileShader(vertSrc, GL_VERTEX_SHADER, programname),
		fragShaderID=CompileShader(fragSrc, GL_FRAGMENT_SHADER, programname);
//	prof_add("compile sh");
	if(!vertShaderID||!fragShaderID)
	{
		GL_ERROR();
		return 0;
	}
	unsigned ProgramID=glCreateProgram();
	glAttachShader(ProgramID, vertShaderID);
	glAttachShader(ProgramID, fragShaderID);
	glLinkProgram(ProgramID);
//	prof_add("link");
	int success=0;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &success);
	if(!success)
	{
		int infoLogLength;
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> errorMessage(infoLogLength+1);
		glGetProgramInfoLog(ProgramID, infoLogLength, 0, &errorMessage[0]);
		copy_to_clipboard_c(&errorMessage[0], infoLogLength);
		if(programname)
			LOG_ERROR("%s compilation failed. Output copied to cipboard.", programname);
		else
			GL_ERROR();
		return 0;
	}
	glDetachShader(ProgramID, vertShaderID);
	glDetachShader(ProgramID, fragShaderID);
	glDeleteShader(vertShaderID);
	glDeleteShader(fragShaderID);
//	prof_add("delete");
	GL_CHECK();
	return ProgramID;
}
void				make_gl_program(ShaderProgram &p)
{
	p.program=make_gl_program_impl(p.vsrc, p.fsrc, p.name);
	for(int ka=0;ka<p.n_attr;++ka)
	{
		auto &attr=p.attributes[ka];
		if((*attr.pvar=glGetAttribLocation(p.program, attr.name))==-1)
			LOG_ERROR("%s: attribute %s == -1", p.name, attr.name);
	}
	for(int ku=0;ku<p.n_unif;++ku)
	{
		auto &unif=p.uniforms[ku];
		if((*unif.pvar=glGetUniformLocation(p.program, unif.name))==-1)
			LOG_ERROR("%s: uniform %s == -1", p.name, unif.name);
	}
}
unsigned			current_program=0;
void				setGLProgram(unsigned program)
{
	if(current_program!=program)
	{
		glUseProgram(current_program=program);
		GL_CHECK();
	}
}

void				send_color(unsigned location, int color)
{
	auto p=(unsigned char*)&color;

	static const __m128 m_255=_mm_set1_ps(inv255);
	__m128 c=_mm_castsi128_ps(_mm_set_epi32(p[3], p[2], p[1], p[0]));
	c=_mm_cvtepi32_ps(_mm_castps_si128(c));
	c=_mm_mul_ps(c, m_255);
	ALIGN(16) float comp[4];
	_mm_store_ps(comp, c);
	glUniform4f(location, comp[0], comp[1], comp[2], comp[3]);

	//glUniform4f(location, p[0]*inv255, p[1]*inv255, p[2]*inv255, p[3]*inv255);
	
	GL_CHECK();
}
void				send_color_rgb(unsigned location, int color)
{
	auto p=(unsigned char*)&color;
	static const __m128 m_255=_mm_set1_ps(inv255);
	__m128 c=_mm_castsi128_ps(_mm_set_epi32(p[3], p[2], p[1], p[0]));
	c=_mm_cvtepi32_ps(_mm_castps_si128(c));
	c=_mm_mul_ps(c, m_255);
	ALIGN(16) float comp[4];
	_mm_store_ps(comp, c);
	glUniform3f(location, comp[0], comp[1], comp[2]);
	GL_CHECK();
}
void				send_vec3(unsigned location, const float *v){glUniform3fv(location, 1, v);}
void				send_texture_pot(unsigned gl_texture, int *rgba, int txw, int txh)
{
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, txw, txh, 0, GL_RGBA,  GL_UNSIGNED_BYTE, rgba);
}
void				select_texture(unsigned tx_id, int u_location)
{
	glActiveTexture(GL_TEXTURE0);			GL_CHECK();
	glBindTexture(GL_TEXTURE_2D, tx_id);	GL_CHECK();//select texture
	glUniform1i(u_location, 0);				GL_CHECK();
}

ivec4				region, current_region;//x1, y1, x2, y2
void				set_region_immediate(int x1, int x2, int y1, int y2)
{
	region.set(x1, y1, x2-x1, y2-y1);
	glViewport(region.x1, region.y1, region.dx, region.dy);
}
void				resize_gl()
{
	glViewport(0, 0, w, h);//x, y, w, h
	current_region.set(0, 0, w, h);//x1, y1, x2, y2
}
void				get_current_region(int &x1, int &y1, int &dx, int &dy)
{
	x1=current_region.x1;
	y1=current_region.y1;
	dx=current_region.dx;
	dy=current_region.dy;
}
void				toNDC_nobias(float xs, float ys, float &xn, float &yn)
{
	xn=(xs-current_region.x1)*2.f/current_region.dx-1;//draw_rectangle_hollow: missing pixel in top-right corner
	yn=1-(ys-current_region.y1)*2.f/current_region.dy;
}
void				toNDC(float xs, float ys, float &xn, float &yn)//2021-01-14 test: fill the screen with horizontal or vertical lines
{
	xn=(xs+0.5f-current_region.x1)*2.f/current_region.dx-1;
	yn=1-(ys+0.5f-current_region.y1)*2.f/current_region.dy;
}