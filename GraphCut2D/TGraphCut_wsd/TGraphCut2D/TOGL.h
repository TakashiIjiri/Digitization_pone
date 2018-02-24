#ifndef __TOGL_H_INCLUDED__
#define __TOGL_H_INCLUDED__

/*-----------------------------------------------
TOGL for OpenGL on MFC 
Written by Takashi Ijiri @ riken / 2011/10/6
call OnCreate () 
     OnSize   ()
	 OnDraw   ()
	 OnDestroy()   from ****View.h
------------------------------------------------*/
#pragma warning (disable:4786)
#pragma warning (disable:4996)


#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <algorithm>

#include <atlimage.h>


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")






class TVector3 {
public:
	double data[3];
	TVector3(){ data[0]=data[1]=data[2]=0; }
	TVector3(double f1,double f2,double f3){ data[0]=f1;data[1]=f2;data[2]=f3; }

	TVector3& operator/=(double f){ data[0]/=f; data[1]/=f; data[2]/=f; return *this; }
	TVector3& operator+=(const TVector3& vec1){
		data[0] += vec1.data[0];
		data[1] += vec1.data[1];
		data[2] += vec1.data[2];
		return *this ;
	}
	inline void Set( const double &f1, const double &f2, const double &f3){ 
		data[0] = f1; data[1] = f2; data[2] = f3;}

};







/*----------------------------------------------------

TOGL_2D
- 2D scene 描画(OpgnGL)のサポートを行うクラス
- OpenGLの初期化やリサイズ等の処理を行う
- 視点とZoomの管理も行う

----------------------------------------------------*/



class TOGL_2D
{
	CWnd*  m_pWnd;//trgt MFC view 
	CDC*   m_pDC ;//devide context of trgtMFC view
	HGLRC  m_hRC ;//open gl handle

	bool   m_bIsDrawing;//flag under rendering or not 
	int    m_cx, m_cy  ;//frame buffer size 
	double m_viewSize  ;//view size
	float  m_bkColor[4];

public:

	TOGL_2D() : m_pWnd(0), m_pDC(0), m_bIsDrawing(false), m_viewSize(10)
	{
		m_bkColor[0] = m_bkColor[1] = m_bkColor[2] = 1.0f; m_bkColor[3] = 0.5f ;
	}

	//OnCreate OnDestroy OnSize
	BOOL OnCreate(CWnd* pWnd)
	{
		m_pWnd = pWnd ;
		m_pDC  = new CClientDC(pWnd);
		if( !m_pDC              )	return FALSE;
		if( !SetupPixelFormat() )	return FALSE;
		
		m_hRC = wglCreateContext( m_pDC->GetSafeHdc() );
		if( !m_hRC                                        )	return FALSE;	
		if( !wglMakeCurrent( m_pDC->GetSafeHdc(), m_hRC ) ) return FALSE;

		SetDefaultProperties();
		return TRUE;
	}

	void OnDestroy()
	{
		MakeOpenGLCurrent() ;
		wglMakeCurrent(0,0);
		wglDeleteContext( m_hRC );
		if( m_pDC )	delete m_pDC;
	}

	void OnSize(int cx, int cy)
	{ 
		if ( cx <= 0 || cy <= 0) return; m_cx = cx; m_cy = cy;
	}

	//pre rendering
	void OnDraw_Begin()
	{
		m_bIsDrawing = true ;
		MakeOpenGLCurrent() ;

		//viewport
		glViewport(0, 0, m_cx, m_cy);
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		double x_y = m_cx/(double)m_cy, y_x = m_cy/(double)m_cx;
		double r = m_viewSize;
		if( m_cx > m_cy ) glOrtho( -x_y * r, x_y * r, -    r,       r, 0, 500);
		else		      glOrtho(       -r,       r, -y_x*r, y_x * r, 0, 500);
		
		glMatrixMode( GL_MODELVIEW ) ;
		glLoadIdentity(); 
		gluLookAt(  0,0,30,  0,0,0, 0,1,0);//gluLookAt(posX, posY, posZ,  focusX,focusY,focusZ,  updirX,updirY,updirZ);

		glClearColor( m_bkColor[0],m_bkColor[1],m_bkColor[2],m_bkColor[3] ) ;
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT );	
	}

	//post rendering
	void OnDraw_End()
	{
		glFinish();
		SwapBuffers( m_pDC->GetSafeHdc() );
		wglMakeCurrent(NULL,NULL);
		m_bIsDrawing = false ;
	}

	void MakeOpenGLCurrent() const { 
		wglMakeCurrent( m_pDC->GetSafeHdc(), m_hRC );
	}


	bool IsDrawing()const{ return m_bIsDrawing  ; }
	void RedrawWindow(){ if( m_pWnd ) m_pWnd->RedrawWindow() ; }

	//PROJECTION  view port座標(cx,cy)を　2次元空間内の座標(x,y) に変換する 
	inline void GetCursorPos(int cx, int cy, double &x, double &y) const
	{

		if( !m_bIsDrawing ) MakeOpenGLCurrent() ;
		double modelMat[16],projMat[16] ;int vp[4];
		glGetDoublev(GL_MODELVIEW_MATRIX,modelMat) ;
		glGetDoublev(GL_PROJECTION_MATRIX,projMat) ;
		glGetIntegerv(GL_VIEWPORT,vp) ;
		double tmp;
		gluUnProject(cx, vp[3] - cy, 0.0, modelMat, projMat, vp, &x, &y, &tmp) ;		
		if( !m_bIsDrawing ) wglMakeCurrent(NULL,NULL);
	}

