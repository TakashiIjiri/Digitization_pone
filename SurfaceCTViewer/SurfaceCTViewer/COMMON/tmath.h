#pragma once

#pragma unmanaged


using namespace std;


//Eigen 
#include <Dense>
#include <Geometry> 

//stl
#include <vector>

typedef Eigen::Vector2i EVec2i;
typedef Eigen::Vector2d EVec2d;
typedef Eigen::Vector2f EVec2f;

typedef Eigen::Vector3i EVec3i;
typedef Eigen::Vector3d EVec3d;
typedef Eigen::Vector3f EVec3f;

typedef Eigen::Vector4i EVec4i;
typedef Eigen::Vector4d EVec4d;
typedef Eigen::Vector4f EVec4f;


typedef Eigen::Matrix2d EMat2d;
typedef Eigen::Matrix3f EMat3f;
typedef Eigen::Matrix4f EMat4f;
typedef Eigen::Matrix3d EMat3d;
typedef Eigen::Matrix4d EMat4d;


#ifndef max3
#define max3(a,b,c)      max((a), max( (b),(c) ))
#endif

#ifndef min3
#define min3(a,b,c)      min((a), min( (b),(c) ))
#endif




inline void Trace(const EVec3d &v)
{
	fprintf(stderr, "%f %f %f\n", v[0], v[1], v[2]);
}

inline void Trace(const EVec2d &v)
{
	fprintf(stderr, "%f %f \n", v[0], v[1]);
}








inline float t_distRayToPoint(const EVec3f &rayP, const EVec3f &rayD, const EVec3f &p)
{
	float t = (p - rayP).dot(rayD) / rayD.dot(rayD);
	return (rayP + t * rayD - p).norm();
}


inline float t_distRayToPoint(const EVec3f &rayP, const EVec3f &rayD, const EVec3f &p, float &t)
{
	t = (p - rayP).dot(rayD) / rayD.dot(rayD);
	return (rayP + t * rayD - p).norm();
}


/*

inline double t_distRayToPointD(const EVec3d &rayP, const EVec3d &rayD, const EVec3d &p)
{
	double t = (p - rayP).dot(rayD) / rayD.dot(rayD);
	return (rayP + t * rayD - p).norm();
}

inline bool t_intersectRayToTriangle
(
	const EVec3d &rayP,
	const EVec3d &rayD,
	const EVec3d &x0,
	const EVec3d &x1,
	const EVec3d &x2,
	EVec3d &pos
	)
{
	Eigen::Matrix3d A;
	A << x1[0] - x0[0], x2[0] - x0[0], -rayD[0],
		x1[1] - x0[1], x2[1] - x0[1], -rayD[1],
		x1[2] - x0[2], x2[2] - x0[2], -rayD[2];

	EVec3d stu = A.inverse()*(rayP - x0);

	if (0 <= stu[0] && stu[0] <= 1 &&
		0 <= stu[1] && stu[1] <= 1 &&
		0 <= stu[0] + stu[1] && stu[0] + stu[1] <= 1)
	{
		pos = rayP + stu[2] * rayD;
		return true;
	}

	return false;
}
*/

inline bool t_intersectRayToTriangle
(
	const EVec3f &rayP,
	const EVec3f &rayD,
	const EVec3f &x0,
	const EVec3f &x1,
	const EVec3f &x2,
	EVec3f &pos
	)
{
	Eigen::Matrix3f A;
	A << x1[0] - x0[0], x2[0] - x0[0], -rayD[0],
		 x1[1] - x0[1], x2[1] - x0[1], -rayD[1],
		 x1[2] - x0[2], x2[2] - x0[2], -rayD[2];

	EVec3f stu = A.inverse()*(rayP - x0);

	if (0 <= stu[0] && stu[0] <= 1 &&
		0 <= stu[1] && stu[1] <= 1 &&
		0 <= stu[0] + stu[1] && stu[0] + stu[1] <= 1)
	{
		pos = rayP + stu[2] * rayD;
		return true;
	}

	return false;
}



inline bool t_intersectRayToQuad
(
	const EVec3f &rayP,
	const EVec3f &rayD,

	const EVec3f &x0,
	const EVec3f &x1,
	const EVec3f &x2,
	const EVec3f &x3,

	EVec3f &pos
	)
{
	if (t_intersectRayToTriangle(rayP, rayD, x0, x1, x2, pos)) return true;
	if (t_intersectRayToTriangle(rayP, rayD, x0, x2, x3, pos)) return true;
	return false;
}


