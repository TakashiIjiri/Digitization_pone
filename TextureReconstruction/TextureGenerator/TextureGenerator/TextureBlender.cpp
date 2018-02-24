#include "stdafx.h"
#include "TextureBlender.h"
#include "TMaxFlow_BK4.h"



TextureBlender::TextureBlender()
{
}


TextureBlender::~TextureBlender()
{
}







/*
eCostX represent (a) edge cost
eCostY represent (b) edge cost

       ( i , j ) -a- ( i ,j+1)
	       |            |
		   b            |
		   |            |
	   (i+1, j ) -- (i+1,j+1)


	   o: node
	   s: source
	   t: sink 

    +--- o o o o ---+  
(s)-+--- o o o o ---+- (t)
    +--- o o o o ---+ 
	
	pixel node id: (0, nodeN-1)
	edge  node id: (nodeN, nodeN+edgeNodeN-1)
	source and sink ID: (nodeN+edgeNodeN, nodeN+edgeNodeN+1)

	edgeNodePix is connected to (t)
*/



class PixInfo
{
public:
	//patch id
	int    patchID;
	//隣接辺とのn-linkコスト（graph cutのシームでない場合は 0）
	double eCostX ; 
	double eCostY ;
	double eCostS ;

	PixInfo(){
		patchID = -1;
		eCostX = eCostY = eCostS = 0;
	}
};







static inline double t_calcEdgeVal
(
	const byte *As, const byte *Bs,
	const byte *At, const byte *Bt 
)
{
	double ds = (As[0] - Bs[0]) * (As[0] - Bs[0]) + 
				(As[1] - Bs[1]) * (As[1] - Bs[1]) + 
				(As[2] - Bs[2]) * (As[2] - Bs[2]);
	double dt = (At[0] - Bt[0]) * (At[0] - Bt[0]) +
				(At[1] - Bt[1]) * (At[1] - Bt[1]) +
				(At[2] - Bt[2]) * (At[2] - Bt[2]);
	return (sqrt(ds) + sqrt(dt)) / 255.0;
}







