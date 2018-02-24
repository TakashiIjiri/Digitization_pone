#pragma once

#pragma warning(disable:4996)

#include "tmath.h"
#include "timage.h"
#include <list>
#include <vector>
#include <set>
using namespace std;



/*
                   + v[0],t[0]
				 / | 
			    /  | 
		e[0]   /   | e[2]
			  /    |
			 /     |
v[1],t[1]   +------+ v[2],t[2]
　　　　　　　e[1]
　　　
　　　
*/



class TPoly
{
public:
	int vIdx[3]; 
	int tIdx[3];
	
	int edge[3]; //edgeIdx

    TPoly(int v0=0, int v1=0, int v2=0, int t0=0, int t1=0, int t2=0){ 
		vIdx[0] = v0; vIdx[1] = v1; vIdx[2] = v2;
		tIdx[0] = t0; tIdx[1] = t1; tIdx[2] = t2;
	}
    TPoly(const TPoly &p ){ Set(p); }

	void Set( const TPoly &p){
		memcpy( vIdx, p.vIdx, sizeof( int ) * 3);
		memcpy( tIdx, p.tIdx, sizeof( int ) * 3);
	}

	TPoly& operator= (const TPoly  &v){ Set(v); return *this; }
};




/*
      + v[1]
	  |
	  |
 p[0] | P[1]
	  |
	  |
	  + v[0]

*/
class TEdge
{
public:
	int v[2]; //v0 < v1
	int p[2];
	bool bAtlsSeam; //is seam on the atras?


	TEdge(int v0=-1, int v1=-1)
	{
		v[0] = v0;
		v[1] = v1;
		p[0] = p[1] = -1;
		bAtlsSeam = false;
	}

	TEdge(const TEdge &e ){ Set(e); }
	TEdge& operator= (const TEdge  &e){ Set(e); return *this; }

	void Set( const TEdge &e){
		memcpy( v, e.v, sizeof( int ) * 2);
		memcpy( p, e.p, sizeof( int ) * 2);
		bAtlsSeam = e.bAtlsSeam;
	}




};


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






class TTexMesh
{
public:

	//Vertex 
	int    m_vSize ;
	EVec3d *m_verts  ;
	EVec3d *m_v_norms;

	vector<vector<int>> m_v_RingPs;
	vector<vector<int>> m_v_RingVs;

	//UV coords 
	int m_uvSize;
	EVec3d *m_uvs    ;

	//Polys
	int m_pSize     ;
	TPoly *m_polys  ;
	EVec3d *m_p_norms;

	//Edges
	int m_eSize;
	TEdge *m_edges;


	TTexMesh()
	{
		m_vSize   = 0;
		m_verts   = 0;
		m_v_norms = 0;
	
		m_uvSize  = 0;
		m_uvs     = 0;

		m_pSize   = 0;
		m_polys   = 0;

		m_eSize   = 0;
		m_edges   = 0;
	}
	TTexMesh(const TTexMesh &src) { Set(src); }
	TTexMesh& operator=(const TTexMesh &src) { Set(src); return *this; }

	~TTexMesh()
	{
		clear();
	}

	void clear()
	{
		m_vSize  = 0;
		m_uvSize = 0;
		m_pSize  = 0;
		m_eSize  = 0;
		if( m_verts   != 0) delete[] m_verts  ;
		if( m_v_norms != 0) delete[] m_v_norms;
		if( m_uvs     != 0) delete[] m_uvs    ;
		if( m_polys   != 0) delete[] m_polys  ;
		if( m_edges   != 0) delete[] m_edges  ;
	}

