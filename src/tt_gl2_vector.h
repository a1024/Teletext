#ifndef				TT_GL2_VECTOR_H
#define				TT_GL2_VECTOR_H
#include			<string.h>
#include			<math.h>
#include			<tmmintrin.h>

//OpenGL error handling
void 				gl_check(const char *file, int line);
#define				GL_CHECK()		gl_check(file, __LINE__)
void				gl_error(const char *file, int line);
#define				GL_ERROR()		gl_error(file, __LINE__)

inline int			floor_log2(unsigned n)
{
	int logn=0;
	int sh=(((short*)&n)[1]!=0)<<4;	logn^=sh, n>>=sh;	//21.54
		sh=(((char*)&n)[1]!=0)<<3;	logn^=sh, n>>=sh;
		sh=((n&0x000000F0)!=0)<<2;	logn^=sh, n>>=sh;
		sh=((n&0x0000000C)!=0)<<1;	logn^=sh, n>>=sh;
		sh=((n&0x00000002)!=0);		logn^=sh;
	return logn;
}
inline int			maximum(int x, int y){return (x+y+abs(x-y))>>1;}
//inline float		maximum(float x, float y){return (x+y+abs(x-y))*0.5f;}
inline char			maximum(char x1, char x2, char x3, char x4, char x5, char x6)
{
	int m1=(x1+x2+abs(x1-x2)), m2=(x3+x4+abs(x3-x4)), m3=(x5+x6+abs(x5-x6));
	int m4=(m1+m2+abs(m1-m2)), m5=m3<<1;
	return (m4+m5+abs(m4-m5))>>3;
}
inline int			clamp(int lo, int x, int hi)
{
	hi<<=1;
	int temp=x+lo+abs(x-lo);
	return (temp+hi-abs(temp-hi))>>2;
}
inline float		clamp(float lo, float x, float hi)
{
	hi+=hi;
	float temp=x+lo+abs(x-lo);
	return (temp+hi-abs(temp-hi))*0.25f;
}

