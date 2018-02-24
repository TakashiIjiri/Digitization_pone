
// Copyright (c) 2017, Takashi Ijiri, Tomofumi Narita, Chika Tomiyama
// All rights reserved.
//
//
// This file is released under GNU GPLv3.
//
//
// This class uses Eigen 3.2.8 released under MPL2 without any modification.
// see http://eigen.tuxfamily.org/index.php?title=Main_Page#License
// 
// This class uses Glew released under the Modified BSD
// see http://glew.sourceforge.net/credits.html
// see http://glew.sourceforge.net/glew.txt
//
// Start development at 2017/7/7




#pragma once
#pragma unmanaged


#include <windows.h> 

#include "gl/glew.h"
#include <gl/gl.h> 
#include <gl/glu.h> 
#include "tmath.h"


class OglForCLI
{
	
private:
	HDC   m_hdc;
	HGLRC m_hglrc;
	
	// Camera position/center/Up direction 
	EVec3f m_camP, m_camC, m_camU;

	// View Size
	bool   m_bDrawing;
	EVec2i m_mousePt;
	EVec4f m_bgColor;


	enum {
		BTN_NON, BTN_TRANS, BTN_ZOOM, BTN_ROT
	} m_btnState;


public:

	
	~OglForCLI() {}


	OglForCLI(HDC dc)
	{
		if (dc == 0) return;

		m_camP = EVec3f(0, 0, 10);
		m_camC = EVec3f(0, 0, 0);
		m_camU = EVec3f(0, 1, 0);

		m_bgColor = EVec4f(0, 0, 0, 0.5);
	
		m_bDrawing = false;

		SetDefaultProperties();


		m_hdc = dc;
		static PIXELFORMATDESCRIPTOR pfd = 
			{
				sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
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
				32,                             // 32-bit z-buffer (or 16, 24 bit)
				0,                              // stencil buffer (no)
				0,                              // auxiliary buffer (no)
				PFD_MAIN_PLANE,                 // main layer
				0,                              // reserved
				0, 0, 0                         // layer masks ignored
			};


		int pfmt = ChoosePixelFormat(m_hdc, &pfd);
		if (pfmt == 0) return;
		if( !SetPixelFormat(m_hdc, pfmt, &pfd) ) return;

		// pure Managed だとランタイムでエラーに 
		if ((m_hglrc = wglCreateContext(m_hdc)) == 0) return; 
		if ((wglMakeCurrent(m_hdc, m_hglrc))    == 0) return;


		
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}


		wglMakeCurrent(0, 0);

