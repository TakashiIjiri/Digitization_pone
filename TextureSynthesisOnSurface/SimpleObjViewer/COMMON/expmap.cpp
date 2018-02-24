#include "stdafx.h"

#include "TOGL.h"
#include "expmap.h"

#include <map>



static Eigen::AngleAxisd calcRotV1toV2
(
	const EVec3d &v1, //should be normalized 
	const EVec3d &v2  //should be normalized
)
{
	EVec3d axis = v1.cross( v2 ).normalized();
	double theta = acos(v1.dot(v2));
	return Eigen::AngleAxisd( theta, axis );
}


static void Trace(const EVec3d p)
{
	fprintf( stderr, "%f %f %f\n", p[0], p[1], p[2]);
}


static void Trace(const EVec2d p)
{
	fprintf( stderr, "%f %f \n", p[0], p[1]);
}





//compute u_q in Tp
static EVec2d calcPosInNormalCoord
(
	const EVec3d &coordO,
	const EVec3d &coordN,
	const EVec3d &coordX,
	const EVec3d &coordY,
	const EVec3d &q
)
{
	EVec3d v   =  q - coordO;
	double len = v.norm();
	EVec3d dir = (v - v.dot( coordN ) * coordN).normalized();

	v = len * dir; 

	return EVec2d( v.dot(coordX), v.dot(coordY));
}




void expnentialMapping
(
	const TTexMesh    &mesh,
	const vector<EVec3d>  &vFlow,

	const EVec3d      &startP,
	const int         &polyIdx,
	const double      &maxDist, //do not compute further than maxDist (Set DBL_MAX when not necessary)
	const byte        *vtxFlg , //do not compute expmap for this vertex (Set 0 when not necessary)
	vector<ExpMapVtx> &expMap
)
{
	//fprintf( stderr, "start...   \n");

	const EVec3d *verts = mesh.m_verts;
	const TPoly  *polys = mesh.m_polys;

	//initialization (flg:0, dist:Inf, from:-2)
	const int *idx = mesh.m_polys[polyIdx].vIdx;
	const EVec3d xDir = vFlow[idx[0]] + vFlow[idx[1]] + vFlow[idx[2]];
	const EVec3d baseN  = mesh.m_p_norms[polyIdx];
	const EVec3d baseX  = (xDir - xDir.dot(baseN)*baseN).normalized(); //(verts[polys[polyIdx].idx[0]] - startP).normalized();
	const EVec3d baseY  = Eigen::AngleAxisd(M_PI * 0.5, baseN ) * baseX;
	
	expMap.clear();
	expMap.resize(mesh.m_vSize);

	multimap<double,int> Q; 
	
	for (int i = 0; i < 3; ++i)
	{
		int vIdx = polys[polyIdx].vIdx[i];

		EVec2d u2D = calcPosInNormalCoord( startP, baseN, baseX, baseY, verts[vIdx]);
		double d   = u2D.norm();

		Q.insert( make_pair(d, vIdx) );
		expMap[vIdx].Set( 1, -1, d, u2D);
	}



	//Dijikstra Growth
	while (!Q.empty())
	{
		//pivot vertex
		double pivD = Q.begin()->first ;
		int    pivI = Q.begin()->second;
		Q.erase( Q.begin () );

		if( pivD > maxDist )continue;
		if( vtxFlg && vtxFlg[pivI]==0) continue;
		
		expMap[pivI].flg = 2;

		//grow this vertex
		const vector<int> &Nei = mesh.m_v_RingVs[pivI];

		EVec3d localO  =  verts[pivI];
		EVec3d tmp     =  verts[Nei[0]] - localO;
		EVec3d localN  =  mesh.m_v_norms[pivI];
		EVec3d localX  =  (tmp - tmp.dot(localN) * localN ).normalized();
		EVec3d localY  =  Eigen::AngleAxisd(M_PI * 0.5, localN ) * localX;

		//compute coordinate transformation
		Eigen::AngleAxisd localToBase = calcRotV1toV2( localN, baseN);
		//EVec3d localN_rot = localToBase * localN;
		EVec3d localX_rot = localToBase * localX;
		//EVec3d localY_rot = localToBase * localY;


		double theta = acos(localX_rot.dot(baseX) );
		if( localX_rot.cross( baseX ).dot( baseN ) < 0 ) theta *= -1;
		Eigen::Rotation2Dd R2d( -theta );

		for (const auto& vi : Nei ) 
		{
			if( expMap[vi].flg == 2 ) continue;

			//Dijikstra法の距離は、graphのedge lengthを利用する実装（たぶん論文はこっちで書いてある。）
			double d = expMap[pivI].dist + (localO - verts[vi]).norm();

			//new coordinate on Tp
			EVec2d u2D = expMap[pivI].pos + R2d * calcPosInNormalCoord( localO, localN, localX, localY, verts[vi]);
		

			if(      expMap[vi].flg == 1 && d < expMap[vi].dist )
			{
				//objects in multimap may share the same key 
				multimap<double,int>::iterator it;
				for( it = Q.find(expMap[vi].dist); it != Q.end(); ++it) if( it->second == vi ) break;

				if (it == Q.end())
				{
					fprintf( stderr, "never comes here");
					expMap[vi].Set( 1, pivI, d, u2D);
				}
				else
				{
					//remove and re-add this neighbor vtx from&to Q
					Q.erase( it );
					expMap[vi].Set( 1, pivI, d, u2D);
					Q.insert( make_pair(d, vi) );
				}
			}
			else if (expMap[vi].flg  == 0)
			{
				expMap[vi].Set( 1, pivI, d, u2D);
				Q.insert( make_pair(d, vi) );
			}
		}
	}
	//fprintf( stderr, "done...\n");
}




