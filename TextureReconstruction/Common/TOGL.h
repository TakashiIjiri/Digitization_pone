#ifndef __TOGL_H_INCLUDED__
#define __TOGL_H_INCLUDED__

// -----------------------------------------------------------------------------
// Copyright(c) 2016, Takashi Ijiri
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// *Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
// * Neither the name of the <organization> nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// 
// TOGL provides OpenGL functions for MFC.
// 
// Start development at 2016/3/4
// This class uses Eigen (MPL2) without modification, OpenMesh (BSD), and glew (BSD)
// OpenMesh http://www.openmesh.org/license/
// Glew     http://glew.sourceforge.net/credits.html
// ------------------------------------------------------------------------------

// use glew 
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include "tmath.h"

#pragma warning(disable:4996)


#include "glew/include/GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>

//glew32s.libファイルの場所をプロパティより指定 (フォルダがx64とw86で違うので注意)
#pragma comment(lib, "glew32s.lib" )
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib"   )




class TOGLCamera
{
public:
	EVec3d m_Pos, m_Piv, m_Up;

	TOGLCamera(){}

	TOGLCamera( const TOGLCamera& src ){
		Set( src.m_Pos, src.m_Piv, src.m_Up);
	}

	inline void Set( const EVec3d &pos, const  EVec3d &piv, const EVec3d &up){
		m_Pos  = pos;
		m_Piv  = piv;
		m_Up   = up ;
	}	

	inline void callGluLookAt(){
		gluLookAt(  m_Pos[0], m_Pos[1], m_Pos[2],  m_Piv[0], m_Piv[1], m_Piv[2],  m_Up [0], m_Up [1], m_Up [2]);
	}
};





class TOGL
{
	enum {
		T_BTN_NON    , 
		T_BTN_ZOOM   , 
		T_BTN_ROTATE , 
		T_BTN_ROTYDIR, 
		T_BTN_TRANS  , T_BTN_ORTHO_ZOOM, T_BTN_ORTHO_TRANS,
	} m_btnState;

	CWnd*      m_pWnd    ;//trgt MFC view 
	CDC*       m_pDC     ;//device context of trgtMFC view
	HGLRC      m_hRC     ;//open gl handle
	int        m_cx, m_cy; //window size

	TOGLCamera m_camera  ;

	bool       m_bDrawing; // flg : true when rendering 
	CPoint     m_PrePos  ;

	double     m_orthoViewSize;
	float      m_clearColor[4];

public:

	TOGL()
	{
		m_pWnd			= 0;
		m_pDC			= 0;
		m_btnState		= T_BTN_NON;
		m_bDrawing	    = false;
		m_orthoViewSize = 10   ;
		m_clearColor[0] = m_clearColor[1] = m_clearColor[2] = 1.0f; m_clearColor[3] = 0.5f ;
		m_camera.Set(EVec3d( 0, 0, 10 ), EVec3d( 0, 0, 0  ), EVec3d( 0,  1, 0  ) );
	}

	inline void setOrthoModeViewSize( double s){ m_orthoViewSize = s;}

	
	virtual BOOL OnCreate(CWnd* pWnd)
	{
		m_pWnd = pWnd ;
		m_pDC  = new CClientDC(pWnd);
		if( !m_pDC              )	return FALSE;
		if( !SetupPixelFormat() )	return FALSE;
		
		m_hRC = wglCreateContext( m_pDC->GetSafeHdc() );
		if( !m_hRC                                        )	return FALSE;	
		if( !wglMakeCurrent( m_pDC->GetSafeHdc(), m_hRC ) ) return FALSE;

		SetDefaultProperties();
		glewInit();//OpenGl extension
		return TRUE;
	}

	virtual void OnDestroy()
	{
		MakeOpenGLCurrent() ;
		wglMakeCurrent(0,0);
		wglDeleteContext( m_hRC );
		if( m_pDC )	delete m_pDC;
	}

	virtual void OnSize(int cx, int cy)
	{ 
		if ( cx <= 0 || cy <= 0) return; 
		m_cx = cx; 
		m_cy = cy;
	}

	virtual void MakeOpenGLCurrent() const 
	{ 
		wglMakeCurrent( m_pDC->GetSafeHdc(), m_hRC );
	}