static void t_graphCutTexture_placePatch
(
	const int W, const int H,
	const int   *imgMapSeamPix,// -1:pix is seam, >0:neighboring pix idx 
	const byte  *imgA         , //base image rgba --  a: 0:fore,1fore&NoVal, 2:back
	const float *imgA_agl     , //base image angle [0,1]
	const byte  *imgB         , //new  image rgba --  a: 0:fore,1fore&NoVal, 2:back
	const float *imgB_agl     , //new  image angle [0,1]
	byte        *resImg       , //res  image rgba --  a: 0:fore,1fore&NoVal, 2:back
	float       *resImg_agl   , //res  image angle [0,1]
	PixInfo     *pixInfo,  
	const int    newPatchId
)
{
	const int WH = W*H;


	int N_pNode = 0, N_eNode = 0;
	byte *imgFlg       = new byte[WH]; //0 non, 1:imgA, 2:imgB, 3:both
	int  *imgNodeID    = new int [WH];

	//count pixel nodes (overlap pixels) 
	for( int i = 0; i < WH; ++i)
	{
		//alpha = 0なら値あり
		bool isA = imgA[4*i + 3] == 0;
		bool isB = imgB[4*i + 3] == 0;
		imgFlg[ i ] = ( !isA && !isB ) ? 0 :
					  (  isA && !isB ) ? 1 :
					  ( !isA &&  isB ) ? 2 : 3;

		imgNodeID[i] = -1;
		if ( imgFlg[i] == 3) imgNodeID[i] = N_pNode++;
	}

	//count pixel nodes (existing edges)
	for( int i = 0; i < WH; ++i)
	{
		if (pixInfo[i].eCostX > 0 && imgFlg[i] == 3 && imgFlg[ i + 1 ] == 3) N_eNode++;
		if (pixInfo[i].eCostY > 0 && imgFlg[i] == 3 && imgFlg[ i + W ] == 3) N_eNode++;
		if (pixInfo[i].eCostS > 0 && imgFlg[i] == 3 && imgFlg[ imgMapSeamPix[i] ] == 3) N_eNode++;
	}


	//generate a graph
	const int SourceID = N_pNode + N_eNode   ;
	const int SinkID   = N_pNode + N_eNode +1;
	TFlowNetwork_BK4 graph( N_pNode + N_eNode + 2, N_pNode * 6 + N_eNode * 4, SourceID, SinkID);

	int seamNodeI = 0;

	//gen n-Link (img[S]-img[T])
	for (int S = 0; S < WH; ++S) if( imgNodeID[ S ] >= 0)
	{
		const int x = S % W;
		const int y = S / W;

		for (int k = 0; k < 3; ++k)
		{
			if( k==0 && x == W-1) continue;
			if( k==1 && y == H-1) continue;
			if( k==2 && imgMapSeamPix[S]==-1) continue;

			const int    T     = (k==0)? S + 1:
				                 (k==1)? S + W: imgMapSeamPix[S];
			if( imgNodeID[T] == -1 ) continue;
			
			const double eC    = (k==0)? pixInfo[S].eCostX : 
				                 (k==1)? pixInfo[S].eCostY :  pixInfo[S].eCostS;
			const double capa  = t_calcEdgeVal( &imgA[4*S], &imgB[4*S], &imgA[4*T], &imgB[4*T]);

			if ( eC > 0 )
			{
				const int si = N_pNode + seamNodeI;
				graph.add_nLink(imgNodeID[S], si, capa);
				graph.add_nLink(si, imgNodeID[T], capa);
				graph.add_tLink(SourceID, SinkID, si, 0, eC);
				seamNodeI++;
			}
			else
				graph.add_nLink(imgNodeID[S], imgNodeID[T], capa);
		}
	}
	fprintf( stderr, "a");

	//gen t Link (strict constrains)
	for (int y = 0; y < H; ++y)
	{
		for (int x = 0; x < W; ++x)
		{
			const int I = x + y*W;
			if ( imgNodeID[I] == -1 ) continue;

			bool bAdjA = (x > 0 && imgFlg[I-1] == 1) || (y > 0 && imgFlg[I-W] == 1) || (x < W-1 && imgFlg[I+1] == 1) || (y < H-1 && imgFlg[I+W] == 1);
			bool bAdjB = (x > 0 && imgFlg[I-1] == 2) || (y > 0 && imgFlg[I-W] == 2) || (x < W-1 && imgFlg[I+1] == 2) || (y < H-1 && imgFlg[I+W] == 2);
		

			double Et_A = ( bAdjA && !bAdjB) ? 100000 : (!bAdjA &&  bAdjB) ? 0      : GRAPH_DATATERM_COEF * imgA_agl[I];
			double Et_B = ( bAdjA && !bAdjB) ? 0      : (!bAdjA &&  bAdjB) ? 100000 : GRAPH_DATATERM_COEF * imgB_agl[I];
		
			graph.add_tLink(SourceID, SinkID, imgNodeID[I], Et_A, Et_B);
		}
	}
	fprintf( stderr, "a");

	//graph cut
	byte *minCut = new byte[N_pNode + N_eNode + 2];
	graph.calcMaxFlow(SourceID, SinkID, minCut);

	//add new edge
	for (int y = 0; y < H; ++y)
	{
		for (int x = 0; x < W; ++x)
		{
			const int S  = x + y * W, Tx = S + 1, Ty = S + W, Ts = imgMapSeamPix[S];
			if (x < W - 1 && imgFlg[S] == 3 && imgFlg[Tx] == 3 && minCut[imgNodeID[S]] != minCut[imgNodeID[Tx]])
			{
				pixInfo[S].eCostX = t_calcEdgeVal( &imgA[4*S], &imgB[4*S], &imgA[4*Tx], &imgB[4*Tx]);
			}
			if (y < H - 1 && imgFlg[S] == 3 && imgFlg[Ty] == 3 && minCut[imgNodeID[S]] != minCut[imgNodeID[Ty]])
			{
				pixInfo[S].eCostY = t_calcEdgeVal( &imgA[4*S], &imgB[4*S], &imgA[4*Ty], &imgB[4*Ty]);
			}
			if (Ts != -1  && imgFlg[S] == 3 && imgFlg[Ts] == 3 && minCut[imgNodeID[S]] != minCut[imgNodeID[Ts]])
			{
				pixInfo[S].eCostS = t_calcEdgeVal( &imgA[4*S], &imgB[4*S], &imgA[4*Ts], &imgB[4*Ts]);
			}

		}
	}
	fprintf( stderr, "a");

	//update image

	for (int i = 0; i < WH; ++i) 
	{
		const int I4 = i * 4;

		if(      imgFlg[i] != 0 && ( imgFlg[i] == 1 || (imgFlg[i] == 3 && minCut[imgNodeID[i]] )) ) // imgA
		{
			memcpy(&resImg[I4], &imgA[I4], sizeof(byte) * 4);
			resImg_agl[i] = imgA_agl[i];
		}
		else if( imgFlg[i] != 0 && ( imgFlg[i] == 2 || (imgFlg[i] == 3 && !minCut[imgNodeID[i]] )) )// imgB
		{
			memcpy(&resImg[I4], &imgB[I4], sizeof(byte) * 4);
			resImg_agl[i] = imgB_agl[i];
			pixInfo[i].patchID = newPatchId;
		}
		else 
		{
			if(imgA[I4+3] == 2) memcpy( &resImg[i*4], BACK_COL, sizeof(byte)*4 );
			else                memcpy( &resImg[i*4], FORE_NAN, sizeof(byte)*4 );
		}
	}
	fprintf( stderr, "a");

	//remove unnecessary edge
	for (int y = 0; y < H; ++y)
	{
		for (int x = 0; x < W; ++x)
		{
			const int S  = x + y * W, Tx = S + 1, Ty = S + W, Ts = imgMapSeamPix[S];;
			if( x < W - 1 && pixInfo[S].patchID == pixInfo[Tx].patchID ) pixInfo[S].eCostX = 0;
			if( y < H - 1 && pixInfo[S].patchID == pixInfo[Ty].patchID ) pixInfo[S].eCostY = 0;
			if( 0<=Ts&&Ts<WH && pixInfo[S].patchID == pixInfo[Ts].patchID ) pixInfo[S].eCostS = 0;
		}
	}
	fprintf( stderr, "b");

	delete[] imgFlg;
	delete[] imgNodeID;
	delete[] minCut;
}