	void Set( const TTexMesh &m)
	{
		clear();
		m_vSize = m.m_vSize;
		if (m_vSize != 0)
		{
			m_verts   = new EVec3d[m_vSize];
			m_v_norms = new EVec3d[m_vSize];
			memcpy(m_verts  , m.m_verts  , sizeof(EVec3d)*m_vSize);
			memcpy(m_v_norms, m.m_v_norms, sizeof(EVec3d)*m_vSize);
		}

		m_uvSize  = m.m_uvSize;
		if (m_uvSize != 0)
		{
			m_uvs = new EVec3d[m_uvSize];
			memcpy(m_uvs, m.m_uvs, sizeof(EVec3d)*m_uvSize);
		}

		m_pSize   = m.m_pSize;
		if (m_pSize != 0)
		{
			m_polys = new TPoly[m_pSize];
			memcpy(m_polys, m.m_polys, sizeof(TPoly)*m_pSize);
		}
		m_v_RingPs = m.m_v_RingPs;
		m_v_RingVs = m.m_v_RingVs;

		m_eSize = m.m_eSize;
		if (m_eSize != 0)
		{
			m_edges = new TEdge[m_eSize];
			memcpy(m_edges, m.m_edges, sizeof(TEdge)*m_eSize);
		}
	}


	bool initialize(const char *fName)
	{	
		FILE* fp = fopen(fName,"r") ;
		if( !fp ) return false;

		list<EVec3d>  vs_list   , uvs_list;
		list<TPoly >  polys_list;


		char buf[512] ;		
		while( fgets(buf,255,fp) )
		{
			char* bkup  = _strdup( buf        );
			char* token =  strtok( buf, " \t" );


			if( stricmp( token,"vt" ) == 0 )
			{ 
				EVec3d vt ;
				sscanf( bkup,"vt %lf %lf",&vt[0], &vt[1] ) ;
				uvs_list.push_back( vt ) ;
			} 
			else if( stricmp( token,"v" ) == 0 )
			{ 
				EVec3d v;
				sscanf( bkup,"v %lf %lf %lf",&v[0],&v[1],&v[2] ) ;
				vs_list.push_back( v ) ;
			} 
			else if( stricmp( token,"f" ) == 0 )
			{
				TPoly p(0,0,0,0,0,0);
				int tmp;

				int vtxnum = sscanf( bkup,"f %d %d %d %d", &p.vIdx[0], &p.vIdx[1], &p.vIdx[2], &tmp) ;//sscanfの返り値は正常に読めたフィールド数 (/が入ったら2フィールドしか読めない)

				//t_info( "(%d %d %d)", p.vIdx[0], p.vIdx[1], p.vIdx[2]);
			
				if( vtxnum < 3 ) vtxnum = sscanf( bkup,"f %d/%d %d/%d %d/%d %d/%d" ,            &p.vIdx[0], &p.tIdx[0],
																								&p.vIdx[1], &p.tIdx[1],
																								&p.vIdx[2], &p.tIdx[2], &tmp, &tmp)/2 ;

				if( vtxnum < 3 ) vtxnum = sscanf( bkup,"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &p.vIdx[0], &p.tIdx[0], &tmp,
																								&p.vIdx[1], &p.tIdx[1], &tmp,
																								&p.vIdx[2], &p.tIdx[2], &tmp, &tmp, &tmp, &tmp)/3 ;
				if( vtxnum < 3 ) vtxnum = sscanf( bkup,"f %d//%d %d//%d %d//%d %d//%d" ,        &p.vIdx[0], &tmp,  
																								&p.vIdx[1], &tmp,
																								&p.vIdx[2], &tmp,  &tmp, &tmp )/2 ;
				--p.vIdx[0]; --p.vIdx[1]; --p.vIdx[2];
				--p.tIdx[0]; --p.tIdx[1]; --p.tIdx[2];
				polys_list  .push_back( p ) ;
			}

			free(bkup) ;
		}
		fclose(fp) ;



		m_vSize   = (int) vs_list   .size();
		m_uvSize  = (int) uvs_list  .size();
		m_pSize   = (int) polys_list.size();
		fprintf(stderr, "loaded object file info : %d %d %d\n", m_vSize, m_uvSize, m_pSize);
	
		m_verts   = new EVec3d[m_vSize ];
		m_v_norms = new EVec3d[m_vSize ];
		m_polys   = new TPoly [m_pSize ];
		m_p_norms = new EVec3d[m_pSize ];
		m_uvs     = new EVec3d[m_uvSize];

		int c = 0;
		for( auto it = vs_list   .begin(); it != vs_list   .end(); ++it) m_verts[c++] = *it;
		c = 0;
		for( auto it = uvs_list  .begin(); it != uvs_list  .end(); ++it) m_uvs  [c++] = *it;
		c = 0;
		for( auto it = polys_list.begin(); it != polys_list.end(); ++it) m_polys[c++] = *it;

		updateEdges   ();
		updateNormal  ();
		updateRingInfo();
		fprintf(stderr, "loaded object file info : %d %d %d\n", m_vSize, m_uvSize, m_pSize);
		return true;
	}


