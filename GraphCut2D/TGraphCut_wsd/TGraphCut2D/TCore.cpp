#include "StdAfx.h"
#include "TCore.h"

#include "../../MaxFlowAlgorithms/TMaxFlow_BK4.h"// BK_algorithm (高速化済み)
#include "../../MaxFlowAlgorithms/TMaxFlow_BK.h" // BK_algorithm
#include "../../MaxFlowAlgorithms/TMaxFlow_Di.h" // Di_algorithm
#include "../../MaxFlowAlgorithms/TMaxFlow_EK.h"  //EK_algorithm
#include "../../MaxFlowAlgorithms/TMaxFlow_PR.h" // Push and Relabeling algorithm 
#include "TWatershedEx.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#endif




//byte *sobelX, byte sobelY should be allocated before
//color [0,255]-->[0,1]
static void t_sobel( const byte *rgba, const int W, const int H, double *sobelX, double *sobelY)
{
	double filterX[3][3]={	{-1,  0,  1}, {-2,  0,  2}, {-1,  0,  1}};
	double filterY[3][3]={  {-1, -2, -1}, { 0,  0,  0}, { 1,  2,  1}};

	for(int y = 0, idx=0; y < H ; y++)
	for(int x = 0       ; x < W ; x++, ++idx)
	{
		double vx = 0, vy = 0 ;		
		for( int wy = -1; wy < 2; ++wy)
		for( int wx = -1; wx < 2; ++wx)
		{
			int pixId = 4*( idx + wx + wy * W );
			double pixVal;
			if( x + wx < 0 || x + wx > W-1 || y + wy < 0 || y + wy > H-1 ) pixVal = 0;
			else pixVal	= (rgba[ pixId + 0] + rgba[ pixId + 1] + rgba[ pixId + 2] ) * 0.33333 / 255.0;
			vx += filterX[wy+1][wx+1] * pixVal;
			vy += filterY[wy+1][wx+1] * pixVal;
		}
		sobelX[ idx ] = fabs( vx ) ;
		sobelY[ idx ] = fabs( vy ) ;
	}
}





//watershed pixel (watershed segmentationの境界pixel) を近傍の領域に関連付ける
static void t_collapseWsdPixel( const int W, const int H, const byte *imgRGBA, int *pix_wsdLabel)
{
	const int WH = W*H;

	//平均color計算
	int maxLabel = 0;
	for( int i=0; i < WH; ++i) maxLabel = max( maxLabel, pix_wsdLabel[i] );

	vector< TVector3 > labelColor( maxLabel+1    );
	vector< int      > labelNum  ( maxLabel+1, 0 );

	for( int i = 0; i < WH; ++i){
		labelColor[ pix_wsdLabel[i] ] += TVector3( imgRGBA[4*i  ], imgRGBA[4*i+1], imgRGBA[4*i+2]);
		labelNum  [ pix_wsdLabel[i] ] += 1;
	}
	for( int i = 0; i < maxLabel+1; ++i){
		labelColor[ i ] /= labelNum[i];
	}

	//watershed pixel (値0)を近傍の領域に関連づける
	for( int y = 0; y < H; ++y)	
	for( int x = 0; x < W; ++x) if( pix_wsdLabel[ x + y*W ] == 0 )
	{	
		const int idx = x + y*W;
		double colDiff = DBL_MAX;
		for( int c = 0; c<4; ++c)
		{
			int neiIdx = 0;
			if     ( c == 0 && x >   0	) neiIdx = idx - 1    ;//左
			else if( c == 1 && x < W-1	) neiIdx = idx + 1    ;//右
			else if( c == 2 && y >   0	) neiIdx = idx - W    ;//下
			else if( c == 3 && y < H-1  ) neiIdx = idx + W    ;//上
			else continue;

			if( pix_wsdLabel[ neiIdx ] > 0 )
			{
				double r = labelColor[ pix_wsdLabel[ neiIdx ] ].data[0];
				double g = labelColor[ pix_wsdLabel[ neiIdx ] ].data[1];
				double b = labelColor[ pix_wsdLabel[ neiIdx ] ].data[2];
				double d =   (imgRGBA[ idx*4   ] - r)*(imgRGBA[ idx*4   ] - r)
						    +(imgRGBA[ idx*4+1 ] - g)*(imgRGBA[ idx*4+1 ] - g)
							+(imgRGBA[ idx*4+2 ] - b)*(imgRGBA[ idx*4+2 ] - b);

				if( d < colDiff )
				{ 
					colDiff = d; 
					pix_wsdLabel[idx] = pix_wsdLabel[ neiIdx ];
				}
			}
		}
	}
}
















