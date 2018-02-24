#include "stdafx.h"

#include "texsynthesissurface.h"
#include "expmap.h"

#include <set>
#include "SimpleObjViewerView.h"



#ifndef MIN3
#define MIN3(  a,b,c)	((a)<(b)?((a)<(c)?(a):(c)):((b)<(c)?(b):(c)))
#define MAX3(  a,b,c)	((a)>(b)?((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))
#define MIN3ID(a,b,c)	((a)<(b)?((a)<(c)?(0):(2)):((b)<(c)?(1):(2)))
#define MAX3ID(a,b,c)	((a)>(b)?((a)>(c)?(0):(2)):((b)>(c)?(1):(2)))
#endif


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



bool t_isInTriangle2D( 
	const EVec2d &p0, 
	const EVec2d &p1,
	const EVec2d &p2, 
	const EVec2d &P )
{
	// p = p0 + s * (p1-p0) + t * (p2-p0)
	//--> p-p0 = (p1-p0, p2-p0) (s,t)^T

	double s,t;
	t_solve2by2LinearEquation( p1[0] - p0[0], p2[0] - p0[0],
		                       p1[1] - p0[1], p2[1] - p0[1],   P[0]-p0[0], P[1]-p0[1],  s,t);
	return 0<=s && 0 <= t && s+t <= 1;
}



// generate Polygon ID texture
// 
// mesh       : surface model with uv on each vertex
// int W, H   : 
// polyIdxImg : int[W*H] image painted by texture ids
void t_genPolygonIDtexture
(
	const TTexMesh &mesh,
	const int W,
	const int H,
	int *polyIdTex
)
{
	const double pX = 1.0 / W;
	const double pY = 1.0 / H;

	for( int i=0,WH=W*H; i < WH; ++i) polyIdTex[i] = -1;

	for( int pi=0; pi < mesh.m_pSize; ++pi)
	{
		const int   *idx = mesh.m_polys[ pi ].tIdx; 
		const EVec2d &uv0 = mesh.m_uvs[ idx[0] ];
		const EVec2d &uv1 = mesh.m_uvs[ idx[1] ];
		const EVec2d &uv2 = mesh.m_uvs[ idx[2] ];

		int minU = max(  0, (int)( MIN3( uv0[0], uv1[0], uv2[0]) * W ) - 1);
		int minV = max(  0, (int)( MIN3( uv0[1], uv1[1], uv2[1]) * H ) - 1);
		int maxU = min(W-1, (int)( MAX3( uv0[0], uv1[0], uv2[0]) * W ) + 1);
		int maxV = min(H-1, (int)( MAX3( uv0[1], uv1[1], uv2[1]) * H ) + 1);

		//fprintf( stderr, "%d %d %d %d\n", minU, minV,maxU, maxV );

		for( int u = minU; u < maxU; ++u)
		{
			for( int v = minV; v < maxV; ++v)
			{
				EVec2d p( (u+0.5) / W, (v+0.5)/H);
				if( t_isInTriangle2D( uv0, uv1, uv2, p) )
				{
					polyIdTex[u + v*W] = pi;
				}
			}
		}
	}

	//update 2015/12/20
	//thicken one-ring pixels (N-times)
	const int N = 3;
	for (int i = 0; i < N; ++i)
	{
		for (int y = 0; y < H; ++y)
		{
			for (int x = 0; x < W; ++x)
			{
				int I = x + y * W;
				if (polyIdTex[I] != -1) continue;
				if (x > 0     && polyIdTex[I - 1] >= 0) { polyIdTex[I] = -2; continue; }
				if (y > 0     && polyIdTex[I - W] >= 0) { polyIdTex[I] = -2; continue; }
				if (x < W - 1 && polyIdTex[I + 1] >= 0) { polyIdTex[I] = -2; continue; }
				if (y < H - 1 && polyIdTex[I + W] >= 0) { polyIdTex[I] = -2; continue; }
			}
		}

		for (int y = 0; y < H; ++y)
		{
			for (int x = 0; x < W; ++x)
			{
				int I = x + y * W;
				if (polyIdTex[I] != -2) continue;
				if (x > 0     && polyIdTex[I - 1] >= 0) { polyIdTex[I] = polyIdTex[I - 1]; continue; }
				if (y > 0     && polyIdTex[I - W] >= 0) { polyIdTex[I] = polyIdTex[I - W]; continue; }
				if (x < W - 1 && polyIdTex[I + 1] >= 0) { polyIdTex[I] = polyIdTex[I + 1]; continue; }
				if (y < H - 1 && polyIdTex[I + W] >= 0) { polyIdTex[I] = polyIdTex[I + W]; continue; }
			}
		}
	}



	//debug -- export texture --
	TImage2D forDebug;
	forDebug.AllocateImage( W,H, 0);
	for( int i=0,WH=W*H; i < WH; ++i) 
	{
		int pi = polyIdTex[i];
		if(      pi == -1)    forDebug.setPix( 4*i, 0  ,0  ,255,0);
		else if( pi % 5 == 0) forDebug.setPix( 4*i, 255,0  ,0  ,0);
		else if( pi % 5 == 1) forDebug.setPix( 4*i, 0  ,255,0  ,0);
		else if( pi % 5 == 2) forDebug.setPix( 4*i, 64 ,255,0  ,0);
		else if( pi % 5 == 3) forDebug.setPix( 4*i, 255,255,0  ,0);
		else                  forDebug.setPix( 4*i, 128,128,0  ,0);
	}

	forDebug.FlipInY();
	forDebug.saveAsFile( "idRefImg.bmp", 0);
}