		return;
	}

	
	void oglMakeCurrent() const
	{
		wglMakeCurrent( m_hdc, m_hglrc);
	}

	void getProjModelViewMats(
		double *model, double *proj, int *vp,
		int viewW, int viewH, double fovY = 45.0, 
		double view_near = 0.02, double view_far = 700.0)
	{	
		if( !m_bDrawing ) oglMakeCurrent();

		glViewport(0, 0, viewW, viewH);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fovY, viewW / (double)viewH, view_near, view_far);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		gluLookAt(m_camP[0], m_camP[1], m_camP[2], 
			      m_camC[0], m_camC[1], m_camC[2],
			      m_camU[0], m_camU[1], m_camU[2]);

		glGetDoublev (GL_MODELVIEW_MATRIX , model );
		glGetDoublev (GL_PROJECTION_MATRIX, proj  );
		glGetIntegerv(GL_VIEWPORT, vp);


		if( !m_bDrawing ) wglMakeCurrent(NULL, NULL);
	}
	




	void OnDrawBegin( int viewW, int viewH, double fovY = 45.0, double view_near = 0.02, double view_far = 700.0 )
	{
		if( m_bDrawing ) return;

		m_bDrawing = true;
		oglMakeCurrent();

		glViewport(0, 0, viewW, viewH);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fovY, viewW / (double)viewH, view_near, view_far);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		gluLookAt(m_camP[0], m_camP[1], m_camP[2], 
			      m_camC[0], m_camC[1], m_camC[2],
			      m_camU[0], m_camU[1], m_camU[2]);
		glClearColor( (float)m_bgColor[0], (float)m_bgColor[1], (float)m_bgColor[2], (float)m_bgColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	}
	
	void OnDrawEnd()
	{
		glFinish();
		SwapBuffers( m_hdc );
		wglMakeCurrent(NULL, NULL);
		m_bDrawing = false;
	}
	







	inline bool   isDrawing() const { return m_bDrawing; }
	inline EVec3f GetCamPos() const { return m_camP; }
	inline EVec3f GetCamCnt() const { return m_camC; }
	inline EVec3f GetCamUp () const { return m_camU; }
	inline void   SetCam(const EVec3f &pos, const EVec3f &cnt, const EVec3f &up) { m_camP = pos; m_camC = cnt; m_camU = up; }
	inline void   SetBgColor(EVec4f bg) { m_bgColor = bg; }
	inline void   SetBgColor(float r, float g, float b, float a) { m_bgColor << r,g,b,a; }


	//Mouse Listener for Camera manipuration
	void BtnDown_Trans(const EVec2i &p) 
	{ 
		m_mousePt = p; 
		m_btnState   = BTN_TRANS; 
	}
	void BtnDown_Zoom(const EVec2i &p)
	{
		m_mousePt = p; 
		m_btnState = BTN_ZOOM;
	}
	void BtnDown_Rot(const EVec2i &p)
	{
		m_mousePt = p; 
		m_btnState = BTN_ROT;
	}

	void BtnUp() 
	{
		m_btnState = BTN_NON;
		ReleaseCapture();
	}


	
	void MouseMove(const EVec2i &p)
	{
		if (m_btnState == BTN_NON ) return;

		float dX = (float) (p[0] - m_mousePt[0]);
		float dY = (float) (p[1] - m_mousePt[1]);

		if (m_btnState == BTN_ROT)
		{
			float theta = -dX / 200.0f;
			float phi   = -dY / 200.0f;

			EVec3f axis = ((m_camC - m_camP).cross(m_camU)).normalized();
			Eigen::AngleAxisf rotTheta = Eigen::AngleAxisf(theta, m_camU);
			Eigen::AngleAxisf rotPhi   = Eigen::AngleAxisf(phi  , axis  );
			m_camU = rotPhi * rotTheta * m_camU;
			m_camP = rotPhi * rotTheta * (m_camP - m_camC) + m_camC;
		}
		else if (m_btnState == BTN_ZOOM)
		{
			EVec3f newEyeP = m_camP + dY / 80.0f * (m_camC - m_camP);
			if( (newEyeP - m_camC).norm() > 0.02f) m_camP = newEyeP;
		}
		else if (m_btnState == BTN_TRANS)
		{
			float c = (m_camP - m_camC).norm() / 900.0f;
			EVec3f Xdir = ((m_camP - m_camC).cross(m_camU) ).normalized();
			EVec3f t = c * dX * Xdir + c * dY * m_camU;
			m_camP += t;
			m_camC += t;
		}
		m_mousePt = p;
	}

	/*
	
	void OnDestroy()
	{
		oglMakeCurrent();
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hRC);
		if (m_pDC) delete m_pDC;
	}
*/
	void ZoomCam(float distance)
	{
		EVec3f rayD = (m_camC - m_camP);
		float  len  = rayD.norm();

		if( distance > len ) return;
		rayD /= len;
		m_camP = m_camP + distance * rayD;
	}


	void unProject_correctY( const double cx, const double cy, const double depth, double &x, double &y, double &z) const 
	{
		if (!m_bDrawing) oglMakeCurrent();
		double modelMat[16], projMat[16]; 
		int vp[4];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelMat);
		glGetDoublev(GL_PROJECTION_MATRIX, projMat);
		glGetIntegerv(GL_VIEWPORT, vp);

		gluUnProject(cx, vp[3] - cy, depth, modelMat, projMat, vp, &x,&y,&z);

		if (!m_bDrawing) wglMakeCurrent(NULL, NULL);
	}

	void Project( const double   inX, const double   inY, const double   inZ, 
				   double &outX,  double &outY,  double &outZ) const
	{
		if ( !m_bDrawing ) oglMakeCurrent();
		double model[16], proj[16]; 
		int vp[4];
		glGetDoublev(GL_MODELVIEW_MATRIX, model);
		glGetDoublev(GL_PROJECTION_MATRIX, proj);
		glGetIntegerv(GL_VIEWPORT, vp);

		gluProject(inX, inY, inZ, model, proj, vp, &outX, &outY, &outZ);
		if ( !m_bDrawing ) wglMakeCurrent(NULL, NULL);
	}

	inline void GetCursorRay(int cx, int cy, EVec3f &rayPos, EVec3f &rayDir) const
	{
		double x1,y1,z1, x2,y2,z2;
		unProject_correctY(cx, cy, 0.01, x1,y1,z1);
		unProject_correctY(cx, cy, 0.2 , x2,y2,z2);

		rayPos << (float)   x1  , (float)   y1  , (float)  z1   ;
		rayDir << (float)(x2-x1), (float)(y2-y1), (float)(z2-z1);
		rayDir.normalize();
	}

	inline void GetCursorRay(const EVec2i &pt, EVec3f &rayPos, EVec3f &rayDir) const
	{
		GetCursorRay(pt[0], pt[1], rayPos, rayDir);
	}

private:



	void SetDefaultProperties()
	{
		glClearDepth(1.0f);

		//Material 
		float   shin[1] = { 64 };
		EVec4f  spec(1, 1, 1, 0.5), diff(0.5f, 0.5f, 0.5f, 0.5f), ambi(0.5f, 0.5f, 0.5f, 0.5f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , spec.data());
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE  , diff.data());
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT  , ambi.data());
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shin);

		//lights
		glEnable(GL_LIGHTING);
		EVec4f lPosi[3] = { EVec4f(1000,1000,-1000,1), EVec4f(-1000,1000,-1000,1), EVec4f(1000,-1000,-1000,1) };
		EVec4f lambi[3] = { EVec4f(1.0f,1.0f,1.0f,1), EVec4f(0,0,0,0)            , EVec4f(0,0,0,0) };
		EVec4f ldiff[3] = { EVec4f(1.0f,1.0f,1.0f,1), EVec4f(0.5f,0.5f,0.5f,1)   , EVec4f(0.5f,0.5f,0.5f,1)   };
		EVec4f lspec[3] = { EVec4f(0.3f,0.3f,0.3f,1), EVec4f(0.3f,0.3f,0.3f,1)   , EVec4f(0.3f,0.3f,0.3f,1) };

		for (int i = 0; i < 3; ++i)
		{
			GLenum light = (i == 0) ? GL_LIGHT0 : (i == 1) ? GL_LIGHT1 : GL_LIGHT2;
			glLightfv(light, GL_POSITION, lPosi[i].data());
			glLightfv(light, GL_AMBIENT , lambi[i].data());
			glLightfv(light, GL_DIFFUSE , ldiff[i].data());
			glLightfv(light, GL_SPECULAR, lspec[i].data());
			glEnable(light);
		}

		// other general states
		glEnable(GL_DEPTH_TEST);
		
		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glShadeModel ( GL_SMOOTH);
		glPixelStorei( GL_UNPACK_ALIGNMENT, 4);

		glPolygonMode(GL_FRONT, GL_FILL);

		glEnable  (GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

};

#pragma managed