TCore::~TCore(void)
{
	delete[] m_wsdNodes;
	delete[] m_pix_wsdLabel;
}

TCore::TCore(void)
{
	//最大フローアルゴリズムのテスト （実際のMax Flow Algorithmの使い方は test()関数内を参照 ）
	TFlowNetwork_PR ::test_simple        ();
	TFlowNetwork_PR ::test_relabelToFront();
	TFlowNetwork_EK ::test();
	TFlowNetwork_Di ::test();
	TFlowNetwork_BK ::test();
	TFlowNetwork_BK4::test();



	//取説ダイアログ表示
	m_dlg.Create( IDD_DIALOG_1 );
	m_dlg.ShowWindow( SW_SHOW );

	//0, load image-----------------------------------------------------------------------------------------
	CString         filter("bmp Files (*.bmp;*.jpg;*.tif)|*.bmp;*.jpg;*.tif||");
	CFileDialog     selDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
	bool inverted;
	if (selDlg.DoModal() == IDOK) m_imgOrig.allocateFromFile( selDlg.GetPathName(), inverted, &m_ogl );
	else exit(1);
	if( inverted ) m_imgOrig.flipImageInY();

	m_fname = selDlg.GetFileName();
	                                           
	m_imgDisp1.allocateImage( m_imgOrig, 0 ); 
	m_imgDisp2.allocateImage( m_imgOrig, 0 ); 
	m_imgDisp3.allocateImage( m_imgOrig, 0 ); 
	m_imgOrig .m_DoInterpolation = false;
	m_imgDisp1.m_DoInterpolation = false;
	m_imgDisp2.m_DoInterpolation = false;
	m_imgDisp3.m_DoInterpolation = false;


	//1.  image size -----------------------------------------------------------------------------------------
	const int W = m_imgOrig.m_width ;
	const int H = m_imgOrig.m_height;
	const int WH = W*H;
	m_imgRectW = IMG_RECT_W;
	m_imgRectH = IMG_RECT_W / (double) W * H;



	//2.  watershed 領域分割を行い、superpixel (m_wsdNodes) を構築する----------------------------------------
	fprintf( stderr, "start watershed segmentation ..." );


	//2.1  sobelフィルタにより勾配強度画像計算 
	double *sobelX   = new double[ WH ];
	double *sobelY   = new double[ WH ];
	byte   *gradMag  = new byte  [ WH ];
	t_sobel( m_imgOrig.m_RGBA, W, H, sobelX, sobelY);
	double maxVal = - DBL_MAX;
	for( int i=0; i<WH; ++i) maxVal     = max( maxVal,  sqrt( sobelX[i]*sobelX[i] + sobelY[i]*sobelY[i])          );
	for( int i=0; i<WH; ++i) gradMag[i] = (byte)( 255 * sqrt( sobelX[i]*sobelX[i] + sobelY[i]*sobelY[i]) / maxVal );

	//2.2  watershed segmentation 
	m_pix_wsdLabel = new int[WH];
	TWatershed2DEx    ( W, H, gradMag         , m_pix_wsdLabel); // 各pixelにラベル(領域ID)がつけられる ラベル値0は境界 (watershed pixelと呼ばれる)
	t_collapseWsdPixel( W, H, m_imgOrig.m_RGBA, m_pix_wsdLabel); // 境界pixelを 色の近い近傍領域に融合させる
	for( int i=0; i < WH; ++i) --m_pix_wsdLabel[i];

	m_wsdMaxLabel = 0;
	for( int i=0; i < WH; ++i) m_wsdMaxLabel = max( m_wsdMaxLabel, m_pix_wsdLabel[i] );

	//2.3 prepare super pixel 平均色・pixel数・n-link構築 
	m_wsdNodes = new TWsdNode[ m_wsdMaxLabel + 1 ];

	for( int y = 0; y < H ; ++y)
	for( int x = 0; x < W ; ++x)
	{
		const int i = x + y*W;
		m_wsdNodes[ m_pix_wsdLabel[i] ].m_color  += TVector3( m_imgOrig.m_RGBA[ 4*i ], m_imgOrig.m_RGBA[4*i+1], m_imgOrig.m_RGBA[4*i+2]);
		m_wsdNodes[ m_pix_wsdLabel[i] ].m_pixNum += 1;
		//edgeを挿入. 右と下のpixelが異なる super pixelならば辺を追加 ( 辺は ラベル値が小さい方から大きい方へ ) 
		if( x < W - 1 && m_pix_wsdLabel[i] < m_pix_wsdLabel[i+1] ) m_wsdNodes[ m_pix_wsdLabel[ i   ] ].m_nLink.insert( m_pix_wsdLabel[ i+1 ] );
		if( x < W - 1 && m_pix_wsdLabel[i] > m_pix_wsdLabel[i+1] ) m_wsdNodes[ m_pix_wsdLabel[ i+1 ] ].m_nLink.insert( m_pix_wsdLabel[ i   ] );
		if( y < H - 1 && m_pix_wsdLabel[i] < m_pix_wsdLabel[i+W] ) m_wsdNodes[ m_pix_wsdLabel[ i   ] ].m_nLink.insert( m_pix_wsdLabel[ i+W ] );
		if( y < H - 1 && m_pix_wsdLabel[i] > m_pix_wsdLabel[i+W] ) m_wsdNodes[ m_pix_wsdLabel[ i+W ] ].m_nLink.insert( m_pix_wsdLabel[ i   ] );
	}
	for( int i = 0; i < m_wsdMaxLabel+1; ++i) m_wsdNodes[ i ].m_color /= m_wsdNodes[ i ].m_pixNum;
	
	delete[] sobelX ;
	delete[] sobelY ;
	delete[] gradMag;

	fprintf( stderr, "画像を初期化しました. super pixel数は : %d \n", m_wsdMaxLabel+1 );

	for( int i = 0; i < WH; ++i) {
		m_imgDisp2.setColor( i*4, m_wsdNodes[ m_pix_wsdLabel[i] ].m_color );
		m_imgDisp3.setColor( i*4, m_wsdNodes[ m_pix_wsdLabel[i] ].m_color );
	}
}





