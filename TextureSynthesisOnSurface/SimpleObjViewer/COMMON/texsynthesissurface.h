#pragma once

#include "ttexmesh.h"

#include "timage.h"





//patch size
#define PATCH_R      10
#define PATCH_W      21 
#define PATCH_WW     PATCH_W*PATCH_W
#define PATCH_WW3    3*PATCH_WW
//#define PATCH_MID_I3 3*(PATCH_R*PATCH_W + PATCH_R)





TImage2D HoleRetrieval
(
	const TTexMesh       &mesh   , //input: mesh should be have tex coord
	const vector<EVec3d> &vFlow  , 
	const TImage2D       &trgtTex, //input: texture (0,0,255) is hole pixel
	const int      &smoothingNum,
	const double   &patchPitch
);


void t_getNeighboringPatch(
	const int     W		  , //texture atlas size
	const int     H		  , //texture atlas size
	const EVec3i &pixOnTex, //trgt pixel P(u,v)
	const int    &pixPolyI, //polygon ID correspondint to P
	const TTexMesh &mesh  ,
	const vector<EVec3d> &vFlow,
	const int    patchR,
	const double samplePitch, 

	int *patchUVidx //output: should be allocated (store v*W + u)
);




inline void t_getPatchByIdx( const TImage2D &texture, const int *patchIdx, byte *patchRGB)
{
	for (int i = 0; i < PATCH_WW; ++i) 
		if( patchIdx[i] != -1)
		{
			patchRGB[3*i+0] = texture[4*patchIdx[i]+0];
			patchRGB[3*i+1] = texture[4*patchIdx[i]+1];
			patchRGB[3*i+2] = texture[4*patchIdx[i]+2];
		}
		else
		{
			//fprintf( stderr, "error strange patch");
		}
}







void t_genPolygonIDtexture
(
	const TTexMesh &mesh,
	const int W,
	const int H,
	int *polyIdTex
);


 
template<class T>
inline void t_exportPatch( const int W, const T *patchRGB, const char* fname)
{
	const int WH = W*W;
	TImage2D img;
	img.AllocateImage(W,W,0);
	for( int i=0; i < WH; ++i) 
	{
		img[4*i+0] = (byte) patchRGB[3*i  ]; 
		img[4*i+1] = (byte) patchRGB[3*i+1]; 
		img[4*i+2] = (byte) patchRGB[3*i+2]; 
	}
	img.saveAsFile( fname, 0);
}

inline void t_exportPatch( const int W, const EVec3d col, const char* fname)
{
	const int WH = W*W;
	TImage2D img;
	img.AllocateImage(W,W,0);
	for( int i=0; i < WH; ++i) 
	{
		img[4*i+0] = (byte) (0.5 + col[0]); 
		img[4*i+1] = (byte) (0.5 + col[1]); 
		img[4*i+2] = (byte) (0.5 + col[2]); 
	}
	img.saveAsFile( fname, 0);
}