inline double t_distance2D_sq(const EVec2d &x1, const EVec2d &x2){
	return (x1[0] - x2[0]) * (x1[0] - x2[0]) + 
		   (x1[1] - x2[1]) * (x1[1] - x2[1]);
}
inline double t_distance2D( const EVec2d& x1, const EVec2d &x2){
	return sqrt( t_distance2D_sq(x1, x2) ); 
}



// ph = p0 + t * (lineP1 - lineP0) / |lineP1 - lineP0)|
inline double t_distPointToLineSegment2D( 
	const EVec2d &p , 
	const EVec2d &lineP0, 
	const EVec2d &lineP1,
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


/*
  pixel p1 is inside the triangle 
  pixel p2 run out from the triangle

      seam edge
	      e
          |
          |
   poly1  |   poly2  
      p1  -   p2  
(inside)  |  (outside)
          |
          |
  
  map p2 considering seam on the atlas
*/



EVec3i t_getNeighbors(
	const int W,
	const int H,
	const TTexMesh &mesh,
	const int      *polyIdTex,
	const EVec3i   &p1, 
	const EVec3i   &p2 )
{

	const int   polyI = polyIdTex[p1[2]];
	if (polyI == -1)
	{
		fprintf( stderr, "never comes here\n");
		return EVec3i(-1,-1,-1);
	}

	const TPoly &poly1 = mesh.m_polys[polyI];

	//get closest edge
	const EVec2d &uv0 = mesh.m_uvs[ poly1.tIdx[0] ];
	const EVec2d &uv1 = mesh.m_uvs[ poly1.tIdx[1] ];
	const EVec2d &uv2 = mesh.m_uvs[ poly1.tIdx[2] ];


	EVec2d pixPos( (p1[0]+0.5) / W, (p1[1]+0.5)/H);
	double t0,t1,t2;
	double d0 = t_distPointToLineSegment2D( pixPos, uv0, uv1, t0 );
	double d1 = t_distPointToLineSegment2D( pixPos, uv1, uv2, t1 );
	double d2 = t_distPointToLineSegment2D( pixPos, uv2, uv0, t2 );

	//k = 0(01), 1(12), 2(20)
	int k = ( d0<= d1 && d0 <= d2 && mesh.m_edges[ poly1.edge[0] ].bAtlsSeam ) ? 0 : 
		    ( d1<= d0 && d1 <= d2 && mesh.m_edges[ poly1.edge[1] ].bAtlsSeam ) ? 1 : 
			(                        mesh.m_edges[ poly1.edge[2] ].bAtlsSeam ) ? 2 : -1;
	if( k == -1 ) return EVec3i(-1,-1,-1);


	const int poly1v0 = poly1.vIdx[ k       ];
	const int poly1v1 = poly1.vIdx[ (k+1)%3 ];


	const TEdge &e     = mesh.m_edges[ poly1.edge[k] ];
	const TPoly &poly2 = mesh.m_polys[ (polyI != e.p[0]) ? e.p[0] : e.p[1] ];

	//poly1が左回りなので poly 2は右回り
	const EVec2d &p2uv0 = mesh.m_uvs[ poly2.tIdx[0] ];
	const EVec2d &p2uv1 = mesh.m_uvs[ poly2.tIdx[1] ];
	const EVec2d &p2uv2 = mesh.m_uvs[ poly2.tIdx[2] ];

	EVec2d uv(0,0);
	if      ( poly1v0 == poly2.vIdx[1] && poly1v1 == poly2.vIdx[0] ) { uv = t0 * (p2uv0 - p2uv1 ) + p2uv1; }
	else if ( poly1v0 == poly2.vIdx[2] && poly1v1 == poly2.vIdx[1] ) { uv = t1 * (p2uv1 - p2uv2 ) + p2uv2; }
	else if ( poly1v0 == poly2.vIdx[0] && poly1v1 == poly2.vIdx[2] ) { uv = t2 * (p2uv2 - p2uv0 ) + p2uv0; }
	else
	{
		fprintf( stderr, "strange configuration %d %d %d,, %d %d %d\n", poly1.vIdx[0], poly1.vIdx[1], poly1.vIdx[2]
			                                                          , poly2.vIdx[0], poly2.vIdx[1], poly2.vIdx[2] );
		return EVec3i(-1,-1,-1);
	}

	int u = (int)(uv[0] * W);
	int v = (int)(uv[1] * H);
	int I = u + v * W;
	if( polyIdTex[I] != -1 ) return EVec3i(u,v,I);
	return EVec3i(-1,-1,-1);
}



void t_getNeighbors(
	const int W,
	const int H,
	const TTexMesh &mesh,
	const int      *polyIdTex,
	const EVec3i   &p, 
	EVec3i   &n1, 
	EVec3i   &n2, 
	EVec3i   &n3,
	EVec3i   &n4)
{
	n1 << -1,-1,-1;
	n2 << -1,-1,-1;
	n3 << -1,-1,-1;
	n4 << -1,-1,-1;

	if (0 <= p[0] )
	{
		n1 << p[0]-1, p[1], p[2]-1;
		if( polyIdTex[n1[2]] == -1 )  n1 = t_getNeighbors(W,H, mesh, polyIdTex, p, n1 );
	}	
	if (p[0] < W )
	{
		n2 << p[0]+1, p[1], p[2]+1;
		if( polyIdTex[n2[2]] == -1 )  n2 = t_getNeighbors(W,H, mesh, polyIdTex, p, n2 );
	}

	if (0 <= p[1] )
	{
		n3 << p[0], p[1]-1, p[2]-W;
		if( polyIdTex[n3[2]] == -1 )  n3 = t_getNeighbors(W,H, mesh, polyIdTex, p, n3 );
	}

	if (p[1] < H)
	{
		n4 << p[0], p[1]+1, p[2]+W;
		if( polyIdTex[n4[2]] == -1 )  n4 = t_getNeighbors(W,H, mesh, polyIdTex, p, n4 );
	}

}


class Neighbors { public: EVec3i n[4]; };

static void t_getNeighbors(
	const int W,
	const int H,
	const TTexMesh &mesh,
	const int      *polyIdTex,
	const EVec3i   &p,
	Neighbors &Ns)
{
	t_getNeighbors(W,H,mesh,polyIdTex,p,Ns.n[0], Ns.n[1], Ns.n[2], Ns.n[3]);
}



//       
// double   0    1/4   2/4   3/4   1.0 
//  int     |  0  |  1  |  2  |  3  |   N = 4  
//
// int    i --> (i + 0.5) / N
// double x --> x * N - 0.5 --> (int)(x * N)  





EVec3d t_mapPoint_UVto3D(const TTexMesh &mesh, const int trgtPolyIdx, const EVec2d &uv)
{
	const int    *tidx = mesh.m_polys[trgtPolyIdx].tIdx;
	const EVec2d &p0   = mesh.m_uvs[tidx[0]];
	const EVec2d &p1   = mesh.m_uvs[tidx[1]];
	const EVec2d &p2   = mesh.m_uvs[tidx[2]];

	double s, t;
	t_solve2by2LinearEquation(p1[0] - p0[0], p2[0] - p0[0], 
		                      p1[1] - p0[1], p2[1] - p0[1], 
		                      uv[0] - p0[0], uv[1] - p0[1], s, t);
	
	const int   *vidx = mesh.m_polys[trgtPolyIdx].vIdx;
	const EVec3d &v0 = mesh.m_verts[vidx[0]];
	const EVec3d &v1 = mesh.m_verts[vidx[1]];
	const EVec3d &v2 = mesh.m_verts[vidx[2]];
	return v0 + s * (v1-v0) + t * (v2-v0);
}




// polyIに対応する点の周囲にexpmapを展開
// exp map上で (u,v)を等間隔にサンプリング（間隔はsampePitch）
// パッチを生成する
//sampling texture patch around pixel by using exponential mapping
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
)
{
	const int patchW = 2 * patchR + 1;

	//consider only 4 ring triangles
	set<int> trgtPs;
	mesh.getNringPs(pixPolyI,4, trgtPs );

	byte *vtxFlg = new byte[mesh.m_vSize];
	memset( vtxFlg, 0, sizeof(byte)*mesh.m_vSize);
	for (auto p : trgtPs)
	{
		vtxFlg[mesh.m_polys[p].vIdx[0]] = 
		vtxFlg[mesh.m_polys[p].vIdx[1]] = 
		vtxFlg[mesh.m_polys[p].vIdx[2]] = 1;
	}

	//pixOnTex --> 3D Point 
	EVec2d pixUV( (pixOnTex[0] + 0.5) / W, (pixOnTex[1] + 0.5) / H );
	EVec3d pos3D = t_mapPoint_UVto3D(mesh, pixPolyI, pixUV);

	//compute exponential Mapping
	vector< ExpMapVtx > expMapVs;
	expnentialMapping( mesh, vFlow, pos3D, pixPolyI, DBL_MAX, vtxFlg, expMapVs);



	//sampling pixel by using exp map 
	for( int v = -patchR; v <= patchR; ++v)
	for (int u = -patchR; u <= patchR; ++u)
	{
		int patchI = u + patchR + (v + patchR) * patchW;
		patchUVidx[patchI] = -1;

		//UV on exp map
		EVec3d UV( u*samplePitch, v*samplePitch, 0);
			
		for (auto pi : trgtPs)
		{
			const int *vidx = mesh.m_polys[pi].vIdx;
			const EVec2d &p0 = expMapVs[vidx[0]].pos;
			const EVec2d &p1 = expMapVs[vidx[1]].pos;
			const EVec2d &p2 = expMapVs[vidx[2]].pos;

			double s,t;
			if( !t_solve2by2LinearEquation( p1[0] - p0[0], p2[0] - p0[0],
									        p1[1] - p0[1], p2[1] - p0[1], UV[0]-p0[0], UV[1]-p0[1],  s,t) ) continue;

			if( s < 0 || t < 0 || 1 < s + t ) continue;

			const int *tidx = mesh.m_polys[pi].tIdx;

			//UV exp map --> uv in texture atlas
			const EVec2d &uv0 = mesh.m_uvs[tidx[0]];
			const EVec2d &uv1 = mesh.m_uvs[tidx[1]];
			const EVec2d &uv2 = mesh.m_uvs[tidx[2]];
			double up = uv0[0] + s * (uv1[0]-uv0[0]) + t * (uv2[0]-uv0[0]);
			double vp = uv0[1] + s * (uv1[1]-uv0[1]) + t * (uv2[1]-uv0[1]);
			int ui = (int)(up * W );
			int vi = (int)(vp * H );
			if (0 <= ui && ui < W && 0 <= vi && vi < H) patchUVidx[patchI] = vi*W + ui;
			break;
		}
	}

	delete[] vtxFlg;
}