void TCore::saveMaskImage()
{
	CString f = m_fname.Mid(0, m_fname.GetLength() - 4) + "_MASK.bmp";
	//load image
	CString       filter("bmp Files (*.bmp;*.bmp)|*.bmp; *.bmp||");
	CFileDialog   dlg(FALSE, NULL, f, OFN_HIDEREADONLY | OFN_CREATEPROMPT, filter);

	if(dlg.DoModal() != IDOK) return;

	CString cstr = dlg.GetPathName();
	string fname( (LPCTSTR) cstr);
	if( dlg.GetFileExt() != "bmp" ) fname += string(".bmp");

	TOGL2DImage img( m_imgDisp2 );

	for( int i = 0, s = img.m_width * img.m_height; i < s; ++i)
	{
		const int i4 = 4*i;
		byte c = ( img.m_RGBA[i4]==0 && img.m_RGBA[i4+1]==0 && img.m_RGBA[i4+2]==128 ) ? 0 : 255;
		img.m_RGBA[i4] = img.m_RGBA[i4+1] = img.m_RGBA[i4+2] = c;
	}

	img.flipImageInY();
	img.saveAsFile( fname.c_str(), 0 );
	img.flipImageInY();
}







/////////////////////////////////////////////////////////////////////////////////////
//画像の描画 (OpenGLを利用) /////////////////////////////////////////////////////////
static void drawImage( const double rectW, const double rectH, TOGL2DImage &img)
{
	glDisable( GL_LIGHTING  );
	glDisable( GL_CULL_FACE );
	glColor3d( 1,1,1        );
	glEnable( GL_TEXTURE_2D );
	img.bind(0);
	glBegin  ( GL_QUADS     );
	glTexCoord2d(0,0); glVertex3d(     0,    0, 0 );
	glTexCoord2d(1,0); glVertex3d( rectW,    0, 0 );
	glTexCoord2d(1,1); glVertex3d( rectW,rectH, 0 );
	glTexCoord2d(0,1); glVertex3d(     0,rectH, 0 );
	glEnd( );
	glDisable( GL_TEXTURE_2D );
}