	//PRE POST RENDERING -- 
	void OnDrawBegin( double fovY = 45, double view_near=0.02, double view_far = 300, bool bOrtho = false)
	{
		if( m_bDrawing ) return;
		m_bDrawing = true ;
		MakeOpenGLCurrent() ;

		//glDrawBuffer(GL_BACK);

		glViewport(0, 0, m_cx, m_cy);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		if (bOrtho)
		{
			double x_y = m_cx / (double)m_cy, y_x = m_cy / (double)m_cx;
			double r = m_orthoViewSize;
			if (m_cx > m_cy) glOrtho(-x_y*r, x_y*r, -r, r, view_near, view_far);
			else		     glOrtho(-r, r, -y_x*r, y_x*r, view_near, view_far); 
		}
		else
		{
			gluPerspective(fovY, m_cx / (double)m_cy, view_near, view_far);
		}

		glMatrixMode( GL_MODELVIEW ) ;
		glLoadIdentity();
		m_camera.callGluLookAt();
		
		glClearColor( m_clearColor[0],m_clearColor[1],m_clearColor[2],m_clearColor[3] ) ;
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT );
	}


	void OnDrawEnd()
	{
		glFinish();
		SwapBuffers( m_pDC->GetSafeHdc() );
		wglMakeCurrent(NULL,NULL);
		m_bDrawing = false ;
	}



	inline bool   IsDrawing() const { return m_bDrawing    ; }
	inline EVec3d GetCamPos() const { return m_camera.m_Pos; }
	inline EVec3d GetCamPiv() const { return m_camera.m_Piv; }
	inline EVec3d GetCamUp () const { return m_camera.m_Up ; }
	inline void   SetCam(const EVec3d &pos, const EVec3d &piv, const EVec3d &up){ m_camera.Set( pos, piv, up);}


	//mouse for camera manipuration 
private:
	inline void SetMouseCapture     ()const{ m_pWnd->SetCapture(); }
	inline void ReleaseMouseCapture ()const{ ReleaseCapture    (); }

	inline void eyeRotation(double theta,double phai)
	{
		EVec3d &eyeP = m_camera.m_Pos;
		EVec3d &eyeF = m_camera.m_Piv;
		EVec3d &eyeU = m_camera.m_Up ;
		EVec3d axisP = ((eyeF - eyeP).cross( eyeU ) ).normalized();

		Eigen::AngleAxisd rotTheta = Eigen::AngleAxisd(theta, eyeU );
		Eigen::AngleAxisd rotPhai = Eigen::AngleAxisd(phai, axisP);
		eyeU = rotPhai * rotTheta * eyeU;
		eyeP = rotPhai * rotTheta * (eyeP - eyeF) + eyeF;
	}