extern const float	_pi, _2pi, pi_2, inv_2pi, sqrt2, torad, todeg, infinity, inv255, inv256, inv128;
inline void			update_angle(float &angle, float &ca, float &sa)
{
	angle-=floor(angle*inv_2pi)*_2pi;
	ca=cos(angle), sa=sin(angle);
}
inline void			reduce_angle(float &angle){angle-=floor(angle*inv_2pi)*_2pi;}
struct				vec2
{
	float x, y;
	vec2():x(0), y(0){}
	vec2(float x, float y):x(x), y(y){}
	void set(float x, float y){this->x=x, this->y=y;}
	vec2& operator+=(vec2 const &b){x+=b.x, y+=b.y; return *this;}
	vec2& operator-=(vec2 const &b){x-=b.x, y-=b.y; return *this;}
	vec2& operator+=(float x){this->x+=x, y+=x; return *this;}
	vec2& operator-=(float x){this->x-=x, y-=x; return *this;}
	vec2& operator*=(float x){this->x*=x, y*=x; return *this;}
	vec2& operator/=(float x){this->x/=x, y/=x; return *this;}
	float dot(vec2 const &other)const{return x*other.x+y*other.y;}
	float cross(vec2 const &other)const{return x*other.y-y*other.x;}
	float magnitude()const{return sqrt(x*x+y*y);}
	float mag_sq()const{return x*x+y*y;}
	float angle()const{return atan(y/x);}
	float angle2()const{return atan2(y, x);}
};
inline vec2			operator*(vec2 const &p, float x){return vec2(p.x*x, p.y*x);}
inline vec2			operator*(float x, vec2 const &p){return vec2(p.x*x, p.y*x);}
inline vec2			operator/(vec2 const &p, float x){return vec2(p.x/x, p.y/x);}
inline vec2			operator+(vec2 const &a, vec2 const &b){return vec2(a.x+b.x, a.y+b.y);}
inline vec2			operator-(vec2 const &a, vec2 const &b){return vec2(a.x-b.x, a.y-b.y);}
inline vec2			operator+(vec2 const &p, float x){return vec2(p.x+x, p.y+x);}
inline vec2			operator-(vec2 const &p, float x){return vec2(p.x-x, p.y-x);}
inline bool			operator==(vec2 const &a, vec2 const &b){return a.x==b.x&&a.y==b.y;}
inline bool			operator!=(vec2 const &a, vec2 const &b){return a.x!=b.x||a.y!=b.y;}
inline vec2			operator-(vec2 const &p){return vec2(-p.x, -p.y);}
struct				mat2
{
	float
		a, b,//row 0
		c, d;//row 1
	mat2():a(0), b(0), c(0), d(0){}
	mat2(float a, float b, float c, float d):a(a), b(b), c(c), d(d){}
};
vec2				operator*(mat2 const &m, vec2 const &v){return vec2(m.a*v.x+m.b*v.y, m.c*v.x+m.d*v.y);}
struct				vec3
{
	float x, y, z;
	vec3():x(0), y(0), z(0){}
	vec3(float x, float y, float z):x(x), y(y), z(z){}
	vec3(float gain):x(gain), y(gain), z(gain){}
	vec3(float *p):x(p[0]), y(p[1]), z(p[2]){}
	void set(float x, float y, float z){this->x=x, this->y=y, this->z=z;}
	void setzero(){x=y=z=0;}
	float& operator[](int idx){return (&x)[idx];}
	float operator[](int idx)const{return (&x)[idx];}
	vec3& operator+=(vec3 const &b){x+=b.x, y+=b.y, z+=b.z; return *this;}
	vec3& operator-=(vec3 const &b){x-=b.x, y-=b.y, z-=b.z; return *this;}
	vec3& operator+=(float x){this->x+=x, y+=x, z+=x; return *this;}
	vec3& operator-=(float x){this->x-=x, y-=x, z-=x; return *this;}
	vec3& operator*=(float x){this->x*=x, y*=x, z*=x; return *this;}
	vec3& operator/=(float x){this->x/=x, y/=x, z/=x; return *this;}
	float dot(vec3 const &other)const{return x*other.x+y*other.y+z*other.z;}
	vec3 cross(vec3 const &other)const{return vec3(y*other.z-z*other.y, z*other.x-x*other.z, x*other.y-y*other.x);}
	vec3 triple_product(vec3 const &b, vec3 const &c)const;
	float magnitude()const{return sqrt(x*x+y*y+z*z);}
	float mag_sq()const{return x*x+y*y+z*z;}
	float theta()const{return atan(z/sqrt(x*x+y*y));}//vertical angle
	//float theta2()const{return atan2(y, x);}
	float phi(){return atan(y/x);}//horizontal angle
	float phi2(){return atan2(y, x);}
	bool isnan(){return x!=x||y!=y||z!=z;}
	bool isnan_or_inf(){return x!=x||y!=y||z!=z||abs(x)==infinity||abs(y)==infinity||abs(z)==infinity;}
	void clamp_xy(float max_mag)
	{
		float mag=sqrt(x*x+y*y), mag2=clamp(0.f, mag, max_mag)/mag;
		x*=mag2, y*=mag2;
	}
};
inline vec3			operator*(vec3 const &p, float x){return vec3(p.x*x, p.y*x, p.z*x);}
inline vec3			operator*(float x, vec3 const &p){return vec3(p.x*x, p.y*x, p.z*x);}
inline vec3			operator/(vec3 const &p, float x){return vec3(p.x/x, p.y/x, p.z/x);}
inline vec3			operator+(vec3 const &a, vec3 const &b){return vec3(a.x+b.x, a.y+b.y, a.z+b.z);}
inline vec3			operator-(vec3 const &a, vec3 const &b){return vec3(a.x-b.x, a.y-b.y, a.z-b.z);}
inline vec3			operator+(vec3 const &p, float x){return vec3(p.x+x, p.y+x, p.z+x);}
inline vec3			operator-(vec3 const &p, float x){return vec3(p.x-x, p.y-x, p.z-x);}
inline bool			operator==(vec3 const &a, vec3 const &b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
inline bool			operator!=(vec3 const &a, vec3 const &b){return a.x!=b.x||a.y!=b.y||a.z!=b.z;}
inline vec3			operator-(vec3 const &p){return vec3(-p.x, -p.y, -p.z);}
vec3				vec3::triple_product(vec3 const &b, vec3 const &c)const{return this->dot(c)*b-this->dot(b)*c;}
vec3				normalize(vec3 const &v){float invm=1/v.magnitude(); return invm*v;}
struct				mat3
{
	vec3	c[3];
	mat3(){}
	mat3(float gain){memset(c, 0, 9<<2), c[0][0]=c[1][1]=c[2][2]=gain;}
	mat3(vec3 const &c0, vec3 const &c1, vec3 const &c2){c[0]=c0, c[1]=c1, c[2]=c2;}
	float* data(){return &c[0][0];}
};
struct				vec4
{
	union
	{
		struct{float x, y, z, w;};
		struct{float r, g, b, a;};
	};
	vec4(){memset(&x, 0, 4<<2);}
	vec4(float gain):x(gain), y(gain), z(gain), w(gain){}
	vec4(float x, float y, float z, float w):x(x), y(y), z(z), w(w){}
	vec4(vec3 const &v, float w):x(v.x), y(v.y), z(v.z), w(w){}
	vec4(__m128 const &v){_mm_storeu_ps(&x, v);}
	float& operator[](int idx){return (&x)[idx];}
	float operator[](int idx)const{return (&x)[idx];}
	operator __m128(){return _mm_loadu_ps(&x);}
	operator vec3(){return vec3(x, y, z);}
	void set(float x, float y, float z, float w){this->x=x, this->y=y, this->z=z, this->w=w;}
	void setzero(){_mm_storeu_ps(&x, _mm_setzero_ps());}
	float dot(vec4 const &other)
	{
		__m128 a=_mm_loadu_ps(&x), b=_mm_loadu_ps(&other.x);
		a=_mm_mul_ps(a, b);
		a=_mm_hadd_ps(a, a);
		a=_mm_hadd_ps(a, a);
		float result;
		_mm_store_ss(&result, a);//does not need to be aligned
		return result;
	}
};
inline vec4			operator+(vec4 const &a, vec4 const &b)
{
	__m128 va=_mm_loadu_ps(&a.x), vb=_mm_loadu_ps(&b.x);
	va=_mm_add_ps(va, vb);
	vec4 r;
	_mm_storeu_ps(&r.x, va);
	return r;
}
inline vec4			operator*(vec4 const &v, float s)
{
	__m128 vv=_mm_loadu_ps(&v.x), vs=_mm_set1_ps(s);
	vv=_mm_mul_ps(vv, vs);
	vec4 r;
	_mm_storeu_ps(&r.x, vv);
	return r;
}
inline vec4			operator*(float s, vec4 const &v){return v*s;}
struct				mat4//column major
{
	vec4	c[4];
	mat4(){}
	mat4(const float *v){memcpy(c, v, 16<<2);}
	mat4(float gain){memset(c, 0, 16<<2), c[0][0]=c[1][1]=c[2][2]=c[3][3]=gain;}
	mat4(vec4 const &c0, vec4 const &c1, vec4 const &c2, vec4 const &c3){c[0]=c0, c[1]=c1, c[2]=c2, c[3]=c3;}
	mat4(
		float a11, float a12, float a13, float a14,
		float a21, float a22, float a23, float a24,
		float a31, float a32, float a33, float a34,
		float a41, float a42, float a43, float a44)
	{
		c[0].set(a11, a21, a31, a41);
		c[1].set(a12, a22, a32, a42);
		c[2].set(a13, a23, a33, a43);
		c[3].set(a14, a24, a34, a44);
	}
	void set(
		float a11, float a12, float a13, float a14,
		float a21, float a22, float a23, float a24,
		float a31, float a32, float a33, float a34,
		float a41, float a42, float a43, float a44)
	{
		c[0].set(a11, a21, a31, a41);
		c[1].set(a12, a22, a32, a42);
		c[2].set(a13, a23, a33, a43);
		c[3].set(a14, a24, a34, a44);
	}
	operator mat3(){return mat3((vec3)c[0], (vec3)c[1], (vec3)c[2]);}
	float* data(){return &c[0][0];}
	const float* data()const{return (float*)c;}
	void setzero(){memset(c, 0, 16<<2);}
	//void setrow(int idx, vec4 const &v){float *p=&c[0][0]+idx; p[0]=v.x, p[4]=v.y, p[8]=v.z, p[12]=v.w;}
	vec4& operator[](int idx){return c[idx];}
	vec4 operator[](int idx)const{return c[idx];}
};
inline mat4			transpose(mat4 const &m)
{
	__m128 r0=_mm_loadu_ps(&m[0][0]), r1=_mm_loadu_ps(&m[1][0]), r2=_mm_loadu_ps(&m[2][0]), r3=_mm_loadu_ps(&m[3][0]);
	_MM_TRANSPOSE4_PS(r0, r1, r2, r3);
	mat4 m2;
	_mm_storeu_ps(&m2[0][0], r0);
	_mm_storeu_ps(&m2[1][0], r1);
	_mm_storeu_ps(&m2[2][0], r2);
	_mm_storeu_ps(&m2[3][0], r3);
	return m2;
}
inline vec4			operator*(mat4 const &m, vec4 const &v)
{
	__m128 c0=_mm_loadu_ps(&m[0][0]), c1=_mm_loadu_ps(&m[1][0]), c2=_mm_loadu_ps(&m[2][0]), c3=_mm_loadu_ps(&m[3][0]);
	//	vv=_mm_loadu_ps(&v.x);
	c0=_mm_mul_ps(c0, _mm_set1_ps(v.x));
	c1=_mm_mul_ps(c1, _mm_set1_ps(v.y));
	c2=_mm_mul_ps(c2, _mm_set1_ps(v.z));
	c3=_mm_mul_ps(c3, _mm_set1_ps(v.w));
	c0=_mm_add_ps(c0, c1);
	c0=_mm_add_ps(c0, c2);
	c0=_mm_add_ps(c0, c3);
	vec4 r;
	_mm_storeu_ps(&r[0], c0);
	return r;
}
inline mat4			operator*(mat4 const &a, mat4 const &b)
{
	mat4 c=transpose(a);
	float d[]=
	{
		c[0].dot(b[0]), c[0].dot(b[1]), c[0].dot(b[2]), c[0].dot(b[3]),
		c[1].dot(b[0]), c[1].dot(b[1]), c[1].dot(b[2]), c[1].dot(b[3]),
		c[2].dot(b[0]), c[2].dot(b[1]), c[2].dot(b[2]), c[2].dot(b[3]),
		c[3].dot(b[0]), c[3].dot(b[1]), c[3].dot(b[2]), c[3].dot(b[3])
	};
	return mat4(
		vec4(c[0].dot(b[0]), c[1].dot(b[0]), c[2].dot(b[0]), c[3].dot(b[0])),
		vec4(c[0].dot(b[1]), c[1].dot(b[1]), c[2].dot(b[1]), c[3].dot(b[1])),
		vec4(c[0].dot(b[2]), c[1].dot(b[2]), c[2].dot(b[2]), c[3].dot(b[2])),
		vec4(c[0].dot(b[3]), c[1].dot(b[3]), c[2].dot(b[3]), c[3].dot(b[3])));
}
inline mat4			translate(mat4 const &m, vec3 const &delta)//from glm
{
	vec4 v2(delta, 1);
	mat4 r=m;
	r[3]=m[0]*v2[0]+m[1]*v2[1]+m[2]*v2[2]+m[3];
	return r;
}
inline mat4			rotate(mat4 const &m, float angle, vec3 const &dir)//from glm
{
	float ca=cos(angle), sa=sin(angle);
	vec3 axis=normalize(dir), temp=(1-ca)*axis;
	mat4 rotate(
		vec4(ca+temp[0]*axis[0],				temp[0]*axis[1]+sa*axis[2],		temp[0]*axis[2]-sa*axis[1],	0),//col 1
		vec4(	temp[1]*axis[0]-sa*axis[2],	ca+	temp[1]*axis[1],				temp[1]*axis[2]+sa*axis[0],	0),
		vec4(	temp[2]*axis[0]+sa*axis[1],		temp[2]*axis[1]-sa*axis[0],	ca+	temp[2]*axis[2],			0),
		vec4(0, 0, 0, 1));//col 4
	return m*rotate;
}
inline mat4			scale(mat4 const &m, vec3 const &ammount)
{
	mat4 r(m[0]*ammount.x, m[1]*ammount.y, m[2]*ammount.z, m[3]);
	return r;
}
inline mat4			lookAt(vec3 const &cam, vec3 const &center, vec3 const &up)//from glm
{
	vec3 f=normalize(center-cam),
		u=normalize(up), s=normalize(f.cross(u));
	u=s.cross(f);
	mat4 r(
		vec4(s, -s.dot(cam)),
		vec4(u, -u.dot(cam)),
		vec4(-f, f.dot(cam)),
		vec4(0, 0, 0, 1));
	r=transpose(r);
	return r;
}
inline mat4			matrixFPSViewRH(vec3 const &_cam, float pitch, float yaw)
{//https://www.3dgep.com/understanding-the-view-matrix/
	vec3 cam(_cam.y, _cam.z, _cam.x);//why yzx?
	float cos_p=cos(pitch), sin_p=sin(pitch), cos_y=cos(yaw), sin_y=sin(yaw);
	vec3
			xaxis(cos_y, 0, -sin_y),
			yaxis(sin_y*sin_p, cos_p, cos_y*sin_p),
			zaxis(sin_y*cos_p, -sin_p, cos_p*cos_y);
	
	return mat4(
		vec4(xaxis.z, yaxis.z, zaxis.z, 0),//why zxy?
		vec4(xaxis.x, yaxis.x, zaxis.x, 0),
		vec4(xaxis.y, yaxis.y, zaxis.y, 0),
		vec4(-xaxis.dot(cam), -yaxis.dot(cam), -zaxis.dot(cam), 1));
}
inline mat4			perspective(float tanfov, float ar, float znear, float zfar)
{
	return mat4(
		vec4(1/tanfov, 0, 0, 0),
		vec4(0, ar/tanfov, 0, 0),
		vec4(0, 0, -(zfar+znear)/(zfar-znear), -1),
		vec4(0, 0, -2*zfar*znear/(zfar-znear), 0));
}
struct				ivec4
{
	union
	{
		struct{int x, y, z, w;};
		struct{int x1, y1, dx, dy;};
	};
	ivec4(){_mm_storeu_si128((__m128i*)&x, _mm_setzero_si128());}
	ivec4(int x, int y, int z, int w):x(x), y(y), z(z), w(w){}
	void set(int x, int y, int z, int w){this->x=x, this->y=y, this->z=z, this->w=w;}
};
inline mat4			GetTransformInverseNoScale(const mat4 &inM)// Requires this matrix to be transform matrix, NoScale version requires this matrix be of scale 1
{
#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))

// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)

// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)
	mat4 r;

	// transpose 3x3, we know m03 = m13 = m23 = 0
	__m128 t0 = VecShuffle_0101(inM[0], inM[1]); // 00, 01, 10, 11
	__m128 t1 = VecShuffle_2323(inM[0], inM[1]); // 02, 03, 12, 13
	r[0] = VecShuffle(t0, inM[2], 0,2,0,3); // 00, 10, 20, 23(=0)
	r[1] = VecShuffle(t0, inM[2], 1,3,1,3); // 01, 11, 21, 23(=0)
	r[2] = VecShuffle(t1, inM[2], 0,2,2,3); // 02, 12, 22, 23(=0)

	// last line
	r[3] =                  _mm_mul_ps(r[0], VecSwizzle1(inM[3], 0));
	r[3] = _mm_add_ps(r[3], _mm_mul_ps(r[1], VecSwizzle1(inM[3], 1)));
	r[3] = _mm_add_ps(r[3], _mm_mul_ps(r[2], VecSwizzle1(inM[3], 2)));
	r[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), r[3]);

	return r;
#undef MakeShuffleMask
#undef VecSwizzleMask
#undef VecSwizzle
#undef VecSwizzle1
#undef VecSwizzle_0022
#undef VecSwizzle_1133
#undef VecShuffle
#undef VecShuffle_0101
#undef VecShuffle_2323
}
inline mat3			normalMatrix(mat4 const &m)//inverse transpose of top left 3x3 submatrix
{
	mat4 r=GetTransformInverseNoScale(m);
	return (mat3)transpose(r);
}
#endif//TT_GL2_VECTOR_H