static void drawCpRect(  const double x, const double y, const double rectR )
{
	glBegin  ( GL_QUADS     );
	glTexCoord2d(0,0); glVertex3d( x-rectR,y-rectR, 1 );
	glTexCoord2d(1,0); glVertex3d( x+rectR,y-rectR, 1 );
	glTexCoord2d(1,1); glVertex3d( x+rectR,y+rectR, 1 );
	glTexCoord2d(0,1); glVertex3d( x-rectR,y+rectR, 1 );
	glEnd( );
}

static void drawRectLine( const double width, const double height )
{
	glDisable( GL_LIGHTING );
	glLineWidth( 5 );
	glColor3d( 1,1,0);
	glBegin( GL_LINE_STRIP );
		glVertex3d(0,0,0);
		glVertex3d(width,0,0);
		glVertex3d(width,height,0);
		glVertex3d(0,height,0);
		glVertex3d(0,0,0);
	glEnd();
}



void TCore::drawScene()
{
	const int W = m_imgOrig.m_width  ;
	const int H = m_imgOrig.m_height ;	

	//画像を表示 
	glPushMatrix();
		drawRectLine(m_imgRectW, m_imgRectH );
		drawImage( m_imgRectW, m_imgRectH, m_imgDisp1 );
		glTranslated( m_imgRectW * 1.25, 0,0); drawImage( m_imgRectW, m_imgRectH, m_imgDisp2 );
		glTranslated( m_imgRectW * 1.1 , 0,0); drawImage( m_imgRectW, m_imgRectH, m_imgDisp3 );	
	glPopMatrix();

	//制御点を表示
	glDisable( GL_LIGHTING );
	for( int i=0,s=(int)m_CPs.size(); i<s; ++i){
		if( m_CPs[i].m_fb ) glColor3d( 1,0,0 );
		else                glColor3d( 0,0,1 );
		drawCpRect( m_CPs[i].m_x, m_CPs[i].m_y, CP_RECT_SIZE );
	}

}








//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//画像領域分割本体////////////////////////////////////////////////////////////////////

static inline double distSq( const TVector3 &c1, const TVector3 &c2){
	return (c1.data[0]-c2.data[0]) * (c1.data[0]-c2.data[0]) + 
		   (c1.data[1]-c2.data[1]) * (c1.data[1]-c2.data[1]) + 
		   (c1.data[2]-c2.data[2]) * (c1.data[2]-c2.data[2]);
}

static inline double calcE2( const TVector3 &c0, const TVector3 &c1 ){
	return  1 / ( 1.0 +  distSq( c0, c1) );
}

static inline double calcE2( const double &r0, const double &g0, const double &b0, 
	                         const double &r1, const double &g1, const double &b1, double lambda )
{
	double d = (r1-r0)*(r1-r0) +
		       (g1-g0)*(g1-g0) + 
			   (b1-b0)*(b1-b0) ;
	return  lambda / ( 1.0 + d );
}




static inline void calcE1(  const vector<TVector3> &f_Cs,
							const vector<TVector3> &b_Cs,
							const TVector3         &c,
							double &ef,//fore energy
							double &eb)//back energy
{
	if( f_Cs.size() == 0 || b_Cs.size() ) {ef = eb = 0.5; return;}

	double df = DBL_MAX;
	double db = DBL_MAX;
	for( int i=0, s=(int)f_Cs.size(); i<s; ++i) df = min(df, distSq( f_Cs[i], c ) );
	for( int i=0, s=(int)b_Cs.size(); i<s; ++i) db = min(db, distSq( b_Cs[i], c ) );
	df = sqrt( df );
	db = sqrt( db );

	if( df == 0 && db == 0 ) ef = eb = 0.5;
	else{
		ef =  db / ( df + db );
		eb =  df / ( df + db );
	}
}