template<class T>
void t_getMaxMinOfArray(const int N, const T* src, T& minV, T& maxV)
{	
	if( N == 0 ) return;

	minV = src[0];
	maxV = src[0];
	for (int i = 0; i < N; ++i)
	{
		if( src[i] < minV) minV = src[i];
		if( src[i] > maxV) maxV = src[i];
	}

}




template<class T>
inline T t_crop(const T &minV, const T &maxV, const T &v)
{
	return min(maxV, max(minV, v));
}




inline bool t_bInWindow3D(const EVec3f &minW, const EVec3f &maxW, const EVec3f &p, float offset = 0)
{
	return  minW[0] - offset <= p[0]  &&  p[0] <= maxW[0] + offset &&
			minW[1] - offset <= p[1]  &&  p[1] <= maxW[1] + offset &&
			minW[2] - offset <= p[2]  &&  p[2] <= maxW[2] + offset;

}






inline double t_dist(const EVec3d &p1, const EVec3d &p2)
{
	return sqrt( (p1[0] - p2[0]) * (p1[0] - p2[0]) +
		         (p1[1] - p2[1]) * (p1[1] - p2[1]) + 
		         (p1[2] - p2[2]) * (p1[2] - p2[2]) );
}


inline float t_dist(const EVec3f &p1, const EVec3f &p2)
{
	return sqrt( (p1[0] - p2[0]) * (p1[0] - p2[0]) +
		         (p1[1] - p2[1]) * (p1[1] - p2[1]) + 
		         (p1[2] - p2[2]) * (p1[2] - p2[2]) );
}









//calc length of a polyline
inline float t_verts_Length(const std::vector<EVec3f> &verts, bool bClosed = false)
{
	const int N = (int)verts.size();
    float d = 0;
    if( bClosed && N >= 2) d += t_dist( verts.back(), verts.front() );

    for( int i = 1; i < N; ++i ) d += t_dist( verts[i], verts[i-1] );
   
	return d;
}


//----------------------------------------------------------------------
// resampling a polyline into "n" section with equal interval
// This function returns n+1 points
// ---------------------------------------------------------------------
inline void t_verts_ResampleEqualInterval
(
    const int n,
    const std::vector<EVec3f> &verts,
          std::vector<EVec3f> &result
)
{
    result.clear();
    if( verts.size() < 2 ) {result.resize( n + 1); return;}

    const float stepD = t_verts_Length( verts  ) / (float) n;

    if( stepD == 0 ) {
        result.resize(n+1);
        return;
    }

    result.push_back( verts[0]);
    float distance = 0;

    EVec3f vec, pivot = verts[0];

    for( int index = 1 ; index < (int)verts.size();)
    {
        distance += t_dist( verts[index], pivot );

        if( distance >= stepD )//steo over
        {
            vec = pivot - verts[index];
            vec *= (distance - stepD) / vec.norm();

            pivot = verts[index] + vec;
            result.push_back( pivot );
            distance = 0;
        }
        else
        {
            pivot = verts[index];
            ++index;
        }
    }
    if( result.size() != n + 1) result.push_back( verts.back() );
}



inline void t_verts_Smoothing( std::vector<EVec3f> &verts )
{
	const int N = (int) verts.size();
    if( N < 2 ) return;

    std::vector<EVec3f> result = verts;

    for (int i = 1; i < N - 1; ++i) result[i] = 0.5 * verts[ i ] + 0.25 * verts[i-1] + 0.25 * verts[i+1];
    verts = result;
}


inline void t_verts_Smoothing(const int times, std::vector<EVec3f> &verts)
{
	for( int i=0; i < times; ++i ) t_verts_Smoothing( verts );
}




//	  | a b | |s|    w1
//    | c d | |t|  = w2
inline bool t_solve2by2LinearEquation(const double a,  const double b, 
									  const double c,  const double d,  const double w1, const double w2, 
									                                          double &s,       double &t)
{
	double det = (a*d - b*c); 
	if(det == 0) return false;
	det = 1.0 / det;
	s = (  d*w1 - b*w2 ) * det;
	t = ( -c*w1 + a*w2 ) * det;
	return true;
}

#pragma managed