static void HoleFillByBoundaryGrow
(
	const vector< Neighbors > &pix_Ns, // 4-neighbors at each target pixel

	int       *flgImg, //input&output flg( -1:nottrgt, 0:valueExist, 1:target, 2:target and fixed)
	TImage2D  &texture //input&output texture
)
{
	const int W  = texture.m_W;
	const int H  = texture.m_H;
	const int WH = W*H;



	//extract all boundary pixel
	list<EVec3i> Q;
	for( int i=0; i < WH; ++i) if( flgImg[i] == 1)
	{
		const int x=i%W, y = i/W;
		const Neighbors &N = pix_Ns[i];

		bool tf = false;
		EVec4i rgba(0,0,0,0);
		if      (N.n[0][0] != -1 && flgImg[N.n[0][2]] == 0 ) { tf=true; rgba = texture.getRGBA(4* N.n[0][2]);}
		else if (N.n[1][0] != -1 && flgImg[N.n[1][2]] == 0 ) { tf=true; rgba = texture.getRGBA(4* N.n[1][2]);}
		else if (N.n[2][0] != -1 && flgImg[N.n[2][2]] == 0 ) { tf=true; rgba = texture.getRGBA(4* N.n[2][2]);}
		else if (N.n[3][0] != -1 && flgImg[N.n[3][2]] == 0 ) { tf=true; rgba = texture.getRGBA(4* N.n[3][2]);}
		if( !tf ) continue;

		texture.setPix( 4*i, rgba);
		flgImg[i] = 2;
		Q.push_back(EVec3i(x,y,i));
	}		
	
	while (1)
	{
		fprintf( stderr, "boundary growth %d\n", (int)Q.size());

		list<EVec3i> nextQ;
		while (!Q.empty())
		{
			EVec3i p = Q.front();
			Q.pop_front();
			EVec4i c = texture.getRGBA(p[2]*4);
			
			const EVec3i &n0 = pix_Ns[p[2]].n[0];
			const EVec3i &n1 = pix_Ns[p[2]].n[1];
			const EVec3i &n2 = pix_Ns[p[2]].n[2];
			const EVec3i &n3 = pix_Ns[p[2]].n[3];

			if (n0[0] != -1 && flgImg[n0[2]] == 1) { flgImg[n0[2]] = 2; nextQ.push_back(n0); texture.setPix(n0[2]*4, c); }
			if (n1[0] != -1 && flgImg[n1[2]] == 1) { flgImg[n1[2]] = 2; nextQ.push_back(n1); texture.setPix(n1[2]*4, c); }
			if (n2[0] != -1 && flgImg[n2[2]] == 1) { flgImg[n2[2]] = 2; nextQ.push_back(n2); texture.setPix(n2[2]*4, c); }
			if (n3[0] != -1 && flgImg[n3[2]] == 1) { flgImg[n3[2]] = 2; nextQ.push_back(n3); texture.setPix(n3[2]*4, c); }
		}

		if( nextQ.size()==0) break;
		Q = nextQ;
	}
}