private:
	BOOL SetupPixelFormat(){
		static PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),      // size of this pfd
				1,                              // version number
				PFD_DRAW_TO_WINDOW |            // support window
				PFD_SUPPORT_OPENGL |            // support OpenGL
				PFD_DOUBLEBUFFER,               // double buffered
				PFD_TYPE_RGBA,                  // RGBA type
				32,                             // 24-bit color depth
				0, 0, 0, 0, 0, 0,               // color bits ignored
				0,                              // no alpha buffer
				0,                              // shift bit ignored
				0,                              // no accumulation buffer
				0, 0, 0, 0,                     // accum bits ignored
				//        32,                             // 32-bit z-buffer
				16,	// NOTE: better performance with 16-bit z-buffer
				0,                              // no stencil buffer
				0,                              // no auxiliary buffer
				PFD_MAIN_PLANE,                 // main layer
				0,                              // reserved
				0, 0, 0                         // layer masks ignored
		};
		
		int pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd);
		if ( !pixelformat                                            ) return FALSE;
		if ( !SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) ) return FALSE;
		return TRUE;
	}
	
	void SetDefaultProperties()
	{
		glClearColor( m_bkColor[0],m_bkColor[1],m_bkColor[2],m_bkColor[3] ) ;
		glClearDepth( 1.0f );
		glEnable( GL_DEPTH_TEST );
		glDisable( GL_BLEND );
		//glBlendFunc( GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA );
		glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );
		//glBlendFunc( GL_SRC_ALPHA,GL_DST_ALPHA );
		//glBlendFunc( GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA );

		GLfloat lightpos[] = { 1000,1000,-50000,1 };
		GLfloat spec    [] = { 0,0,0.05f,1 } ;
		GLfloat diff    [] = { 0.5f,0.5f,0.5f,1.0f };
		GLfloat amb     [] = { 0.3f,0.3f,0.3f,1.0f };

		GLfloat shininess = 1.5f ;

		glMaterialfv( GL_FRONT, GL_SPECULAR,  spec );
		glMaterialfv( GL_FRONT, GL_DIFFUSE,   diff );
		glMaterialfv( GL_FRONT, GL_AMBIENT,   amb  );
		glMaterialfv( GL_FRONT, GL_SHININESS, &shininess );
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos );

		GLfloat light_Ambient0[] = { 0.2f , 0.2f , 0.2f , 1};
		GLfloat light_Diffuse0[] = { 1,1,1,1};
		GLfloat light_Specular0[]= { 0.2f , 0.2f , 0.2f , 1};
		glLightfv(GL_LIGHT0,GL_AMBIENT,light_Ambient0);
		glLightfv(GL_LIGHT0,GL_DIFFUSE,light_Diffuse0);
		glLightfv(GL_LIGHT0,GL_SPECULAR,light_Specular0);

		GLfloat light_Ambient1[] = { 0,0,0,1};
		GLfloat light_Diffuse1[] = { 0.5f , 0.5f , 0.5f , 1};
		GLfloat light_Specular1[]= { 0,0,0,1};
		glLightfv(GL_LIGHT1,GL_AMBIENT,light_Ambient1);
		glLightfv(GL_LIGHT1,GL_DIFFUSE,light_Diffuse1);
		glLightfv(GL_LIGHT1,GL_SPECULAR,light_Specular1);
		
		GLfloat light_Ambient2[] = { 0,0,0, 1};
		GLfloat light_Diffuse2[] = { 0.5f , 0.5f , 0.5f , 1};
		GLfloat light_Specular2[]= { 0,0,0,1};
		glLightfv(GL_LIGHT2,GL_AMBIENT,light_Ambient2);
		glLightfv(GL_LIGHT2,GL_DIFFUSE,light_Diffuse2);
		glLightfv(GL_LIGHT2,GL_SPECULAR,light_Specular2);
		
		glTexEnvi( GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE ) ;
		
		glShadeModel( GL_SMOOTH ) ;
		glPixelStorei( GL_UNPACK_ALIGNMENT,4 ) ;

		glCullFace( GL_BACK ) ;
		
		glPolygonMode( GL_FRONT,GL_FILL ) ;
		glEnable( GL_CULL_FACE ) ;
	}
};