	void updateEdges()
	{
		static const int edg_mat[3][2]  = {{0,1},{1,2},{2,0}} ;
		
		vector<TEdge> Es; //まだサイズが不明
		vector< list<int> > emanatingEdges( m_vSize ) ;//list for Eminating edge IDs

		for( int pi = 0; pi < m_pSize; ++pi )
		{
			const int *pVtx = m_polys[pi].vIdx ;
		
			for( int i = 0; i < 3; i++ )
			{
				//edgeを生成: v[0,1], p[0,1], oneEdge, polygon.edge登録
				int v0 = pVtx[ edg_mat[i][0] ] ;
				int v1 = pVtx[ edg_mat[i][1] ] ;
 
				bool bInverted = v0 > v1;
				if( bInverted ) std::swap( v0, v1 );
				
				//find eminating edge
				list<int>& emanEs = emanatingEdges[ v0 ] ;
				list<int>::iterator it;
				for( it = emanEs.begin() ; it != emanEs.end() ; it++ ) if( Es[*it].v[1] == v1 ) break ;

				// the edge not exixt --> add new one
				if( it == emanEs.end() ){
					Es.push_back( TEdge(v0, v1) ) ;
					emanatingEdges[ v0 ].push_back( (int)Es.size()-1 );
					Es.back().p[bInverted?1:0] = pi;
					m_polys[pi].edge[i] =(int) Es.size()-1;
				}
				else
				{
					Es[*it].p[bInverted?1:0] = pi;
					m_polys[pi].edge[i]      = *it;
				}
			}
		}

		m_eSize = (int)Es.size();
		m_edges = new TEdge[ m_eSize ];
		for( int ei=0; ei < m_eSize; ++ei) 
		{
			//copy
			m_edges[ei] = Es[ei];

			//set bAtlsSeam
			TEdge &e   = m_edges[ei];
			TPoly &pA  = m_polys[e.p[0]];
			TPoly &pB  = m_polys[e.p[1]];
			e.bAtlsSeam = true;

			for (int i = 0; i < 3 && e.bAtlsSeam; ++i)
			for (int j = 0; j < 3 && e.bAtlsSeam; ++j)
			{
				int tA0 = pA.tIdx[ edg_mat[i][0] ];
				int tA1 = pA.tIdx[ edg_mat[i][1] ];
				int tB0 = pB.tIdx[ edg_mat[j][0] ];
				int tB1 = pB.tIdx[ edg_mat[j][1] ];
				if( (tA0 == tB0 && tA1 == tB1) || ( tA0 == tB1 && tA1 == tB0 ) ) e.bAtlsSeam = false;
			}
		}
	}


