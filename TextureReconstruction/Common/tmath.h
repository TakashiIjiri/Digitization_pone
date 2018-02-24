#ifndef __TMATH_H_INCLUDED__
#define __TMATH_H_INCLUDED__

// -----------------------------------------------------------------------------
// Copyright(c) 2016, Takashi Ijiri
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// *Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
// * Neither the name of the <organization> nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// 
// TOGL provides OpenGL functions for MFC.
// 
// Start development at 2016/3/4
// This class uses Eigen (MPL2) without modification, OpenMesh (BSD), and glew (BSD)
// OpenMesh http://www.openmesh.org/license/
// Glew     http://glew.sourceforge.net/credits.html
// ------------------------------------------------------------------------------


#include <Eigen/Dense>
#include <Eigen/Geometry> 

typedef Eigen::Vector2i EVec2i;
typedef Eigen::Vector3i EVec3i;


typedef Eigen::Vector2d EVec2d;
typedef Eigen::Vector3d EVec3d;
typedef Eigen::Vector4d EVec4d;

typedef Eigen::Matrix3d EMat3d;
typedef Eigen::Matrix4d EMat4d;


inline void Trace(const EVec3d &v)
{
	fprintf(stderr, "%f %f %f\n", v[0], v[1], v[2]);
}

inline void Trace(const EVec2d &v)
{
	fprintf(stderr, "%f %f \n", v[0], v[1]);
}




//Mathematical functions -------------------------------------------------------

//	  | a b | |s|    w1
//    | c d | |t|  = w2
inline bool t_solve2by2LinearEquation
(
	const double a, const double b,
	const double c, const double d,
	const double w1, const double w2,
	double &s, double &t
	)
{
	double det = (a*d - b*c);
	if (det == 0) return false;
	det = 1.0 / det;
	s = (d*w1 - b*w2) * det;
	t = (-c*w1 + a*w2) * det;
	return true;
}


inline bool t_isInTriangle2D( const EVec3d &p0, const EVec3d &p1,const EVec3d &p2, const EVec3d P)
{
	// p = p0 + s * (p1-p0) + t * (p2-p0)
	//--> p-p0 = (p1-p0, p2-p0) (s,t)^T

	double s,t;
	t_solve2by2LinearEquation( p1[0] - p0[0], p2[0] - p0[0],
		                       p1[1] - p0[1], p2[1] - p0[1],   P[0]-p0[0], P[1]-p0[1],  s,t);
	return 0<=s && 0 <= t && s+t <= 1;
}



inline double t_distance2D_sq(const EVec3d &x1, const EVec3d &x2){
	return (x1[0] - x2[0]) * (x1[0] - x2[0]) + 
		   (x1[1] - x2[1]) * (x1[1] - x2[1]);
}
inline double t_distance2D( const EVec3d& x1, const EVec3d &x2){
	return sqrt( t_distance2D_sq(x1, x2) ); 
}



// ph = p0 + t * (lineP1 - lineP0) / |lineP1 - lineP0)|
inline double t_distPointToLineSegment2D( 
	const EVec3d &p , 
	const EVec3d &lineP0, 
	const EVec3d &lineP1,
	double &t)
{
	t =  (p[0] - lineP0[0]) * (lineP1[0] - lineP0[0]) + 
		 (p[1] - lineP0[1]) * (lineP1[1] - lineP0[1]);
	t /= t_distance2D_sq(lineP0,lineP1);

	if( t < 0 ) { t = 0; return t_distance2D( p, lineP0 );}
	if( t > 1 ) { t = 1; return t_distance2D( p, lineP1 );}

	double x = lineP0[0]  + t * (lineP1[0]-lineP0[0]) - p[0];
	double y = lineP0[1]  + t * (lineP1[1]-lineP0[1]) - p[1];

	return sqrt( x*x + y*y);
}












//http://www21.atwiki.jp/opengl/pages/130.html
inline void MyGlPerspective(
	const double &fovy  , 
	const double &aspect, 
	const double &cNear , 
	const double &cFar  , EMat4d &M)
{
	double tanFov = tan(fovy / 360.0 * M_PI);
	//double f = 1.0 / tan( (float)( (fovy * 0.5) / 180.0 * M_PI) );
	double a = 1.0 / (aspect* tanFov);
	double f = 1.0 / tanFov;
	double b = (cFar + cNear) / (cNear - cFar);
	double c = 2 * cFar*cNear / (cNear - cFar);
	M << a, 0, 0, 0,
		 0, f, 0, 0,
		 0, 0, b, c, 
		 0, 0,-1, 1;
}



inline void MyGlLookAt(
	const EVec3d &camP, 
	const EVec3d &camY, 
	const EVec3d &camF,
	EMat4d &M
	)
{
	EVec3d f = (camP - camF).normalized();
	EVec3d s = camY.cross(f).normalized();
	EVec3d u = f.cross(s)   .normalized();
	M << s[0], s[1], s[2], -s.dot(camP),
		 u[0], u[1], u[2], -u.dot(camP),
		 f[0], f[1], f[2], -f.dot(camP),
		 0, 0, 0, 1;
}



//returns pos on screen [-1,1]x[-1,1]
inline EVec3d t_Project(
	const double fovY,
	const double aspect,
	const double zNear,
	const double zFar,
	const EVec3d &camPos,
	const EVec3d &camUp,
	const EVec3d &camFocus, 
	const EVec3d &pos)
{
	EMat4d P, M;
	MyGlPerspective(fovY, aspect, zNear, zFar, P);
	MyGlLookAt   (camPos, camUp, camFocus    , M);

	EVec4d t1( pos[0], pos[1], pos[2], 1);
	t1 = P * M * t1;
	
	return EVec3d(t1[0] / t1[3], t1[1] / t1[3], t1[2] / t1[3]);
}


#ifndef MIN3
#define MIN3(  a,b,c)	((a)<(b)?((a)<(c)?(a):(c)):((b)<(c)?(b):(c)))
#define MAX3(  a,b,c)	((a)>(b)?((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))
#define MIN3ID(a,b,c)	((a)<(b)?((a)<(c)?(0):(2)):((b)<(c)?(1):(2)))
#define MAX3ID(a,b,c)	((a)>(b)?((a)>(c)?(0):(2)):((b)>(c)?(1):(2)))
#endif






#endif	// __TMATH_H_INCLUDED__