/*--------------------------------------------------------------------------------//
Watershed segmentation のsuper pixel levelでグラフカット方により領域分割を計算する//
----------------------------------------------------------------------------------*/
double GraphCutSegmentation_SuperpixelLv
	(
	const int       wsdNodeNum ,  //入力 superpixel数
	const TWsdNode *wsdNodes   ,  //入力 superpixelの配列
	const set<int> &foreNodeIds,  //入力 前景制約 super pixel
	const set<int> &backNodeIds,  //入力 背景制約 super pixel
	      byte     *wsdNode_InOut //出力 各super pixelが前景か背景かのフラグ
	) 
{

	const int sourceID = wsdNodeNum     ;
	const int sinkID   = wsdNodeNum + 1 ;

	int nLinkNum = 0 ;
	for( int i=0; i < wsdNodeNum; ++i) nLinkNum += wsdNodes[i].m_nLink.size(); 


	//1. 制約の抽出 -------------------------------------------------------------
	byte *node_condi = new byte[ wsdNodeNum ];
	memset( node_condi, 0, sizeof( byte ) * wsdNodeNum );

	vector<TVector3> foreCs, backCs; 
	foreCs.reserve(foreNodeIds.size()); 
	backCs.reserve(backNodeIds.size());  
	for( set<int>::const_iterator it = foreNodeIds.begin(); it != foreNodeIds.end(); ++it ) {
		foreCs.push_back( wsdNodes[ *it ].m_color );
		node_condi[*it] = 1;
	}
	for( set<int>::const_iterator it = backNodeIds.begin(); it != backNodeIds.end(); ++it ) {
		backCs.push_back( wsdNodes[ *it ].m_color );
		node_condi[*it] = 2; 
	}


	//2. フローネットワーク構築 ---------------------------------------------------
	//TFlowNetwork_EK  network( nodeNum + 2, nLinkNum * 2 + nodeNum * 2 );
	//TFlowNetwork_Di  network( nodeNum + 2, nLinkNum * 2 + nodeNum * 2 );
	//TFlowNetwork_BK  network( nodeNum + 2, nLinkNum * 2 + nodeNum * 2 );
	TFlowNetwork_BK4  network( wsdNodeNum + 2,  nLinkNum * 2 + wsdNodeNum * 4, sourceID, sinkID );

	// insert t-link (compute E1)
	for(int i = 0; i <wsdNodeNum; ++i )
	{
		double e1_f = 0, e1_b = 0;
		if(      node_condi[i] == 0 ) calcE1( foreCs, backCs, wsdNodes[i].m_color, e1_f, e1_b);
		else if( node_condi[i] == 1 ) e1_f = GRAPHCUT_MAXFLOW;
		else                          e1_b = GRAPHCUT_MAXFLOW;

		network.add_tLink( sourceID, sinkID, i, e1_f, e1_b);
	}

	// insert n-link (compute E2)
	for(int i = 0; i <wsdNodeNum; ++i) 
	{
		for( set<int>::const_iterator eIt = wsdNodes[i].m_nLink.begin(); eIt != wsdNodes[i].m_nLink.end(); ++eIt)
		{
			network.add_nLink(i, *eIt, calcE2( wsdNodes[ i ].m_color, wsdNodes[ *eIt ].m_color ) * LAMBDA );
		}
	}
	
	//3. Max Flow 計算  -----------------------------------------------------------
	byte *node_inOut = new byte[ wsdNodeNum + 2 ];
	double flow = network.calcMaxFlow( sourceID, sinkID, node_inOut );
	memcpy( wsdNode_InOut, node_inOut, sizeof(byte) * wsdNodeNum );//souceとsinkの分だけサイズが違うのでこの実装(ちょっとダサい)
	
	delete[] node_condi;
	delete[] node_inOut;
	return flow ;
}