public:


	void ButtonDownForZoom      (const CPoint& pos){ m_PrePos=pos; m_btnState = T_BTN_ZOOM       ; m_pWnd->SetCapture();}
	void ButtonDownForRotate    (const CPoint& pos){ m_PrePos=pos; m_btnState = T_BTN_ROTATE     ; m_pWnd->SetCapture();}
	void ButtonDownForYdirRot   (const CPoint& pos){ m_PrePos=pos; m_btnState = T_BTN_ROTYDIR    ; m_pWnd->SetCapture();}
	void ButtonDownForTranslate (const CPoint& pos){ m_PrePos=pos; m_btnState = T_BTN_TRANS      ; m_pWnd->SetCapture();}
	void ButtonDownForOrthoZoom (const CPoint& pos){ m_PrePos=pos; m_btnState = T_BTN_ORTHO_ZOOM ; m_pWnd->SetCapture();}
	void ButtonDownForOrthoTrans(const CPoint& pos){ m_PrePos=pos; m_btnState = T_BTN_ORTHO_TRANS; m_pWnd->SetCapture();}
	void ButtonDownForTransYRotZoom(const CPoint& pos, int wOffset= 30, int hOffset = 30){ 
		m_PrePos=pos; 
		m_pWnd->SetCapture();
		m_btnState = ( pos.x < wOffset || m_cx - wOffset < pos.x ) ? T_BTN_ZOOM   : 
		             ( pos.y < hOffset || m_cy - hOffset < pos.y ) ? T_BTN_ROTYDIR: T_BTN_TRANS;
	}
	void ButtonDownForRotYRotZoom(const CPoint& pos, int wOffset= 30, int hOffset = 30){ 
		m_PrePos=pos; 
		m_pWnd->SetCapture();
		m_btnState = ( pos.x < wOffset || m_cx  - wOffset < pos.x ) ? T_BTN_ZOOM    : 
			         ( pos.y < hOffset || m_cy - hOffset < pos.y ) ? T_BTN_ROTYDIR : T_BTN_ROTATE;
	}

	void ButtonDownForOrthoZoomYRotZoom(const CPoint& pos, int wOffset= 30, int hOffset = 30){ 
		m_PrePos=pos; 
		m_pWnd->SetCapture();
		m_btnState = ( pos.x < wOffset || m_cx - wOffset < pos.x ) ? T_BTN_ORTHO_ZOOM : 
			         ( pos.y < hOffset || m_cy - hOffset < pos.y ) ? T_BTN_ROTYDIR    : T_BTN_ORTHO_ZOOM;
	}
	void ButtonDownForOrthoTransYRotZoom(const CPoint& pos, int wOffset= 30, int hOffset = 30){ 
		m_PrePos=pos; 
		m_pWnd->SetCapture();
		m_btnState = ( pos.x < wOffset || m_cx - wOffset < pos.x ) ? T_BTN_ORTHO_ZOOM : 
			         ( pos.y < hOffset || m_cy - hOffset < pos.y ) ? T_BTN_ROTYDIR    : T_BTN_ORTHO_TRANS;
	}

	void ButtonUp(){
		m_btnState = T_BTN_NON;	
		if( m_pWnd == m_pWnd->GetCapture()) ReleaseCapture();
	}

	bool MouseMove(const CPoint& pos)
	{
		if( m_btnState == T_BTN_NON || m_pWnd != m_pWnd->GetCapture()) return false;
		
		EVec3d &eyeP = m_camera.m_Pos;
		EVec3d &eyeF = m_camera.m_Piv;
		EVec3d &eyeU = m_camera.m_Up ;
		double dX = pos.x - m_PrePos.x;
		double dY = pos.y - m_PrePos.y;

		if( m_btnState == T_BTN_ROTATE)
		{			
			eyeRotation( -dX / 200.0, -dY / 200.0 );
		}
		else if( m_btnState == T_BTN_ZOOM)
		{				
			EVec3d newEyeP = eyeP + dY / 80.0 * (eyeF - eyeP);
			if( (newEyeP - eyeF).norm() > 0.02) eyeP = newEyeP;
		}
		else if( m_btnState == T_BTN_ROTYDIR )
		{
			eyeU = Eigen::AngleAxisd(dX * 0.005, (eyeF - eyeP).normalized()) * eyeU;
		}
		else if(m_btnState == T_BTN_TRANS)
		{
			double c = (eyeP- eyeF).norm() / 900.0;
			EVec3d Xdir = ((eyeP - eyeF).cross( eyeU ) ).normalized();
			EVec3d t    = c * dX * Xdir  +  c * dY * eyeU;
			eyeP += t;
			eyeF += t;
		}
		else if(m_btnState == T_BTN_ORTHO_TRANS)
		{
			double pixelWidth = (m_cx==0 && m_cy==0) ? 0.01 : 
				                ( m_cx > m_cy      ) ? 2 * m_orthoViewSize / m_cy : 
								                       2 * m_orthoViewSize / m_cx ;
			EVec3d Xdir = ( (eyeP - eyeF).cross( eyeU )  ).normalized();
			EVec3d t    = dX * pixelWidth * Xdir  +  dY * pixelWidth * eyeU ;
			eyeP += t;
			eyeF += t;
		}
		else if(m_btnState == T_BTN_ORTHO_ZOOM) 
		{ 
			m_orthoViewSize *= (1 - dY * 0.001); 
			if( m_orthoViewSize < 0.01 ) m_orthoViewSize = 0.01;
		}
		else{}
		m_PrePos = pos;
		return TRUE ;
	}


	//Projection --
	inline void GetCursorRay(int cx, int cy, EVec3d &eyePos, EVec3d &rayDir) const
	{
		if( !m_bDrawing ) MakeOpenGLCurrent() ;
		double modelMat[16], projMat[16] ;int vp[4];
		glGetDoublev (GL_MODELVIEW_MATRIX , modelMat) ;
		glGetDoublev (GL_PROJECTION_MATRIX, projMat) ;
		glGetIntegerv(GL_VIEWPORT,vp) ;
		
		gluUnProject(cx, vp[3] - cy,   0, modelMat, projMat, vp, &eyePos[0], &eyePos[1], &eyePos[2]) ;
		gluUnProject(cx, vp[3] - cy, 0.2, modelMat, projMat, vp, &rayDir[0], &rayDir[1], &rayDir[2]) ;
		rayDir -= eyePos ; 
		rayDir.normalize();
		if( !m_bDrawing ) wglMakeCurrent(NULL,NULL);
	}

	inline void GetCursorRay( const CPoint &point, EVec3d &rayPos, EVec3d &rayDir) const{
		GetCursorRay( point.x, point.y, rayPos, rayDir);
	}

	void unProject_correctY( double cx, double cy, float depth, EVec3d &pos)
	{
		if( !m_bDrawing ) MakeOpenGLCurrent() ;
		double modelMat[16],projMat[16] ;int vp[4];
		glGetDoublev(GL_MODELVIEW_MATRIX,modelMat) ;
		glGetDoublev(GL_PROJECTION_MATRIX,projMat) ;
		glGetIntegerv(GL_VIEWPORT,vp) ;

		gluUnProject(cx, vp[3] - cy, depth, modelMat, projMat, vp, &pos[0], &pos[1], &pos[2]) ;
		
		if( !m_bDrawing ) wglMakeCurrent(NULL,NULL);
	}

	void Project( double  objx,double  objy,double  objz, double& winx,double& winy,double& winz ) const
	{
		if( !m_bDrawing ) MakeOpenGLCurrent();
		double model[16],proj[16] ;int vp[4] ;
		glGetDoublev(GL_MODELVIEW_MATRIX,model) ;
		glGetDoublev(GL_PROJECTION_MATRIX,proj) ;
		glGetIntegerv(GL_VIEWPORT, vp) ;

		gluProject( objx,objy,objz,model,proj,vp,&winx,&winy,&winz ) ;
		if( !m_bDrawing ) wglMakeCurrent(NULL,NULL);
	}

	//MISCS
	inline void GetClearColor(EVec3d &color ){
		color = EVec3d( m_clearColor[0], m_clearColor[1], m_clearColor[2] );
	}
	void SetClearColor( double r, double g, double b, double a){
		m_clearColor[0] = (float)r;
		m_clearColor[1] = (float)g;
		m_clearColor[2] = (float)b;
		m_clearColor[3] = (float)a;
	}


	CDC* GetDC(){	 return m_pDC; }
	int GetWidth (){ return m_cx ; }
	int GetHeight(){ return m_cy ; }

	double GetOrthoViewSize(){ return m_orthoViewSize; }
	void   SetOrthoViewSize(double r){ m_orthoViewSize = r;}

	void RedrawWindow(){ if( m_pWnd ) m_pWnd->RedrawWindow() ; }




	inline double getDistEyeToScreen()
	{
		if( !m_bDrawing ) MakeOpenGLCurrent() ;
		double length = 0;
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix  ();
		glLoadIdentity();
		{
			double modelMat[16],projMat[16];
			int vp[4];
			glGetDoublev(GL_MODELVIEW_MATRIX,modelMat) ;
			glGetDoublev(GL_PROJECTION_MATRIX,projMat) ;
			glGetIntegerv(GL_VIEWPORT,vp) ;
			double x,y,z;
			gluUnProject(vp[0]+vp[2]/2, vp[1]+vp[3]/2, 0.0001, modelMat, projMat, vp, &x,&y,&z);
			length = fabs( z );
		}
		glPopMatrix();

		if( !m_bDrawing ) wglMakeCurrent(NULL,NULL);
		return length;
	}
	