static byte Col[16][4] = { 
	{64,64,64,0},
	{255,0,0,0  }, {0,255,0,0}, {0,0,255,0}, 
	{128,128,0,0}, {128,0,128,0}, {0,128,128,0}, 
	{128,0,0,0  }, {0,128,0,0}, {0,0,128,0}, 
	{255,255,0,0}, {0,255,255,0}, {255,0,255,0},
	{64,196,0,0}, {196,64,0,0}, {196,0,64,0}, 
};




static void t_exportTmpImgs
(
	const int W, const int H,
	CString str,
	const byte *rgbA, //new  image rgba --  a: 0:fore,1fore&NoVal, 2:back
	const byte *rgbB, //new  image rgba --  a: 0:fore,1fore&NoVal, 2:back
	const byte *rgbC, //new  image rgba --  a: 0:fore,1fore&NoVal, 2:back
	const float *fA,
	const float *fB,
	const float *fC, 
	PixInfo* pixInfo
)
{
	const int WH = W*H;
	float *tmpfA = new float[WH];
	float *tmpfB = new float[WH];
	float *tmpfC = new float[WH];

	for (int i = 0; i < WH; ++i)
	{
		tmpfA[i] = fA[i];
		tmpfB[i] = fB[i];
		tmpfC[i] = fC[i];
		if (rgbA[4 * i + 3] == 2)
		{
			tmpfA[i] = 0.1f;
			tmpfB[i] = 0.1f;
			tmpfC[i] = 0.1f;
		}
		
	}


	t_exportBmp    ( str + "_rA.bmp", W,H, rgbA , 1);
	t_exportBmp    ( str + "_rB.bmp", W,H, rgbB , 1);
	t_exportBmp    ( str + "_rC.bmp", W,H, rgbC , 1);
	t_ExportGrayBmp( str + "_nA.bmp" , W, H, 255.0f, tmpfA,1 );
	t_ExportGrayBmp( str + "_nB.bmp" , W, H, 255.0f, tmpfB,1 );
	t_ExportGrayBmp( str + "_nC.bmp" , W, H, 255.0f, tmpfC,1 );


	//export
	byte *patchId = new byte[ W * H * 4 ];
	byte *seamImg = new byte[ W * H * 4 ];
	for (int i = 0; i < W*H; ++i)
	{
		if (rgbA[4 * i + 3] == 2)
		{
			memcpy( &patchId[4*i], BACK_COL, sizeof(byte) * 4);
			memcpy( &seamImg[4*i], BACK_COL, sizeof(byte) * 4);
		}
		else
		{
			int ci = (pixInfo[i].patchID + 1) % 16;
			memcpy( &patchId[4*i], Col[ci], sizeof(byte) * 4);
			seamImg[4*i + 0] = pixInfo[i].eCostX > 0 || pixInfo[i].eCostY > 0 ? 255 : 0;
		}
	}
	t_exportBmp    ( str + "patch.bmp" , W,H, patchId, 1);
	t_exportBmp    ( str + "seam.bmp"  , W,H, seamImg, 1);

	delete[] patchId;
	delete[] seamImg;
	delete[] tmpfA;
	delete[] tmpfB;
	delete[] tmpfC;
}