//smoothing texture pixel with flgImg[i] == 2 by using pix_Ns
static void smoothingTexture
(
	const vector< Neighbors > &pix_Ns, // 4-neighbors at each target pixel
	const int				  *flgImg, // input flg( -1:nottrgt, 0:valueExist, 1:target, 2:target and fixed)
	const int            smoothingNum, 

	TImage2D &texture //input&output Texture
)
{
	const int W  = texture.m_W;
	const int H  = texture.m_H;
	const int WH = W*H;

	for (int kkk = 0; kkk < smoothingNum; ++kkk)
	{
		TImage2D img = texture;
		for( int i=0; i < WH; ++i) if( flgImg[i] == 2)
		{
			const Neighbors& Ns = pix_Ns[i];
			EVec4i c(0,0,0,0);
			int s = 0;
			for (int j = 0; j < 4; ++j)if ( Ns.n[j][0] != -1)
			{
				c += texture.getRGBA(4*Ns.n[j][2]);
				s ++;
			}
			int r = (int)(c[0] / (double)s + 0.5 );
			int g = (int)(c[1] / (double)s + 0.5 );
			int b = (int)(c[2] / (double)s + 0.5 );
			img.setPix( 4*i, r,g,b,128);
		}
		texture = img;
		fprintf(stderr, "*");
	}
}