	void updateNormal()
	{
		for(int i=0; i<m_vSize; ++i) m_v_norms[i].Zero();


		for(int i=0; i<m_pSize; ++i)
		{
			int *idx = m_polys[i].vIdx;
			m_p_norms[i] = (m_verts[ idx[1] ] - m_verts[ idx[0] ]).cross( m_verts[ idx[2] ] - m_verts[ idx[0] ]);

			if( m_p_norms[i].norm() > 0 )
			{
				m_p_norms[i].normalize();
				m_v_norms[ idx[0] ] += m_p_norms[i];
				m_v_norms[ idx[1] ] += m_p_norms[i];
				m_v_norms[ idx[2] ] += m_p_norms[i];
			}
		}

		for(int i=0; i<m_vSize; ++i) m_v_norms[i].normalize();
	}


	
	void updateRingInfo()
	{
		m_v_RingPs.clear();
		m_v_RingVs.clear();
		m_v_RingPs.resize(m_vSize);
		m_v_RingVs.resize(m_vSize);

		for( int i =0; i < m_pSize; ++i)
		{
			int *idx = m_polys[i].vIdx;
			m_v_RingPs[ idx[0] ].push_back(i);
			m_v_RingPs[ idx[1] ].push_back(i);
			m_v_RingPs[ idx[2] ].push_back(i);
			m_v_RingVs[ idx[0] ].push_back(idx[1]);
			m_v_RingVs[ idx[0] ].push_back(idx[2]);
			m_v_RingVs[ idx[1] ].push_back(idx[0]);
			m_v_RingVs[ idx[1] ].push_back(idx[2]);
			m_v_RingVs[ idx[2] ].push_back(idx[0]);
			m_v_RingVs[ idx[2] ].push_back(idx[1]);
		}

 		for (int i = 0; i < m_vSize; ++i) 
		{
			sort  (m_v_RingVs[i].begin(), m_v_RingVs[i].end());
			auto it = unique(m_v_RingVs[i].begin(), m_v_RingVs[i].end());
			m_v_RingVs[i].erase( it, m_v_RingVs[i].end());
		}
	}
		




	void drawWithTex( TImage2D &img )
	{
		img.Bind(0);
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_LIGHTING   );

		glBegin ( GL_TRIANGLES  );

		const EVec3d *verts = m_verts  ;
		const EVec3d *norms = m_v_norms;
		const EVec3d *uvs   = m_uvs    ;

		for (int i = 0; i < m_pSize; ++i)
		{
			const int *idx   = m_polys[i].vIdx;
			const int *uvIdx = m_polys[i].tIdx;
			glNormal3dv( norms[idx[0] ].data()); glTexCoord2dv( uvs[uvIdx[0]].data()); glVertex3dv( verts[idx[0]].data());
			glNormal3dv( norms[idx[1] ].data()); glTexCoord2dv( uvs[uvIdx[1]].data()); glVertex3dv( verts[idx[1]].data());
			glNormal3dv( norms[idx[2] ].data()); glTexCoord2dv( uvs[uvIdx[2]].data()); glVertex3dv( verts[idx[2]].data());
		}
		glEnd();