//textures: alpha value 0:fore, 1:fore&NoValue, 2:background
//imgMapSeamPix[i] : -1: non seam pix,  >= 0 : seam pix (target pixel index)
void TextureBlender::BlendTexture(
	const vector<TImage2D>   &textures, 
	const vector<float*  >   &textures_angle, 
	const int                *imgMapSeamPix,
	TImage2D &resTex
)
{
	const int W = textures[0].m_W;
	const int H = textures[0].m_H;
	const int WH = W*H;

	PixInfo *pixInfo      = new PixInfo[WH  ];
	byte    *resImg       = new byte   [WH*4];
	float   *resImg_angle = new float  [WH  ];


	//一枚目 配置
	memcpy(resImg      , textures[0].m_RGBA, sizeof(byte ) * WH * 4);
	memcpy(resImg_angle, textures_angle[0] , sizeof(float) * WH    );
	for( int i=0; i < WH; ++i)
	{
		pixInfo[i].patchID = (resImg[i*4+3] == 0) ? 0 : -1;
	}

	//2枚目以降を追加 (by graph cut) 
	for( int k = 1; k < (int)textures.size(); ++k)
	{
		fprintf( stderr, "add patch ... ");

		byte  *tmp       = new byte [ WH*4 ];
		float *tmp_angle = new float[ WH   ];
		t_graphCutTexture_placePatch ( W, H, imgMapSeamPix, resImg, resImg_angle, textures[k].m_RGBA, textures_angle[k], tmp, tmp_angle, pixInfo, k );
		

		//dbg -------------------------------------
		CString str;
		str.Format("./TmpImg/res%d", k);
		t_exportTmpImgs(W,H, str, resImg, textures[k].m_RGBA, tmp, resImg_angle, textures_angle[k],tmp_angle, pixInfo );
		//dbg-------------------------------------

		memcpy( resImg      , tmp      , sizeof(byte ) * WH*4);
		memcpy( resImg_angle, tmp_angle, sizeof(float) * WH  );
		
		delete[] tmp;
		delete[] tmp_angle;

		fprintf( stderr, "Done!!!!\n\n");
	}


	resTex.AllocateImage( W,H, 0);
	memcpy(resTex.m_RGBA, resImg, sizeof(byte)*W*H*4);

	delete[] pixInfo;
	delete[] resImg ;
	delete[] resImg_angle;

}