static double t_patchHolePixRate
(
	const int *patchUvId,
	const int *flgImg //flg - -1:nontarget, 0:value exist, 1:hole, 2: fixed hole
)
{
	double sum = 0;
	for (int i = 0; i < PATCH_WW; ++i) if (patchUvId[i] != -1 && flgImg[patchUvId[i]] >= 1) sum += 1;
	return sum / PATCH_WW;
}









class TexPatch
{
public:
	byte   rgb[PATCH_WW3];
	EVec3d mean;

	void copy(const TexPatch &src)
	{
		memcpy( rgb , src.rgb , sizeof(byte) * PATCH_WW3);
		mean = src.mean;
	}
	
	TexPatch()
	{
		memset( rgb, 0, sizeof( byte) * PATCH_WW3);
		mean << 0,0,0;
	}

	TexPatch(const TexPatch &src)
	{
		copy(src);
	}

	TexPatch &operator=( const TexPatch &src)
	{
		copy(src);
		return *this;
	}

	void updateMean()
	{
		mean << 0,0,0;
		double r = 0, g = 0, b = 0;
		for (int i = 0; i < PATCH_WW; ++i)
		{
			mean[0] += rgb[i*3+0];
			mean[1] += rgb[i*3+1];
			mean[2] += rgb[i*3+2];
		}
		mean[0] /= PATCH_WW;
		mean[1] /= PATCH_WW;
		mean[2] /= PATCH_WW;
	}
};


#define CLUSTER_SIZE 20


static void calcClusterColor
(
	const vector<TexPatch> &patches,
	const int              *labels  ,
	EVec3d                 *ClstCols
)
{
	int counter[CLUSTER_SIZE] = {};

	for( int c=0; c < CLUSTER_SIZE; ++c ) ClstCols[c] << 0,0,0;

	for( int i=0; i < patches.size(); ++i ) 
	{
		counter[labels[i]]++;
		ClstCols[labels[i]] += patches[i].mean;
	}

	for( int c=0; c < CLUSTER_SIZE; ++c ) ClstCols[c] /= counter[c];
}




class TexPatchSet
{
public:
	vector<TexPatch> m_patches;