		glDisable( GL_TEXTURE_2D );
	}
	
	void drawEdge( float lineWidth)
	{
		glColor3d(1,1,1);
		glLineWidth( lineWidth );

		glDisable( GL_LIGHTING   );

		glBegin  ( GL_LINES);

		const EVec3d *verts = m_verts  ;

		for (int i = 0; i < m_pSize; ++i)
		{
			const int *idx   = m_polys[i].vIdx;
			glVertex3dv( verts[idx[0]].data()); glVertex3dv( verts[idx[1]].data());
			glVertex3dv( verts[idx[1]].data()); glVertex3dv( verts[idx[2]].data());
			glVertex3dv( verts[idx[2]].data()); glVertex3dv( verts[idx[0]].data());
		}
		glEnd();
	}




	void Translate(EVec3d t)
	{
		for( int i=0; i < m_vSize; ++i ) m_verts[i] += t;
	}

	void Scale(double d)
	{
		for( int i=0; i < m_vSize; ++i ) m_verts[i] *= d;
	}



	EVec3d getGravityCenter()
	{
		EVec3d p(0,0,0);
		for( int i=0; i < m_vSize; ++i ) p += m_verts[i];
		return  p / (double)m_vSize;
	}



	//compute collision to all polygons ald (p0-p1)
	//ignor back surface (p1-p0) * norm > 0
	bool intersectToLineSegment_ignorBackSurface(const EVec3d &p0, const EVec3d &p1, int ignorVid) const
	{
		EMat3d M;
		EVec3d b, stk, cRay(p1 - p0);

		for (int i = 0; i < m_pSize; ++i)
		{
			if (m_polys[i].vIdx[0] == ignorVid ||
				m_polys[i].vIdx[1] == ignorVid ||
				m_polys[i].vIdx[2] == ignorVid || cRay.dot(m_p_norms[i]) > 0) continue;

			const EVec3d &v0 = m_verts[m_polys[i].vIdx[0]];
			const EVec3d &v1 = m_verts[m_polys[i].vIdx[1]];
			const EVec3d &v2 = m_verts[m_polys[i].vIdx[2]];

			M << v1[0] - v0[0], v2[0] - v0[0], -(p1[0] - p0[0]),
				 v1[1] - v0[1], v2[1] - v0[1], -(p1[1] - p0[1]),
				 v1[2] - v0[2], v2[2] - v0[2], -(p1[2] - p0[2]);

			if (M.determinant() == 0) continue;
			stk = M.partialPivLu().solve( p0 - v0 );

			if (0 <= stk[0] && 0 <= stk[1] && stk[0] + stk[1] <= 1 && 0 <= stk[2] && stk[2] <= 1) return true;
		}


		return false;
	}




	
	bool pickByRay( const EVec3d &rayP, const EVec3d &rayD, EVec3d &pos, int &pIdx ) const
	{
		double depth = DBL_MAX;
		EVec3d tmpPos;
		pIdx  = -1;
		for (int pi = 0; pi < m_pSize; ++pi)
		{
			const int *p = m_polys[pi].vIdx;

			if (t_intersectRayToTriangle( rayP, rayD, m_verts[p[0]], m_verts[p[1]], m_verts[p[2]], tmpPos) )
			{
				double d = (tmpPos - rayP).norm();
				if (d < depth)
				{
					depth = d;
					pos   = tmpPos;
					pIdx  = pi;
				}
			}
		}
		return depth != DBL_MAX;
	}



	void getNringPs(const int pid, const int N, set<int> &Ps) const
	{
		Ps.insert(pid);

		for (int k = 0; k < N; ++k)
		{
			set<int> tmp = Ps;
			Ps.clear();

			for( const auto it : tmp) 
			{
				int *vidx = m_polys[ it ].vIdx;
				for (int i = 0; i < 3; ++i) 
					for( auto pid : m_v_RingPs[ vidx[i] ] ) Ps.insert( pid );
			}			
		}
	}


};








