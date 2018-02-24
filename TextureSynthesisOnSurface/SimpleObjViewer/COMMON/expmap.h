#pragma once

#include "ttexmesh.h"


	
class ExpMapVtx
{
public:
	byte   flg   ; // 0:yet visited, 1:in Q, 2:fixed
	int    from  ; // vertex index from which the path comes from (-1:startP, -2,yet)
	double dist  ; // dist from start P
	EVec2d pos   ; // position in 2D normal coordinate

	ExpMapVtx()
	{
		flg  = 0;
		from = -2;
		dist = FLT_MAX;
		pos << 0,0;
	}

	void Set(byte _flg, int _from, double _dist)
	{
		flg  = _flg;
		from = _from;
		dist = _dist;
	}
	
	void Set(byte _flg, int _from, double _dist, EVec2d &_pos)
	{
		flg    = _flg;
		from   = _from;
		dist   = _dist;
		pos    = _pos ;
	}
};




/* ----------------------------------
const TMesh    &mesh    // surface mesh model
const EVec3f   &startP  // center point of exponential map
const int      &polyIdx // polygon idx of the mesh on which the startP exist
const float    &scaleC  // scaling coefficients for 2D to 3D 
vector<EVec2d> &expMap  //computed exponential map ([0,1]x[0,1]) startP = (0,5,0.5)
---------------------------------- */


void DijikstraMapping
(
	const TTexMesh    &mesh   ,
	const EVec3d      &startP ,
	const int         &polyIdx,
	vector<ExpMapVtx> &expMap

);




void expnentialMapping
(
	const TTexMesh    &mesh   ,
	const vector<EVec3d>    &vFlow,

	const EVec3d      &startP ,
	const int         &polyIdx,
	const double      &maxDist, //do not compute further than maxDist
	const byte        *vtxFlg , //do not compute expmap for this vertex (Set 0 when not necessary)
	vector<ExpMapVtx> &expMap
);