	//cluster info
	EVec3d      m_ClstCols[CLUSTER_SIZE];
	vector<int> m_ClstPatdhIDs[CLUSTER_SIZE];

	TexPatchSet()
	{
		m_patches.clear();
	}


	void update_kMeanClusters()
	{
		fprintf( stderr, "update_kMeanClusters\n");
		const int N = (int)m_patches.size();

		int *labels = new int[N];

		//initialize
		for( int i=0; i < N; ++i) {
			labels[i] = (int)(rand()/(double)RAND_MAX*CLUSTER_SIZE);
			if (labels[i] >= CLUSTER_SIZE) labels[i] = CLUSTER_SIZE-1;
		}
		calcClusterColor( m_patches, labels, m_ClstCols);

		//k-mean clustering iteration
		for (int kkk = 0; kkk < 500; ++kkk)
		{

			//find nearest col
			for( int i=0; i < N; ++i ) 
			{
				double minD = DBL_MAX;
				labels[i] = 0;
				for (int j = 0; j < CLUSTER_SIZE; ++j)
				{
					double d = ( m_patches[i].mean - m_ClstCols[j] ).squaredNorm();
					if (d < minD) 
					{
						minD = d;
						labels[i] = j;
					}
				}
			}
			//calc clstCol
			calcClusterColor( m_patches, labels, m_ClstCols);
		}

		//update cluster color
		for( int c=0; c < CLUSTER_SIZE; ++c ) m_ClstPatdhIDs[c].clear();
		for( int i=0; i < N; ++i ) 
		{
			m_ClstPatdhIDs[labels[i]].push_back(i);
		}
		delete[] labels ;

		fprintf( stderr, "fin\n");

	}


};




static void t_dbg_exportPatch(
	const float *patchRGB
)
{
	static int kk=0;

	TImage2D img;
	img.AllocateImage(PATCH_W,PATCH_W,0);

	for( int i=0; i < PATCH_WW; ++i) 
		img.setPix(4*i, (byte)patchRGB[3*i+0], (byte)patchRGB[3*i+1], (byte)patchRGB[3*i+2], 128);

	CString fname;
	fname.Format("out%d.bmp", kk);
	img.saveAsFile( fname, 0);

	++kk;
}





void genPatchesByExpMap
(
	const TTexMesh        &mesh ,  //input: mesh should be have tex coord
	const vector<EVec3d>  &vFlow,//input: smooth tangent direction (u-dir of exp map is aligned)

	const int      *polyIdTex,//input: id ref img ( -1:nottrgt, 0~:triangle id)
	const int	   *flgImg   ,//input: flg img    ( -1:nottrgt, 0:valueExist, 1:hole, 2:fixed hole)
	const TImage2D &texture  ,//input: texture (hole was fixed by simple regio growing ) 
	const double   patchPitch,//input: sampling pitch for patch pixel
	
	TexPatchSet &patchSet //output
)
{
	fprintf( stderr, "genPatchesByExpMap!!!\n");
	const int W  = texture.m_W;
	const int H  = texture.m_H;
	const int WH = W*H;

	//gen multiple patches
	for(int i=0; i < WH; i+= 20)if( flgImg[i] == 0 )
	{
		const int x = i%W, y = i/W;

		int patchUvId[PATCH_WW];
		t_getNeighboringPatch(W,H,EVec3i(x,y,i), polyIdTex[i],mesh, vFlow, PATCH_R, patchPitch, patchUvId);

		if( t_patchHolePixRate( patchUvId, flgImg ) < 0.1 )
		{
			patchSet.m_patches.push_back(TexPatch());

			for (int j = 0; j < PATCH_WW; ++j) if( patchUvId[j] != -1)
			{
				patchSet.m_patches.back().rgb[3*j+0] = texture[ patchUvId[j]*4+0 ];
				patchSet.m_patches.back().rgb[3*j+1] = texture[ patchUvId[j]*4+1 ];
				patchSet.m_patches.back().rgb[3*j+2] = texture[ patchUvId[j]*4+2 ];
			}
			patchSet.m_patches.back().updateMean();
		}
		
		//if( patchSet.m_patches.size() > 30000) break;
		if ( i % 5000 == 0 ) fprintf(stderr, "*");
	}

	//debug用
	/*
	for (int i = 0; i < patchSet.m_patches.size(); i += 500)
	{
		CString fname;
		fname.Format("patch%d.bmp", i);
		t_exportPatch( PATCH_W, patchSet.m_patches[i].rgb, fname);
		fname.Format("patch%da.bmp", i);
		t_exportPatch( PATCH_W, patchSet.m_patches[i].mean, fname);
	}
	*/

	//compute k-mean clustering
	patchSet.update_kMeanClusters();

	fprintf( stderr, "finished %d \n", (int) patchSet.m_patches.size());

}