//TOgl2DImage
class TOGL2DImage
{
public:
	GLubyte* m_RGBA;
	unsigned int m_textureName;
	unsigned int m_width, m_height;
	bool m_DoInterpolation;

public:
	~TOGL2DImage(){clear(0);}
	TOGL2DImage ()
	{
		m_RGBA = 0;
		m_width = m_height = 0;
		m_textureName = -1;
		m_DoInterpolation = true;
	}

	TOGL2DImage(const TOGL2DImage & src)
	{
		m_RGBA            = 0;
		m_width			  = src.m_width ;
		m_height		  = src.m_height;
		m_textureName	  = src.m_textureName;
		m_DoInterpolation = src.m_DoInterpolation;
		if( src.m_RGBA != 0){
			m_RGBA = new GLubyte[ m_width * m_height * 4 ] ;
			memcpy(m_RGBA, src.m_RGBA, sizeof( GLubyte) * m_width * m_height * 4) ;
		}
	}


	void clear(TOGL_2D* ogl){
		unbind( ogl );
		if( m_RGBA != 0 ) delete[] m_RGBA ; 
		m_RGBA = 0;	
		m_width = m_height = 0;
	}

	void unbind(TOGL_2D* ogl){
		if( ogl != 0 && !ogl->IsDrawing() ) ogl->MakeOpenGLCurrent();
		if( m_textureName != -1 && glIsTexture( m_textureName ) ) glDeleteTextures(1,&m_textureName ) ; 
		m_textureName = -1;
		if( ogl != 0 && !ogl->IsDrawing() ) wglMakeCurrent( NULL, NULL);
	}

