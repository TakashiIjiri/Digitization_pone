#pragma once 

#include "./TOGL.h"
#include "./timage.h"
#include "./ttexmesh.h"



// rendering 
#define  REND_X_NUM 4
#define  REND_Y_NUM 3
#define  REND_NUM   12

// parameters only for photo by D7000 (3696 x 2448)
#define   CAMERA_SENSOR_W 23.6  
#define   CAMERA_SENSOR_H 15.6 
#define   CAMERA_RES_W    462  
#define   CAMERA_RES_H    306  
#define   H_ANGLE  0.0001
#define   H_RADI  10.0
#define   H_ROT    0.0000001
#define   OBJ_SCALLING    1.0
const double FINSEARCH_OFST_ANGL = 0.1 * (2 * M_PI / 360.0); //[radian]
const double FINSEARCH_OFST_RADI = 10                      ; //[mm]
const double FINSEARCH_OFST_ROT  = 0.1 * (2 * M_PI / 360.0); //[radian]

/*

// parameters only for photo by EOS-1Ds Mark III (5616 x 3744)
#define   CAMERA_SENSOR_W 36  
#define   CAMERA_SENSOR_H 24 
#define   CAMERA_RES_W    702  
#define   CAMERA_RES_H    468  
#define   H_ANGLE         0.0001
#define   H_RADI          0.1
#define   H_ROT           0.0000001
const double FINSEARCH_OFST_ANGL = 0.02 * (2 * M_PI / 360.0); //[radian]
const double FINSEARCH_OFST_RADI = 0.02                    ; //[mm]
const double FINSEARCH_OFST_ROT  = 0.02 * (2 * M_PI / 360.0); //[radian]
*/



//texture generation
#define GEN_TEXTURE_SIZE 5000
const double VISIBLE_TRIANGLE_THRESH = M_PI / 2; //[radian]





class CameraParam
{

public:
	//position 
	double m_tx, m_ty, m_tz, m_theta, m_phi, m_radius;

	//focal length
	double m_fLen;

	void Set( double theta, double phi, double R, double Tx, double Ty, double Tz, double fLen)
	{
		m_theta  = theta;
		m_phi    = phi  ;
		m_radius = R    ;
		m_tx     = Tx   ;
		m_ty     = Ty   ;
		m_tz     = Tz   ;
		m_fLen   = fLen ;
	}


	CameraParam(double theta= 0, double phi= 0, double radius= 500, 
		        double tx   = 0, double ty = 0, double tz    = 0  , double fLength = 50)
	{
		Set( theta, phi, radius, tx, ty, tz, fLength);
	}

	CameraParam(const CameraParam  &v){
		Set( v.m_theta, v.m_phi, v.m_radius, v.m_tx, v.m_ty, v.m_tz, v.m_fLen);
	}

	CameraParam& operator= (const CameraParam &v){
		Set( v.m_theta, v.m_phi, v.m_radius, v.m_tx, v.m_ty, v.m_tz, v.m_fLen);
		return *this ; 
	}

	void Trace(){ fprintf(stderr, "%f %f %f %f %f %f %f\n", m_theta, m_phi, m_radius, m_tx, m_ty, m_tz, m_fLen); }


	void getCamearaPos(EVec3d &pos, EVec3d &focus, EVec3d &yDir, double distToFocusPos = 500) const
	{
		Eigen::AngleAxis<double> Rt(m_theta, EVec3d(0, 1, 0));
		Eigen::AngleAxis<double> Rp(m_phi  , EVec3d(0, 0, 1));

		Eigen::AngleAxis<double> Rx(m_tx   , EVec3d(1, 0, 0));
		Eigen::AngleAxis<double> Ry(m_ty   , EVec3d(0, 1, 0));
		Eigen::AngleAxis<double> Rz(m_tz   , EVec3d(0, 0, 1));

		EVec3d ray;
		pos  = Rt * Rp * EVec3d(m_radius, 0, 0);
		ray  = Rt * Rp * Rx * Ry * Rz * EVec3d(-1, 0, 0);
		yDir = Rt * Rp * Rx * Ry * Rz * EVec3d( 0, 1, 0);

		focus = pos + distToFocusPos * ray;
	}


	double calcFovInY() const
	{
		return ( 2 * atan( CAMERA_SENSOR_H / 2 / m_fLen) ) * 360.0 / (2 * M_PI);
	}

	void drawCameraRect(TImage2D &img)
	{
		double scale = 3.0;

		EVec3d p,f,Y;
		getCamearaPos( p,f,Y);

		EVec3d ray = f-p;
		ray.normalize();
		EVec3d X = - Y.cross( ray ) ;

		EVec3d center = p + scale * m_fLen * ray;
		EVec3d x0 = center - (CAMERA_SENSOR_W * 0.5 ) * scale * X -  (CAMERA_SENSOR_H * 0.5 ) * scale * Y;
		EVec3d x1 = center + (CAMERA_SENSOR_W * 0.5 ) * scale * X -  (CAMERA_SENSOR_H * 0.5 ) * scale * Y;
		EVec3d x2 = center + (CAMERA_SENSOR_W * 0.5 ) * scale * X +  (CAMERA_SENSOR_H * 0.5 ) * scale * Y;
		EVec3d x3 = center - (CAMERA_SENSOR_W * 0.5 ) * scale * X +  (CAMERA_SENSOR_H * 0.5 ) * scale * Y;

		glDisable( GL_LIGHTING );
		glColor3d(1, 1, 0);
		glLineWidth( 5 );
		glBegin( GL_LINES );
			glVertex3dv( p .data() ); glVertex3dv( x0.data() );
			glVertex3dv( p .data() ); glVertex3dv( x1.data() );
			glVertex3dv( p .data() ); glVertex3dv( x2.data() );
			glVertex3dv( p .data() ); glVertex3dv( x3.data() );
			glVertex3dv( x0.data() ); glVertex3dv( x1.data() );
			glVertex3dv( x1.data() ); glVertex3dv( x2.data() );
			glVertex3dv( x2.data() ); glVertex3dv( x3.data() );
			glVertex3dv( x3.data() ); glVertex3dv( x0.data() );
		glEnd();

		glColor4d( 1,1,1,0.8 );
		glEnable( GL_BLEND     );
		glEnable( GL_TEXTURE_2D);
		img.Bind(0);
		glBegin (GL_QUADS);
			glTexCoord2d(0,0); glVertex3dv( x0.data() );
			glTexCoord2d(1,0); glVertex3dv( x1.data() );
			glTexCoord2d(1,1); glVertex3dv( x2.data() );
			glTexCoord2d(0,1); glVertex3dv( x3.data() );
		glEnd();
		glDisable( GL_BLEND );
		glDisable( GL_TEXTURE_2D);
	}
};




inline void t_drawMesh(const TTexMesh &m)
{
	glEnable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	const EVec3d *verts = m.m_verts;
	const EVec3d *norms = m.m_v_norms;
	for (int i = 0; i < m.m_pSize; ++i)
	{
		const TPoly &p = m.m_polys[i];
		glNormal3dv(norms[p.vIdx[0]].data()); glVertex3dv(verts[p.vIdx[0]].data());
		glNormal3dv(norms[p.vIdx[1]].data()); glVertex3dv(verts[p.vIdx[1]].data());
		glNormal3dv(norms[p.vIdx[2]].data()); glVertex3dv(verts[p.vIdx[2]].data());
	}
	glEnd();
}