/*----------------------------------------------------------------------------------
境界からbandWidth分の帯状の部分についてのみGraph Cut法を適用し前景背景を計算する////
----------------------------------------------------------------------------------*/
double GraphCutSegmentation_PixelLv
	( 
	const int        W          , // 入力 画像サイズ width
	const int        H          , // 入力 画像サイズ height
	const byte      *rgba       , // 入力 画素値 (r1 g1 b1 r2 g2 b2...の順序 )
	const set<int>  &forePixIds , // 入力 前景制約画素
	const set<int>  &backPixIds , // 入力 背景制約画素
	const int        bandWidth  , // 入力 帯状領域の幅
	      byte      *pixel_inOut  // 入力 かつ 出力 各画素が前景か背景かのフラグ
	)
{


	//0. 制約画素から 色を抽出
	vector< TVector3 > foreColors, backColors;
	foreColors.reserve( forePixIds.size() );
	backColors.reserve( backPixIds.size() );
	for( const auto& it: forePixIds) foreColors.push_back( TVector3( rgba[4*it], rgba[4*it+1], rgba[4*it+2] ) );
	for( const auto& it: backPixIds) backColors.push_back( TVector3( rgba[4*it], rgba[4*it+1], rgba[4*it+2] ) );



	//1. 帯上領域構築---------------------------------------------------------------------
	byte *pixelStates = new byte [ W*H ]; //0 : 帯状領域の外,  1: 帯状領域内
	memset( pixelStates, 0, sizeof( byte )* W*H );

	for( int y = 1; y < H; ++y)
	for( int x = 1; x < W; ++x) //手前との差をみるから1からスタート
	{
		const int idx = x + y * W;
		if( pixel_inOut[ idx ] != pixel_inOut[ idx-1 ] || pixel_inOut[ idx ] != pixel_inOut[ idx-W ])
		{
			for( int y_off = -bandWidth; y_off <= bandWidth; ++y_off)
			for( int x_off = -bandWidth; x_off <= bandWidth; ++x_off)
			if( 0 <= x+x_off && x+x_off < W && 0 <= y+y_off && y+y_off < H) pixelStates[idx + x_off + y_off * W] = 1;
		}
	}


	//2. 帯状領域内のpixelのindexを計算 (帯状領域内のpixelのみにindexを付け直す)----------
	int  *pix2node = new int[ W*H ]; //fore=-1 back=-2, 計算対象ノード には　0以上の indexが入る
	vector< TPixelNode* > pixNodes;  
	pixNodes.reserve( W*H / 4 );

	for( int i = 0, s=W*H; i < s; ++i) 
	{
		if( pixelStates[i] == 1)
		{
			pix2node[ i ] = (int) pixNodes.size();
			pixNodes.push_back( new TPixelNode( i, 0, rgba[4*i], rgba[4*i+1], rgba[4*i+2] ) );
		}
		else
			pix2node[i] = pixel_inOut[i] ? -1 : -2;
	}

	for( const auto& it: forePixIds) if( pix2node[it] >= 0) pixNodes[pix2node[it]]->m_bConst = 1;
	for( const auto& it: backPixIds) if( pix2node[it] >= 0) pixNodes[pix2node[it]]->m_bConst = 2;





	//3. graph構築-------------------------------------------------------------------------
	const int nodeNum  = pixNodes.size();
	const int sourceID = nodeNum    ;
	const int sinkID   = nodeNum + 1;

	//TFlowNetwork_EK network( nodeNum + 2, nodeNum * 6 );//好きなアルゴリズムを選ぶ
	//TFlowNetwork_Di network( nodeNum + 2, nodeNum * 6 );
	//TFlowNetwork_BK network( nodeNum + 2, nodeNum * 6 );
	TFlowNetwork_BK4 network( nodeNum + 2, nodeNum * 6, sourceID, sinkID ); //これが最速

	for( int nodeI = 0; nodeI < (int) pixNodes.size(); ++nodeI)
	{
		const int pixI = pixNodes[nodeI]->m_pixID;
		const int pixX = pixI %  W;
		const int pixY = pixI /  W;
		bool edgeToR = false, edgeToB = false;
		
		int pixCondition = 0;// -1 backpixelが隣接  1 forepixelが隣接
		//上下左右をみてfore/backに隣接していたら、E1f/E1b = 無限
		if( pixX != 0   ) { if     ( pix2node[ pixI-1 ] == -1 ) pixCondition =  1;
			                else if( pix2node[ pixI-1 ] == -2 ) pixCondition = -1;}
		if( pixY != 0   ){  if     ( pix2node[ pixI-W ] == -1 ) pixCondition =  1;
			                else if( pix2node[ pixI-W ] == -2 ) pixCondition = -1;}

		if( pixX != W-1 ) { if     ( pix2node[ pixI+1 ] == -1 ) pixCondition =  1;
							else if( pix2node[ pixI+1 ] == -2 ) pixCondition = -1; 
							else edgeToR = true; }
		if( pixY != H-1 ){  if     ( pix2node[ pixI+W ] == -1 ) pixCondition =  1;
							else if( pix2node[ pixI+W ] == -2 ) pixCondition = -1; 
							else edgeToB = true; }

		double E1_f = 0.5;
		double E1_b = 0.5;

		if( pixNodes[nodeI]->m_bConst == 1 || pixCondition == 1) 
		{
			E1_f = GRAPHCUT_MAXFLOW;
			E1_b = 0;
		}
		if( pixNodes[nodeI]->m_bConst == 2 || pixCondition ==-1) 
		{
			E1_b = GRAPHCUT_MAXFLOW;
			E1_f = 0;
		}

		//if( pixCondition != 1 && pixCondition != -1) calcE1( foreColors, backColors, pixNodes[nodeI]->m_color, E1_f, E1_b); //ここあっても無くてもOK

		//t-link
		network.add_tLink( sourceID, sinkID, nodeI, E1_f, E1_b);

		//n-link
		if( edgeToR ) network.add_nLink( nodeI, pix2node[ pixI+1 ], calcE2( pixNodes[nodeI]->m_color, pixNodes[pix2node[ pixI+1 ]]->m_color) * LAMBDA );
		if( edgeToB ) network.add_nLink( nodeI, pix2node[ pixI+W ], calcE2( pixNodes[nodeI]->m_color, pixNodes[pix2node[ pixI+W ]]->m_color) * LAMBDA );
	}

	//4. MaxFlow Algorithm ----------------------------------------------------------------
	byte *minCut = new byte[ nodeNum + 2];
	double flow = network.calcMaxFlow( sourceID, sinkID, minCut );
	for( int i = 0, s=W*H; i < s; ++i) pixel_inOut[i] = ( pix2node[i] == -1    ) ?  1 : 
		                                                ( pix2node[i] == -2    ) ?  0 : 
														( minCut[ pix2node[i] ]) ?  1 : 0;

	//5.後処理 ---------------------------------------------------------------------------
	delete[] minCut     ;
	delete[] pixelStates;
	delete[] pix2node   ;
	for( int i= 0; i < (int) pixNodes.size(); ++i) delete pixNodes[i]; 
	pixNodes.clear();
	return flow;
}