	void bind(TOGL_2D* ogl)//textureをoglに送る//
	{
		if( ogl != 0 && !ogl->IsDrawing() ) ogl->MakeOpenGLCurrent();
		if( m_textureName == -1 || !glIsTexture( m_textureName  ) )
		{
			glPixelStorei( GL_UNPACK_ALIGNMENT,4);
			glGenTextures( 1,&m_textureName ) ;

			glBindTexture( GL_TEXTURE_2D, m_textureName ) ;
			glTexImage2D( GL_TEXTURE_2D, 0 ,GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RGBA);		

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			if( m_DoInterpolation ) { glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
				                      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;}
			else{                     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				                      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);}
		} else {
			glBindTexture  (GL_TEXTURE_2D, m_textureName) ;
			if( m_DoInterpolation ) { glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
				                      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;}
			else{                     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				                      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);}

		}
		if( ogl != 0 && !ogl->IsDrawing() ) wglMakeCurrent( NULL, NULL);
	}

	inline void flipImageInY()
	{
		byte *tmp = new byte[ m_width * m_height * 4 ];
		for( unsigned int y = 0; y < m_height; ++y)
		for( unsigned int x = 0; x < m_width ; ++x)
		{
			int iold = 4*(       y        * m_width + x) ;
			int inew = 4*( (m_height-1-y) * m_width + x) ;
			memcpy( &tmp[ inew ], &m_RGBA[ iold ], sizeof( byte ) * 4 ); 
		}
		memcpy( m_RGBA, tmp, sizeof( byte ) * m_width* m_height * 4 );
		delete[] tmp;
	}

	//allocation//
	inline bool isAllocated(){ return m_RGBA != 0; }
	void allocateImage( unsigned int width, unsigned int height, TOGL_2D* ogl)
	{
		clear(ogl);
		m_width  = width ;
		m_height = height;
		m_RGBA = new GLubyte[ width * height * 4 ];
		memset( m_RGBA, 0 , sizeof( GLubyte ) * width * height * 4 );
	}
	void allocateImage( const TOGL2DImage &src , TOGL_2D* ogl)
	{
		clear(ogl);
		m_width  = src.m_width ;
		m_height = src.m_height;
		m_RGBA = new GLubyte[ m_width * m_height * 4 ];
		memcpy( m_RGBA, src.m_RGBA, sizeof( GLubyte ) * m_width * m_height * 4 );
	}



	bool allocateFromFile( const char *fname, bool &inverted, TOGL_2D* ogl)
	{
		CImage pic;
		if( !SUCCEEDED( pic.Load( _T(fname) ) ) ) return false;
	
		//メージのピッチ(行方向のバイト数)を返します。
		//戻り値が負の値の場合、ビットマップは左下隅を起点とする逆方向 (下から上) の DIB です。
		//戻り値が正の値の場合、ビットマップは左上隅を起点とする順方向 (上から下の向き) の DIB です
		int pitch   = pic.GetPitch();
		if( pitch < 0) { inverted = true ; pitch *= -1; }
		else             inverted = false;
		int W       = pic.GetWidth ();
		int H       = pic.GetHeight();
		int byteNum = pitch / W;

		allocateImage( W, H, ogl);

		if( byteNum == 1 )
		{
			for( int y=0, i=0; y<H; ++y     )
			for( int x=0     ; x<W; ++x, ++i)
			{
				//RGB(24bit)に対するアドレスをbyteにキャストしてるので, R G B順の一番下位のBを指すものになる
				byte *c = (byte*)pic.GetPixelAddress( x, y );
				m_RGBA[ i*4 + 3] = 128;
				m_RGBA[ i*4 + 2] = *c ; 
				m_RGBA[ i*4 + 1] = *c ; 
				m_RGBA[ i*4 + 0] = *c ; 
			}
		}
		else if( byteNum == 3 ) //24bit color 
		{
			for( int y=0, i=0; y<H; ++y     )
			for( int x=0     ; x<W; ++x, ++i)
			{
				//RGB(24bit)に対するアドレスをbyteにキャストしてるので, R G B順の一番下位のBを指すものになる
				byte *c = (byte*)pic.GetPixelAddress( x, y );
				m_RGBA[ i*4 + 3] = 128;
				m_RGBA[ i*4 + 2] = *c ; ++c;
				m_RGBA[ i*4 + 1] = *c ; ++c;
				m_RGBA[ i*4 + 0] = *c ; 
			}
		}
		else //その他(ピクセルごとにビデオメモリをポーリングする遅い実装)
		{
			for( int y=0, i=0; y<H; ++y     )
			for( int x=0     ; x<W; ++x, ++i)
			{
				COLORREF c = pic.GetPixel( x, y );
				m_RGBA[ i*4 + 3] = 128;
				m_RGBA[ i*4 + 0] = GetRValue(c);
				m_RGBA[ i*4 + 1] = GetGValue(c);
				m_RGBA[ i*4 + 2] = GetBValue(c);
			}
		}	
		return true;
	}

	bool saveAsFile      ( const char *fname, int flg_BmpJpgPngTiff = 0 )
	{
		CImage pic;
		pic.Create(m_width, m_height, 24, 0 );
		for( unsigned int y = 0, i=0; y < m_height; ++y     )
		for( unsigned int x = 0     ; x < m_width ; ++x, ++i)
		{
			byte *c = (byte*)pic.GetPixelAddress( x, y );
			*c = m_RGBA[ i*4 + 2]; ++c;
			*c = m_RGBA[ i*4 + 1]; ++c;
			*c = m_RGBA[ i*4 + 0]; 
		}
		if(      flg_BmpJpgPngTiff == 0 )  pic.Save( _T( fname ), Gdiplus::ImageFormatBMP  );
		else if( flg_BmpJpgPngTiff == 1 )  pic.Save( _T( fname ), Gdiplus::ImageFormatJPEG );
		else if( flg_BmpJpgPngTiff == 2 )  pic.Save( _T( fname ), Gdiplus::ImageFormatPNG  );
		else if( flg_BmpJpgPngTiff == 3 )  pic.Save( _T( fname ), Gdiplus::ImageFormatTIFF );	
		return true;
	}
	
	inline void setColor( int idx, byte r, byte g, byte b){
		m_RGBA[idx] = r;
		m_RGBA[idx+1] = g;
		m_RGBA[idx+2] = b;
	}
	inline void setColor( int idx, const TVector3 &color){
		m_RGBA[idx  ] = (byte)color.data[0];
		m_RGBA[idx+1] = (byte)color.data[1];
		m_RGBA[idx+2] = (byte)color.data[2];
	}

};





#endif	// __TOGL_H_INCLUDED__