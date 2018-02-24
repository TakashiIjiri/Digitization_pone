#pragma once

#include "TOGL.h"
#include "TDlgManual.h"
#include <vector> 
#include <set   > 


//fixed parameters//
#define IMG_RECT_W       7.0
#define CP_RECT_SIZE     0.01
#define GRAPHCUT_MAXFLOW 10000
#define LAMBDA           1000

using namespace std;






//graph cut segmentationの制約点
struct TConstPoint
{
public:
	bool   m_fb    ; //fore or back
	double m_x, m_y; //2次元空間の位置
	int    m_idx   ; //画素のindex

	void Set( double x, double y, int idx){
		m_x   = x        ;
		m_y   = y        ;
		m_idx = idx      ;
	}
	TConstPoint( double x, double y, bool fore_back, int idx )
	{
		m_fb  = fore_back;
		Set( x,y,idx );
	}
};



//watershed segmentationの Super Pixel
struct TWsdNode
{
	TVector3 m_color ;//このsuper pixelの平均色
	int      m_pixNum;//このsuper pixelに含まれる画素数
	set<int> m_nLink ;//このsuper pixelから出る n-link (ただし，自分のindexより大きなindexを持つsuper pixelにしかlinkを綱がない)
	TWsdNode( ){
		m_pixNum = 0;
		m_color.data[0] = m_color.data[1] = m_color.data[2] = 0;
	}
};



//pixel level graph cut 時のpixelノード (indexが画像上のpixelと変わるのでこのデータ構造が必要)
struct TPixelNode
{
	int      m_pixID;
	TVector3 m_color;
	byte     m_bConst; // 0:non const, 1:fore const, 2:back const
	inline TPixelNode(int pixelIdx, byte bConst, const byte r, const byte g, const byte b)
	{
		m_pixID = pixelIdx;
		m_bConst = bConst;
		m_color.data[0]=r;
		m_color.data[1]=g;
		m_color.data[2]=b;
	}
};










class TCore
{
	TCore(void);
public:
	~TCore(void);
	inline static TCore* getInst(){ static TCore p; return &p;}
	
	TOGL_2D m_ogl;
	double  m_imgRectW, m_imgRectH;
	vector< TConstPoint > m_CPs;//制約点

	TDlgManual m_dlg;

	CString m_fname;

	//画像データ
	TOGL2DImage  m_imgOrig  ; // 元画像
	TOGL2DImage  m_imgDisp1 ; // 表示用画像  
	TOGL2DImage  m_imgDisp2 ; // 表示用画像  Pixel     levelの領域分割結果
	TOGL2DImage  m_imgDisp3 ; // 表示用画像  Watershed levelの領域分割結果


	//watershed over segmentationのデータ構造
	int       m_wsdMaxLabel   ; //最大wsd label値
	int      *m_pix_wsdLabel  ; //各 pixel の watershed label (super pixel ID)
	TWsdNode *m_wsdNodes      ; //

	void  drawScene();
	void  runGraphCutSegmentation();
	void  saveMaskImage();


	//interface for control points//
	void addCP   ( const double x, const double y, const bool fb, const double checkOfst = 0){
		if( x< 0 || m_imgRectW < x || y<0 || m_imgRectH<y) return;


		if (m_CPs.size() != 0)
		{
			double d = sqrt( (x - m_CPs.back().m_x) * (x - m_CPs.back().m_x) + (y - m_CPs.back().m_y) * (y - m_CPs.back().m_y) ) ;
			if( d < checkOfst) return;
		}

		int xIdx = (int) ( x / m_imgRectW * (m_imgOrig.m_width -1));
		int yIdx = (int) ( y / m_imgRectH * (m_imgOrig.m_height-1));
		int idx  = xIdx + yIdx * m_imgOrig.m_width;

		m_CPs.push_back( TConstPoint(x,y,fb, idx) ); 
	}
	void moveCP  ( const int cpIdx, const double x, const double y){
		if( x< 0 || m_imgRectW < x || y<0 || m_imgRectH<y) return;

		int xIdx = (int) ( x / m_imgRectW * (m_imgOrig.m_width -1));
		int yIdx = (int) ( y / m_imgRectH * (m_imgOrig.m_height-1));
		int idx  = xIdx + yIdx * m_imgOrig.m_width;
		m_CPs[cpIdx].Set( x,y,idx)  ; 
	}
	void removeCP( const int cpIdx){m_CPs.erase( m_CPs.begin() + cpIdx ); }

	int  pickCP  ( const double x, const double y)
	{
		for( int i=0; i<(int)m_CPs.size(); ++i){
			if( m_CPs[i].m_x - CP_RECT_SIZE <= x && x <= m_CPs[i].m_x + CP_RECT_SIZE && 
				m_CPs[i].m_y - CP_RECT_SIZE <= y && y <= m_CPs[i].m_y + CP_RECT_SIZE ) return i; 
		}
		return -1;
	}

	void pickCPtoRemove(const double x, const double y)
	{
		int idx = pickCP(x,y);
		if( idx != -1) removeCP( idx );
	}

};