struct PixelInfo{
	int x, y,idx;
	PixelInfo( int _x, int _y, int _idx) { x=_x; y=_y; idx=_idx;}
};


/*------------------------------------------------------------------
W×Hの二値画像 pix_foreBack が入力されると，
forPixIdsから連結している領域のみがforegroundとなるように修正する

入力    pix_foreBack[i] 0:背景 1:前景
計算中  pix_foreBack[i] 0:背景 1:前景(訪問前) 2:訪問後(前景)
出力    pix_foreBack[i] 0:背景 1:前景
------------------------------------------------------------------*/
static void extractConnectedRegion (const int W, const int H, const set<int> &forePixIds, byte* pix_foreBack) 
{
	const int WH = W*H;
	queue<PixelInfo> frontier;
	for( set<int>::const_iterator it = forePixIds.begin(); it != forePixIds.end(); ++it) 
	{
		int y = (*it) / W;
		int x = (*it) - y * W;
		frontier.push( PixelInfo( x, y, *it) );
		pix_foreBack[*it] = 2;
	}

	while( !frontier.empty() )
	{
		const PixelInfo piv = frontier.front(); frontier.pop();
		if( piv.x > 0   &&  pix_foreBack[piv.idx-1] == 1 ){ pix_foreBack[ piv.idx-1 ] = 2;  frontier.push( PixelInfo( piv.x-1, piv.y  , piv.idx-1 ) ); }
		if( piv.x < W-1 &&  pix_foreBack[piv.idx+1] == 1 ){ pix_foreBack[ piv.idx+1 ] = 2;  frontier.push( PixelInfo( piv.x+1, piv.y  , piv.idx+1 ) ); }
		if( piv.y > 0   &&  pix_foreBack[piv.idx-W] == 1 ){ pix_foreBack[ piv.idx-W ] = 2;  frontier.push( PixelInfo( piv.x  , piv.y-1, piv.idx-W ) ); }
		if( piv.y < H-1 &&  pix_foreBack[piv.idx+W] == 1 ){ pix_foreBack[ piv.idx+W ] = 2;  frontier.push( PixelInfo( piv.x  , piv.y+1, piv.idx+W ) ); }
	}
	for( int i=0; i<WH; ++i) pix_foreBack[i] = pix_foreBack[i] == 2 ? 1: 0;
}