static int similarPatchIdx1( const TexPatchSet &patchSet, const byte *patchRGB)
{
	int    minI = 0;
	double minD = DBL_MAX;
	for (int i = 0; i < patchSet.m_patches.size(); ++i)
	{
		const byte *rgb = patchSet.m_patches[i].rgb;
		double s = 0;
		for( int j=0; j < PATCH_WW3; ++j) 
			s += ( rgb[j] - patchRGB[j]) * ( rgb[j] - patchRGB[j]);
		
		if (s < minD) 
		{
			minD = s;
			minI = i;
		}
	}
	// fprintf( stderr, "%d",minI );
	return minI;
}


static int similarPatchIdx2( const TexPatchSet &patchSet, const byte *patchRGB)
{
	EVec3d col(0,0,0);
	for (int j = 0; j < PATCH_WW; ++j)
	{
		col[0] += patchRGB[3*j+0];
		col[1] += patchRGB[3*j+1];
		col[2] += patchRGB[3*j+2];
	}
	col[0] /= PATCH_WW;
	col[1] /= PATCH_WW;
	col[2] /= PATCH_WW;

	int    minClstI = 0;
	double minClstD = DBL_MAX;
	for (int c = 0; c < CLUSTER_SIZE; ++c)
	{
		double d = (col - patchSet.m_ClstCols[c]).squaredNorm();
		if (d < minClstD)
		{
			minClstD = d;
			minClstI = c;
		}
	}


	int    minI = 0;
	double minD = DBL_MAX;

	for (int k = 0; k < patchSet.m_ClstPatdhIDs[minClstI].size(); ++k)
	{
		const int i =  patchSet.m_ClstPatdhIDs[minClstI][k];

		const byte *rgb = patchSet.m_patches[i].rgb;
		double s = 0;
		for( int j=0; j < PATCH_WW3; ++j) 
			s += ( (double) rgb[j] - (double) patchRGB[j]) * ( (double)rgb[j] - (double)patchRGB[j]);
		
		if (s < minD) 
		{
			minD = s;
			minI = i;
		}
	}
	// fprintf( stderr, "%d",minI );
	return minI;
}