private:
	BOOL SetupPixelFormat(){
		static PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
				1,                              // version number
				PFD_DRAW_TO_WINDOW |            // support window
				PFD_SUPPORT_OPENGL |          // support OpenGL
				PFD_DOUBLEBUFFER,             // double buffered
				PFD_TYPE_RGBA,                  // RGBA type
				32,                             // 24-bit color depth
				0, 0, 0, 0, 0, 0,               // color bits ignored
				0,                              // no alpha buffer
				0,                              // shift bit ignored
				0,                              // no accumulation buffer
				0, 0, 0, 0,                     // accum bits ignored
				32,                             // 32-bit z-buffer
				//16,	// NOTE: better performance with 16-bit z-buffer
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
		glClearColor( m_clearColor[0],m_clearColor[1],m_clearColor[2],m_clearColor[3] ) ;
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
		//glLightfv(GL_LIGHT0, GL_POSITION, lightpos );

		GLfloat light_Ambient0[] = { 0.15f , 0.15f , 0.15f , 1};
		GLfloat light_Diffuse0[] = { 1,1,1,1};
		GLfloat light_Specular0[]= { 0.3f , 0.3f , 0.3f , 1};
		glLightfv(GL_LIGHT0,GL_AMBIENT,light_Ambient0);
		glLightfv(GL_LIGHT0,GL_DIFFUSE,light_Diffuse0);
		glLightfv(GL_LIGHT0,GL_SPECULAR,light_Specular0);

		GLfloat light_Ambient1[] = { 0,0,0,1};
		GLfloat light_Diffuse1[] = { 0.4f , 0.4f , 0.4f , 1};
		GLfloat light_Specular1[]= { 0.1f,0.1f,0.1f,1};
		glLightfv(GL_LIGHT1,GL_AMBIENT,light_Ambient1);
		glLightfv(GL_LIGHT1,GL_DIFFUSE,light_Diffuse1);
		glLightfv(GL_LIGHT1,GL_SPECULAR,light_Specular1);
		
		GLfloat light_Ambient2[] = { 0,0,0, 1};
		GLfloat light_Diffuse2[] = { 0.3f , 0.3f , 0.3f , 1};
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
		
		glEnable( GL_LIGHT0 );
		glEnable( GL_LIGHT1 );
		glEnable( GL_LIGHT2 );
		glEnable( GL_LIGHTING ) ;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glEnable( GL_TEXTURE_2D );
	}

};








#endif	// __TOGL_H_INCLUDED__