void TCore::runGraphCutSegmentation()
{
	const int W = m_imgOrig.m_width ;
	const int H = m_imgOrig.m_height, WH = W*H;

	//0, 制約の置かれた pixel idx と node idx をまとめる
	set<int> setPix_F, setPix_B, setNode_F, setNode_B;
	for( const auto& p : m_CPs)
	{
		if( p.m_fb ) { setPix_F.insert( p.m_idx ); setNode_F.insert( m_pix_wsdLabel[ p.m_idx ] ); }
		else         { setPix_B.insert( p.m_idx ); setNode_B.insert( m_pix_wsdLabel[ p.m_idx ] ); }
	}

	byte *wsd_inOut = new byte[ m_wsdMaxLabel + 1 ]; //superpixel [i] が 前景か背景かのフラグ
	byte *pix_inOut = new byte[ WH                ]; //     pixel [i] が 前景か背景かのフラグ


	//1. GRAPHCUT segmentation in wsd-(superpixel-) level (wsdNode数は wsdMaxLabel + 1 )  
	double flowWsd  = GraphCutSegmentation_SuperpixelLv( m_wsdMaxLabel + 1, m_wsdNodes,  setNode_F, setNode_B, wsd_inOut); 
	fprintf( stderr, "compute superpixel level graph cut %f\n", flowWsd);

	//2. 制約画素に対する連結領域だけ抽出 (無くてもおｋ)
	for( int i=0; i < WH; ++i) pix_inOut[i] = wsd_inOut[ m_pix_wsdLabel[i] ] ? 1 : 0;
	extractConnectedRegion (W,H, setPix_F, pix_inOut); 

	//3. GRAPHCUT segmentation in pixel level
	double flowPix = GraphCutSegmentation_PixelLv( W, H, m_imgOrig.m_RGBA, setPix_F, setPix_B, 5, pix_inOut); 
	fprintf( stderr, "compute PIXEL      level graph cut %f\n", flowPix);


	//可視化用Image作成
	memcpy( m_imgDisp1.m_RGBA,  m_imgOrig.m_RGBA,  sizeof( byte ) * 4 * WH);
	memcpy( m_imgDisp2.m_RGBA,  m_imgOrig.m_RGBA,  sizeof( byte ) * 4 * WH);
	memcpy( m_imgDisp3.m_RGBA,  m_imgOrig.m_RGBA,  sizeof( byte ) * 4 * WH);

	for( int i=0; i < WH; ++i) 
	{
		if( !pix_inOut[i] ){
			m_imgDisp1.m_RGBA[ 4*i + 0 ] /= 2;
			m_imgDisp1.m_RGBA[ 4*i + 2 ] /= 2;

			m_imgDisp2.m_RGBA[ 4*i + 0 ] = 0  ;
			m_imgDisp2.m_RGBA[ 4*i + 1 ] = 0  ;
			m_imgDisp2.m_RGBA[ 4*i + 2 ] = 128;
		}
		if( !wsd_inOut[ m_pix_wsdLabel[i] ] ) m_imgDisp3.setColor( 4*i, 0,0,128);

	}
	delete[] wsd_inOut;
	delete[] pix_inOut ;

	m_imgDisp1.unbind( &m_ogl );
	m_imgDisp2.unbind( &m_ogl );
	m_imgDisp3.unbind( &m_ogl );
}