TImage2D patchMatchToRefineHole
(
	const TTexMesh        &mesh ,//input: mesh should be have tex coord
	const vector<EVec3d>  &vFlow,//input: smooth tangent direction (u-dir of exp map is aligned)

	const int       *polyIdTex	,//input: id ref img ( -1:nottrgt, 0~:triangle id)
	const int	    *flgImg		,//input: flg img    ( -1:nottrgt, 0:valueExist, 1:hole, 2:fixed hole)
	const TImage2D  &texture	,//input: texture (hole was fixed by simple regio growing ) 
	const double     patchPitch	,//input: sampling pitch for patch pixel

	const TexPatchSet  &patchSet //input: patches
)
{
	fprintf( stderr, "\npatchMatchToRefineHole\n");
	const int IterN = 20;
	const int W  = texture.m_W;
	const int H  = texture.m_H;
	const int WH = W*H;

	TImage2D result = texture;

	const int THREAD_N = omp_get_max_threads();
	float **rgbn = new float*[THREAD_N];
	for( int i=0; i < THREAD_N; ++i) rgbn[i] = new float[WH*4];

	vector<int> trgtPixIdx;
	trgtPixIdx.reserve(WH);
	for (int i = 0; i < WH; ++i) if (flgImg[i] == 2) trgtPixIdx.push_back(i);

	for (int iter = 0; iter < IterN; ++iter)
	{
		for( int i=0; i < THREAD_N; ++i) memset( rgbn[i], 0, sizeof(float)*WH*4);


		const int ROOP = (int)trgtPixIdx.size() / 20;

#pragma omp parallel for schedule(dynamic)
		for (int roopI = 0; roopI < ROOP; ++roopI) 
		{
			const int thI = omp_get_thread_num();
			const int i   = trgtPixIdx [ (int)(  rand() / (double)RAND_MAX * (trgtPixIdx.size()-1) ) ];
			const int x   = i%W, y = i/W;
			
			if( roopI % 20000 == 0 ) fprintf( stderr, "%d/%d  (%d %d)\n", i, WH, roopI, thI);
			if (flgImg[i] != 2) continue;

			int  patchUvId[PATCH_WW ];
			byte patchRGB [PATCH_WW3];
			t_getNeighboringPatch(W,H,EVec3i(x,y,i), polyIdTex[i], mesh, vFlow, PATCH_R, patchPitch, patchUvId);
			t_getPatchByIdx(result, patchUvId, patchRGB);

			//clock_t t0 = clock();
			//int minI1 = similarPatchIdx1( patchSet, patchRGB); 
			//clock_t t1 = clock();
			int minI2 = similarPatchIdx2( patchSet, patchRGB); 
			//clock_t t2 = clock();

			//fprintf( stderr, "%d %d %d %d\n", t1-t0, t2-t1, minI1, minI2);

			//update result texture  result <-- patches[minI]
			set<int> uvIds;
			for (int k = 0; k < PATCH_WW; ++k) if( patchUvId[k] != -1 && uvIds.find(patchUvId[k]) == uvIds.end())
			{
				uvIds.insert(patchUvId[k]);
				rgbn[thI][ patchUvId[k]*4+0] += patchSet.m_patches[minI2].rgb[3*k+0];
				rgbn[thI][ patchUvId[k]*4+1] += patchSet.m_patches[minI2].rgb[3*k+1];
				rgbn[thI][ patchUvId[k]*4+2] += patchSet.m_patches[minI2].rgb[3*k+2];
				rgbn[thI][ patchUvId[k]*4+3] += 1;
			}
		}
		fprintf( stderr, "ccccc");

		for (int i = 0; i < WH; ++i) if (flgImg[i] == 2)
		{
			for (int thI = 1; thI < THREAD_N; ++thI)
			{
				rgbn[0][i*4 + 0] += rgbn[thI][i*4 + 0];
				rgbn[0][i*4 + 1] += rgbn[thI][i*4 + 1];
				rgbn[0][i*4 + 2] += rgbn[thI][i*4 + 2];
				rgbn[0][i*4 + 3] += rgbn[thI][i*4 + 3];
			}
			if( rgbn[0][i*4 + 3] == 0 ) continue;
			result[4*i+0] = (int)( 0.5 + rgbn[0][ i*4+0]/rgbn[0][ i*4+3]);
			result[4*i+1] = (int)( 0.5 + rgbn[0][ i*4+1]/rgbn[0][ i*4+3]);
			result[4*i+2] = (int)( 0.5 + rgbn[0][ i*4+2]/rgbn[0][ i*4+3]);
			result[4*i+3] = 128;
		}

		CString fname;
		fname.Format( "test%d.bmp", iter);
		result.saveAsFile(fname, 0);
	}



	for( int i=0; i < THREAD_N; ++i) delete[] rgbn[i];
	delete[] rgbn;


	return result;
}





TImage2D HoleRetrieval
(
	const TTexMesh       &mesh   , //input: mesh should be have tex coord
	const vector<EVec3d> &vFlow  , 
	const TImage2D       &trgtTex, //input: texture (0,0,255) is hole pixel
	const int      &smoothingNum,
	const double   &patchPitch
)	
{
	const int W  = trgtTex.m_W;
	const int H  = trgtTex.m_H;
	const int WH = W*H;

	//0. -- preparation --
	int *polyIdTex = new int[WH]; //tex idref image (-1:nontarget, triangleID)
 	int *flgImg    = new int[WH]; //outOfTex, 0:valueExist, 1:target, 2:target and fixed
	vector< Neighbors > pix_Ns(WH);

	t_genPolygonIDtexture( mesh, W,H, polyIdTex);

	for( int i=0; i < WH; ++i) 
	{
		flgImg[i] = (polyIdTex[i] == -1      ) ? -1 : 
		            (!trgtTex.isBlackPix(4*i)) ?  0 : 1;

		if( flgImg[i] != 1) continue;
		const int x=i%W, y = i/W;
		t_getNeighbors(W,H,mesh, polyIdTex, EVec3i(x,y,i), pix_Ns[i]);
	}

	//step1. boundary growth
	TImage2D texture = trgtTex;
	texture.saveAsFile("aaa.bmp", 0);
	HoleFillByBoundaryGrow( pix_Ns, flgImg, texture);
	texture.saveAsFile("bbb.bmp", 0);

	//step2. smoothing 
	smoothingTexture( pix_Ns, flgImg, smoothingNum, texture);
	texture.saveAsFile("ccc.bmp", 0);

	//step3. patch-based synthesis
	TexPatchSet patchSet;
	genPatchesByExpMap(mesh, vFlow, polyIdTex, flgImg, texture, patchPitch, patchSet);
	texture = patchMatchToRefineHole( mesh, vFlow, polyIdTex, flgImg, texture, patchPitch, patchSet);

	texture.saveAsFile("ddd.bmp", 0);

	delete[] polyIdTex;
	delete[] flgImg;

	return texture;
}