/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*----------------------------------------------------
// generate Polygon ID texture
// 
// mesh       : surface model with uv on each vertex
// THICK_N    : The size to thicken the boundary 
// int W, H   : 
// polyIdxImg : int[W*H] image painted by texture ids
-----------------------------------------------------*/
inline void t_genPolygonIDtexture
(
	const int W,
	const int H,
	const int THICK_N, 
	const TTexMesh &mesh,
	int *polyIdTex
)
{
	const double pX = 1.0 / W;
	const double pY = 1.0 / H;

	for( int i=0,WH=W*H; i < WH; ++i) polyIdTex[i] = -1;

	for( int pi=0; pi < mesh.m_pSize; ++pi)
	{
		const int   *idx = mesh.m_polys[ pi ].tIdx; 
		const EVec3d &uv0 = mesh.m_uvs[ idx[0] ];
		const EVec3d &uv1 = mesh.m_uvs[ idx[1] ];
		const EVec3d &uv2 = mesh.m_uvs[ idx[2] ];

		int minU = max(  0, (int)( MIN3( uv0[0], uv1[0], uv2[0]) * W ) - 1);
		int minV = max(  0, (int)( MIN3( uv0[1], uv1[1], uv2[1]) * H ) - 1);
		int maxU = min(W-1, (int)( MAX3( uv0[0], uv1[0], uv2[0]) * W ) + 1);
		int maxV = min(H-1, (int)( MAX3( uv0[1], uv1[1], uv2[1]) * H ) + 1);

		//fprintf( stderr, "%d %d %d %d\n", minU, minV,maxU, maxV );

		for( int u = minU; u < maxU; ++u)
		{
			for( int v = minV; v < maxV; ++v)
			{
				EVec3d p( (u+0.5) / W, (v+0.5)/H, 0);
				if( t_isInTriangle2D( uv0, uv1, uv2, p) )
				{
					polyIdTex[u + v*W] = pi;
				}
			}
		}
	}

	//thicken one-ring pixels (N-times)
	for (int i = 0; i < THICK_N; ++i)
	{
		for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x)
		{
			int I = x + y * W;
			if (polyIdTex[I] != -1) continue;
			if (x > 0     && polyIdTex[I - 1] >= 0) { polyIdTex[I] = -2; continue; }
			if (y > 0     && polyIdTex[I - W] >= 0) { polyIdTex[I] = -2; continue; }
			if (x < W - 1 && polyIdTex[I + 1] >= 0) { polyIdTex[I] = -2; continue; }
			if (y < H - 1 && polyIdTex[I + W] >= 0) { polyIdTex[I] = -2; continue; }
		}

		for (int y = 0; y < H; ++y)
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

inline TImage2D t_genImage2DforIdImg(const int W, const int H, const int *idImg)
{
	//only for debug texture
	TImage2D forDebug;
	forDebug.AllocateImage( W,H, 0);
	for( int i=0,WH=W*H; i < WH; ++i) 
	{
		int pi = idImg[i];
		if(      pi == -1    ) forDebug.setPix( 4*i, 0  ,0  ,255,0);
		else if( pi % 10 == 0) forDebug.setPix( 4*i, 255,0  ,0  ,0);
		else if( pi % 10 == 1) forDebug.setPix( 4*i, 0  ,0  ,0  ,0);
		else if( pi % 10 == 2) forDebug.setPix( 4*i, 64 ,0  ,0  ,0);
		else if( pi % 10 == 3) forDebug.setPix( 4*i, 128,0  ,0  ,0);
		else if( pi % 10 == 4) forDebug.setPix( 4*i, 196,0  ,0  ,0);
		else if( pi % 10 == 5) forDebug.setPix( 4*i, 255,0  ,0  ,0);
		else if( pi % 10 == 6) forDebug.setPix( 4*i, 0  ,255,0  ,0);
		else if( pi % 10 == 7) forDebug.setPix( 4*i, 64 ,255,0  ,0);
		else if( pi % 10 == 8) forDebug.setPix( 4*i, 128,255,0  ,0);
		else if( pi % 10 == 9) forDebug.setPix( 4*i, 190,255,0  ,0);
		else                   forDebug.setPix( 4*i, 128,128,0  ,0);
	}
	return forDebug;
}





/* ------------------------------------------------------
uvテクスチャアトラス上でseam edgeをまたいだ近傍画素を返す
const int       W   ,  // width of texture atlas 
const int       H   ,  // height of texture atlas
const EVec3i   &pix ,  // target pixel (u,v, u+v*W) this should be on seam 
const TTexMesh &mesh,  // target mesh
const int      *polyIdTex // texture atlas with polygon id (-1 for background)
-------------------------------------------------------*/
inline EVec3i t_neighborUVaccrossSeam(
	const int		W,
	const int	    H,
	const TTexMesh &mesh,
	const int      *polyIdTex,
	const EVec3i   &pix)
{
	const int   polyI = polyIdTex[pix[2]];
	if (polyI == -1)
	{
		fprintf( stderr, "never comes here\n");
		return EVec3i(-1,-1,-1);
	}

	// 1. pixが一番近いpoly1の辺を検索する
	EVec3d pixPos( (pix[0]+0.5) / W, (pix[1]+0.5)/H, 0);
	const TPoly &poly1     = mesh.m_polys[polyI];
	const EVec3d &uv0      = mesh.m_uvs[ poly1.tIdx[0] ];
	const EVec3d &uv1      = mesh.m_uvs[ poly1.tIdx[1] ];
	const EVec3d &uv2      = mesh.m_uvs[ poly1.tIdx[2] ];
	const bool   &e0seam   = mesh.m_edges[ poly1.edge[0] ].bAtlsSeam;
	const bool   &e1seam   = mesh.m_edges[ poly1.edge[1] ].bAtlsSeam;
	const bool   &e2seam   = mesh.m_edges[ poly1.edge[2] ].bAtlsSeam;
	const int    seamEdgeN = (e0seam ? 1 : 0) + (e1seam ? 1 : 0) + (e2seam ? 1 : 0);

	if( seamEdgeN  == 0 ) return EVec3i(-1,-1,-1);

	//get closest edge

	double t0,t1,t2;
	double d0 = t_distPointToLineSegment2D( pixPos, uv0, uv1, t0 );
	double d1 = t_distPointToLineSegment2D( pixPos, uv1, uv2, t1 );
	double d2 = t_distPointToLineSegment2D( pixPos, uv2, uv0, t2 );

	//k = 0(01), 1(12), 2(20)
	int k = -1;
	if (seamEdgeN  == 1)
	{
		 k = ( e0seam ) ? 0 : ( e1seam ) ? 1 : ( e2seam ) ? 2 : -1;
	}
	else
	{
		if(      e0seam && e1seam && e2seam  ) k = (d0<= d1 && d0 <= d2) ? 0 : (d1<= d0 && d1 <= d2) ? 1 : 2;
		else if( e0seam && e1seam            ) k = (d0<= d1) ? 0 : 1; 
		else if(           e1seam && e2seam  ) k = (d1<= d2) ? 1 : 2; 
		else if( e0seam           && e2seam  ) k = (d0<= d2) ? 0 : 2; 
	}

	if( k == -1 ) {
		fprintf( stderr, "never comes here2[%d]\n", seamEdgeN);
		return EVec3i(-1,-1,-1);
	}
	const int    poly1v0 = poly1.vIdx[ k       ];
	const int    poly1v1 = poly1.vIdx[ (k+1)%3 ];


	// 2. 辺eを共有するpoly2を検索し，相対位置が一致するuv座標を取得
	const TEdge  &e      = mesh.m_edges[ poly1.edge[k] ];
	const TPoly  &poly2  = mesh.m_polys[ (polyI != e.p[0]) ? e.p[0] : e.p[1] ];
	const EVec3d &p2uv0  = mesh.m_uvs[ poly2.tIdx[0] ];
	const EVec3d &p2uv1  = mesh.m_uvs[ poly2.tIdx[1] ];
	const EVec3d &p2uv2  = mesh.m_uvs[ poly2.tIdx[2] ];

	//poly1が左回りなので poly 2は右回り
	EVec3d uv;
	if ( poly1v0 == poly2.vIdx[1] && poly1v1 == poly2.vIdx[0] ) uv = t0 * (p2uv0 - p2uv1 ) + p2uv1; 
	if ( poly1v0 == poly2.vIdx[2] && poly1v1 == poly2.vIdx[1] ) uv = t1 * (p2uv1 - p2uv2 ) + p2uv2; 
	if ( poly1v0 == poly2.vIdx[0] && poly1v1 == poly2.vIdx[2] ) uv = t2 * (p2uv2 - p2uv0 ) + p2uv0; 

	//3. 結果を返す
	int u = (int)(uv[0] * W);
	int v = (int)(uv[1] * H);
	int I = u + v * W;
	if( polyIdTex[I] != -1 ) return EVec3i(u,v,I);

	fprintf( stderr, "StrangeMap(%f %f)",uv[0],uv[1]);
	return EVec3i(-1,-1,-1);
}




// texture atlas の全ての境界画素について，境界の先の隣接画素へのmappingを作成する
// seamMapTex : -1:non seam, 0:以上の正値 seam画素のmap先
inline void t_genSeamMapTexture
(
	const int W,
	const int H,
	const int *polyIdTex,
	const TTexMesh &mesh,
	int *seamMapTex // should be allocated
)
{
	for( int i=0; i < W*H; ++i ) seamMapTex[i] = -1;

	for (int y = 1; y < H-1; ++y)
	{
		for (int x = 1; x < W-1; ++x)
		{
			//check 4 neighbors 
			int I = x + y*W;
			if( polyIdTex[I] < 0 ) continue;
			
			if (polyIdTex[I - 1] < 0 || polyIdTex[I + 1] < 0 || polyIdTex[I - W] < 0 || polyIdTex[I + W] < 0)
			{
				seamMapTex[I] = t_neighborUVaccrossSeam(W,H,mesh,polyIdTex, EVec3i(x,y,I))[2];
			}
		}
	}





}