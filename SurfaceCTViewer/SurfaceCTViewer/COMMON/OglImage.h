#pragma once

#include "OglForCLI.h"
#include "tqueue.h"
#include "./timageloader.h"

enum OGL_IMAGE_CH
{
	CH_INTES = 1,
	CH_RGBA = 4
};



template<class T>
void t_flipVolumeInZ(const int W, const int H, const int D, T* vol)
{
	const int WH = W*H;

	T *tmp = new T[WH];

	for (int z = 0; z < D / 2; ++z)
	{
		memcpy( tmp                 , &vol[ z       * WH ], sizeof( T ) * WH );
		memcpy( &vol[ z       * WH ], &vol[ (D-1-z) * WH ], sizeof( T ) * WH );
		memcpy( &vol[ (D-1-z) * WH ], tmp                 , sizeof( T ) * WH );
	}
	delete[] tmp;

}


//voxel value 0:never change, 1:background, 255:foreground
template <class T>
inline void t_erode3D(
	const int &W,
	const int &H,	
	const int &D,
	T *vol
)
{
	const int WH = W*H, WHD = W*H*D;

	for (int z = 0; z < D; ++z)
		for (int y = 0; y < H; ++y)
			for (int x = 0; x < W; ++x)
			{
				int idx = x + y * W + z*WH;
				if (vol[idx] != 255) continue;

				if (x == 0 || y == 0 || z == 0 || x == W - 1 || y == H - 1 || z == D - 1 ||
					vol[idx - 1] <= 1 || vol[idx - W] <= 1 || vol[idx - WH] <= 1 ||
					vol[idx + 1] <= 1 || vol[idx + W] <= 1 || vol[idx + WH] <= 1)
				{
					vol[idx] = 2;
				}
			}

	for (int i = 0; i < WHD; ++i) if (vol[i] == 2) vol[i] = 1;

}


//voxel value 0:never cahnge, 1:background, 255:foreground
template <class T>
inline void t_dilate3D(
	const int &W,
	const int &H,
	const int &D,
	T *vol
)
{
	const int WH = W*H, WHD = W*H*D;

	for (int z = 0; z < D; ++z)
		for (int y = 0; y < H; ++y)
			for (int x = 0; x < W; ++x)
			{
				int idx = x + y * W + z*WH;
				if (vol[idx] != 1) continue;

				if ((x >  0   && vol[idx - 1] == 255) || (y >  0   && vol[idx - W] == 255) || (z >  0   && vol[idx - WH] == 255) ||
					(x <W - 1 && vol[idx + 1] == 255) || (y <H - 1 && vol[idx + W] == 255) || (z <D - 1 && vol[idx + WH] == 255)) vol[idx] = 2;
			}

	for (int i = 0; i < WHD; ++i) if (vol[i] == 2) vol[i] = 255;

}



//TODO takashi double check the places that uses this function
//voxel value 0: background, 255:foreground
template <class T>
inline void t_fillHole3D(
	const int &W,
	const int &H,
	const int &D,
	T *vol
)
{
	const int WH = W*H, WHD = W*H*D;


	TQueue<EVec3i> Q(WHD / 10);

	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x)
		{
			if (vol[x + y * W + 0 * WH] == 0)      Q.push_back(EVec3i(x, y, 0));
			if (vol[x + y * W + (D - 1)*WH] == 0)  Q.push_back(EVec3i(x, y, (D - 1)));
		}

	for (int z = 0; z < D; ++z)
		for (int y = 0; y < H; ++y)
		{
			if (vol[0 + y * W + z*WH] == 0)        Q.push_back(EVec3i(0, y, z));
			if (vol[(W - 1) + y * W + z*WH] == 0)  Q.push_back(EVec3i(W - 1, y, z));
		}

	for (int z = 0; z < D; ++z)
		for (int x = 0; x < W; ++x)
		{
			if (vol[x + 0 * W + z*WH] == 0)       Q.push_back(EVec3i(x, 0, z));
			if (vol[x + (H - 1)* W + z*WH] == 0)  Q.push_back(EVec3i(x, H - 1, z));
		}

	for (int i = 0; i < Q.size(); ++i) vol[Q[i].x() + Q[i].y()*W + Q[i].z()*WH] = 2;



	//region growing for background
	while (!Q.empty())
	{
		const int x = Q.front().x();
		const int y = Q.front().y();
		const int z = Q.front().z();
		const int I = x + y*W + z*WH;
		Q.pop_front();

		if (x != 0     && !vol[I - 1])  { vol[I - 1] = 2;   Q.push_back(EVec3i(x-1, y,   z)); }
		if (x != W - 1 && !vol[I + 1])  { vol[I + 1] = 2;   Q.push_back(EVec3i(x+1, y,   z)); }
		if (y != 0     && !vol[I - W])  { vol[I - W] = 2;   Q.push_back(EVec3i(x,   y-1, z)); }
		if (y != H - 1 && !vol[I + W])  { vol[I + W] = 2;   Q.push_back(EVec3i(x,   y+1, z)); }
		if (z != 0     && !vol[I - WH]) { vol[I - WH] = 2;  Q.push_back(EVec3i(x,   y,   z-1)); }
		if (z != D - 1 && !vol[I + WH]) { vol[I + WH] = 2;  Q.push_back(EVec3i(x,   y,   z+1)); }
	}

	for (int i = 0; i < WHD; ++i) vol[i] = (vol[i] == 2) ? 0 : 255;

}



/*
//voxel value 0: none change, 1: background, 255:foreground
template <class T>
inline void t_fillHole3D(
	const int &W,
	const int &H,
	const int &D,
	T *vol
)
{
	const int WH = W*H, WHD = W*H*D;


	TQueue<EVec3i> Q(WHD / 10);

	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x)
		{
			if (vol[x + y * W + 0 * WH] == 1)      Q.push_back(EVec3i(x, y, 0));
			if (vol[x + y * W + (D - 1)*WH] == 1)  Q.push_back(EVec3i(x, y, (D - 1)));
		}

	for (int z = 0; z < D; ++z)
		for (int y = 0; y < H; ++y)
		{
			if (vol[0 + y * W + z*WH] == 1)        Q.push_back(EVec3i(0, y, z));
			if (vol[(W - 1) + y * W + z*WH] == 1)  Q.push_back(EVec3i(W - 1, y, z));
		}

	for (int z = 0; z < D; ++z)
		for (int x = 0; x < W; ++x)
		{
			if (vol[x + 0 * W + z*WH] == 1)       Q.push_back(EVec3i(x, 0, z));
			if (vol[x + (H - 1)* W + z*WH] == 1)  Q.push_back(EVec3i(x, H - 1, z));
		}

	for (int i = 0; i < Q.size(); ++i) vol[Q[i].x() + Q[i].y()*W + Q[i].z()*WH] = 2;



	//region growing for background
	while (!Q.empty())
	{
		const int x = Q.front().x();
		const int y = Q.front().y();
		const int z = Q.front().z();
		const int I = x + y*W + z*WH;
		Q.pop_front();

		if (x != 0     && vol[I - 1] == 1) { vol[I - 1] = 2;   Q.push_back(EVec3i(x - 1, y, z)); }
		if (x != W - 1 && vol[I + 1] == 1) { vol[I + 1] = 2;   Q.push_back(EVec3i(x + 1, y, z)); }
		if (y != 0     && vol[I - W] == 1) { vol[I - W] = 2;   Q.push_back(EVec3i(x, y - 1, z)); }
		if (y != H - 1 && vol[I + W] == 1) { vol[I + W] = 2;   Q.push_back(EVec3i(x, y + 1, z)); }
		if (z != 0     && vol[I - WH]== 1) { vol[I - WH] = 2;  Q.push_back(EVec3i(x, y, z - 1)); }
		if (z != D - 1 && vol[I + WH]== 1) { vol[I + WH] = 2;  Q.push_back(EVec3i(x, y, z + 1)); }
	}

	for (int i = 0; i < WHD; ++i) {
		if (vol[i] == 1)     vol[i] = 255;
		else if(vol[i] == 2) vol[i] = 1;
	}
}
*/












// single channel volume image
// Note oglBindについて
//   - bindのタイミングで bUpdated == trueなら unbindする
//   - unbindを忘れるとGRAMでleakするので注意


class OglImage3D
{
private:
	GLuint m_oglName ;
	EVec3i m_res     ;
	bool   m_bUpdated;
	GLubyte *m_vol;

public:


	~OglImage3D()
	{
		if (m_vol) delete[] m_vol;
	}


	OglImage3D()
	{
		m_res = EVec3i(0,0,0);
		m_vol = 0;
		m_oglName = -1;
		m_bUpdated = true;
	}

	OglImage3D(const OglImage3D &src)
	{
		copy(src);
	}

	OglImage3D &operator=( const OglImage3D &src)
	{
		copy(src);
		return *this;
	}

private:
	void copy(const OglImage3D &src)
	{
		m_res = src.m_res;
		if( src.m_vol )
		{
			int N = m_res[0] * m_res[1] * m_res[2];
			m_vol = new GLubyte[N];
			memcpy( m_vol, src.m_vol, sizeof( GLubyte ) * N );
		}
	}

public:
	void Allocate(const int W, const int H, const int D )
	{
		if (m_vol) delete[] m_vol;
		m_res = EVec3i( W,H,D);
		m_vol = new GLubyte[m_res[0] * m_res[1] * m_res[2]];
		m_bUpdated = true;
	}

	void Allocate(const EVec3i reso)
	{
		Allocate( reso[0], reso[1], reso[2]); 
	}


	

	//should be wglMakeCurrent
	void bindOgl( bool bInterpolate = true )
	{


		if( m_bUpdated ) 
		{
			unbindOgl(); 
			m_bUpdated = false;
		}

		if( m_oglName == -1 || !glIsTexture( m_oglName ) )
		{
			//generate textrue
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glGenTextures(1, &m_oglName);
			
			glBindTexture(GL_TEXTURE_3D, m_oglName);
			glTexImage3D (GL_TEXTURE_3D, 0, GL_LUMINANCE8, m_res[0], m_res[1], m_res[2], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_vol );
		}
		else
		{
			//use the previous texture on GPU
			glBindTexture(GL_TEXTURE_3D, m_oglName);
		}

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

		GLint param = bInterpolate ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, param);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, param);
	}


	//should be wglMakeCurrent
	void unbindOgl()
	{
		if (m_oglName != -1 && glIsTexture(m_oglName) ) glDeleteTextures(1, &m_oglName);
		m_oglName = -1;
	}







	// should be allocated & the size of v is samge as m_res
	template<class T>
	void SetValue(const T* v, const T minV, const T maxV)
	{
		if ( m_vol == 0 ) return;

		float rate = 255.0f / (maxV - minV);
		int N = m_res[0] * m_res[1] * m_res[2];

#pragma omp parallel for 
		for (int i = 0; i < N; ++i) m_vol[i] = (byte)max(0, min(255, (v[i] - minV) * rate));

		m_bUpdated = true;
	}
	
	void SetValue( byte *v )
	{
		if ( m_vol == 0 ) return;
		memcpy( m_vol, v, sizeof( byte) * m_res[0] * m_res[1] * m_res[2] );
		m_bUpdated = true;
	}


	void SetValue(float** slices, const float minV, const float maxV)
	{
		if (m_vol == 0) return;

		float rate = 255.0f / (maxV - minV);
		int WH = m_res[0] * m_res[1];
		
		for (int z = 0; z < m_res[2]; ++z)
		{
			for (int i = 0; i < WH; ++i)
			{
				m_vol[z * WH + i] = (byte)max(0, min(255, (slices[z][i] - minV) * rate));
			}
		}

		m_bUpdated = true;
	}

	void SetAllZero()
	{
		memset(m_vol, 0, sizeof(GLubyte) * m_res[0] * m_res[1] * m_res[2]);
		m_bUpdated = true;
	}

	inline GLubyte& operator[](int i) { return m_vol[i]; }
	inline GLubyte  operator[](int i) const { return m_vol[i]; }

	inline GLubyte getV(const int x, const int y, const int z) const
	{
		return m_vol[ x + y * m_res[0] + z * m_res[0] * m_res[1] ];
	}

	inline GLubyte setV(const int x, const int y, const int z, GLubyte v)
	{
		m_vol[x + y * m_res[0] + z * m_res[0] * m_res[1]] = v;
		m_bUpdated = true;
	}

	inline void setUpdated() { m_bUpdated = true; }

	const int getW() { return m_res[0]; }
	const int getH() { return m_res[1]; }
	const int getD() { return m_res[2]; }
	GLubyte* getVol() { return m_vol; }

	void flipVolumeInZ() {
		t_flipVolumeInZ<byte>( m_res[0],m_res[1],m_res[2], m_vol);
	}
};



//voxel value 0:never change, 1:background, 255:foreground
inline void t_morpho3D_erode(OglImage3D &v)
{
	const int W = v.getW();
	const int H = v.getH();
	const int D = v.getD();
	
	t_erode3D(W, H, D, v.getVol());

	v.setUpdated();
}



//voxel value 0:never cahnge, 1:background, 255:foreground
inline void t_morpho3D_dilate( OglImage3D &v )
{
	const int W = v.getW();
	const int H = v.getH();
	const int D = v.getD();

	t_dilate3D(W, H, D, v.getVol());

	v.setUpdated();
}




//voxel value 0: background, 255:foreground
inline void t_morpho3D_FillHole(OglImage3D &v)
{
	const int W = v.getW();
	const int H = v.getH();
	const int D = v.getD();
	
	t_fillHole3D(W, H, D, v.getVol());

	v.setUpdated();
}









//2D image 
template <OGL_IMAGE_CH CH>
class OglImage2D
{
protected:
	GLubyte *m_img     ;
	GLuint   m_oglName ;
	EVec2i   m_res     ;
	bool     m_bUpdated;

public:
	~OglImage2D()
	{
		if (m_img) delete[] m_img;
	}

	OglImage2D()
	{
		m_res << 0, 0;
		m_img      = 0;
		m_oglName  = -1;
		m_bUpdated = true;
	}

	OglImage2D(const OglImage2D &src)
	{
		copy(src);
	}

	OglImage2D &operator=(const OglImage2D &src)
	{
		copy(src);
		return *this;
	}
	inline GLubyte& operator[](int i)       { return m_img[i]; }
	inline GLubyte  operator[](int i) const { return m_img[i]; }


	int getWidth () const { return m_res[0]; }
	int getHeight() const { return m_res[1]; }

private:
	void copy(const OglImage2D &src)
	{
		m_res = src.m_res;
		//m_oglName = src.m_oglName; コピーしない
		if ( src.m_img )
		{
			m_rgba = new GLubyte[m_res[0] * m_res[1] * CH];
			memcpy(m_img, src.m_img, sizeof(GLubyte) * m_res[0] * m_res[1] * CH);
		}
	}
public:

	void Allocate(const int W, const int H)
	{
		if (m_img) delete[] m_img;
		m_res << W, H;
		m_img = new GLubyte[m_res[0] * m_res[1] * CH];
		m_bUpdated = true;
	}

	//bmp/png/tif/jpeg (only 24 bit color image) are supported
	//画像左上が原点となるので、OpenGL利用の際はflipする
	bool Allocate (const char *fname);
	bool SaveAs   (const char *fname, int flg_BmpJpgPngTiff);


	//should be wglMakeCurrent
	void unbindOgl()
	{
		if (m_oglName != -1 && glIsTexture(m_oglName) ) glDeleteTextures(1, &m_oglName);
		m_oglName = -1;
	}

	void FlipInY()
	{
		const int W = m_res[0];
		const int H = m_res[1];
		byte *tmp = new byte[ W * H * CH ];
		for( int y = 0; y < H; ++y) memcpy( &tmp[ CH*(H-1-y)*W ], &m_img[ 4*y* W ], sizeof( byte ) * CH * W ); 
		memcpy( m_img, tmp, sizeof( byte ) * W * H * CH );
		delete[] tmp;
		m_bUpdated = true;
	}


	//should be wglMakeCurrent
	void bindOgl(bool bInterpolate = true)
	{
		if (m_bUpdated)
		{
			unbindOgl();
			m_bUpdated = false;
		}

		if (m_oglName == -1 || !glIsTexture(m_oglName))
		{
			//generate textrue
			glGenTextures(1, &m_oglName);
			glBindTexture(GL_TEXTURE_2D, m_oglName);
			sendImageToGPU();
		}
		else
		{
			//use the previous texture on GPU
			glBindTexture(GL_TEXTURE_2D, m_oglName);
		}

		//CHに異存、ここにあったらだめ　glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

		GLint param = bInterpolate ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
	}

	bool isAllocated(){ return m_img ? true:false;}
private:
	void sendImageToGPU();
};




inline void OglImage2D<CH_RGBA>::sendImageToGPU()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_res[0], m_res[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, m_img);
}

inline void OglImage2D<CH_INTES>::sendImageToGPU()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, m_res[0], m_res[1], 0, GL_LUMINANCE8, GL_UNSIGNED_BYTE, m_img);
}


inline bool OglImage2D<CH_RGBA>::Allocate(const char *fname)
{
	if (m_img != 0) delete[] m_img;
	m_oglName = -1;
	m_img = 0;
	m_bUpdated = true;
	return t_loadImage(fname, m_res[0], m_res[1], m_img);
}



inline bool OglImage2D<CH_RGBA>::SaveAs( const char *fname, int flg_BmpJpgPngTiff)
{
	t_saveImage(fname, m_res[0], m_res[1], m_img);
	return true;
}



// rgba 1D image 
template <OGL_IMAGE_CH CH>
class OglImage1D
{
protected:
	GLubyte *m_img     ;
	GLuint   m_oglName ;
	int      m_N       ;
	bool     m_bUpdated;

public:
	~OglImage1D()
	{ 
		if (m_img) delete[] m_img;
	}

	OglImage1D() 
	{
		m_N        = 0   ;
		m_img      = 0   ;
		m_oglName  = -1  ;
		m_bUpdated = true;
	}

	OglImage1D(const OglImage1D &src)
	{ 
		copy(src); 
	}

	OglImage1D &operator=(const OglImage1D &src) 
	{
		copy(src);
		return *this;
	}

private:
	void copy(const OglImage1D &src)
	{
		m_N = src.m_N;
		// m_oglName = src.m_oglName;
		if (src.m_img)
		{
			m_rgba = new GLubyte[ m_N * CH ];
			memcpy(m_img, src.m_img, sizeof(GLubyte) * m_N * CH);
		}
	}
public:

	void Allocate(const int N )
	{
		if (m_img) delete[] m_img;
		m_N = N;
		m_img = new GLubyte[ m_N * CH];
	}

	void AllocateHeuImg(const int N)
	{
		Allocate(N);
		const float S = N / 6.0f;
		Allocate(N);
		for (int i = 0; i < N; ++i)
		{
			EVec3f C;
			if (     i < 1 * S ) C << 1          ,  (i-0*S)/S , 0; 
			else if( i < 2 * S ) C << 1-(i-1*S)/S,  1         , 0;
			else if( i < 3 * S ) C << 0          ,  1         , (i-S*2)/S;
			else if( i < 4 * S ) C << 0          ,1-(i-S*3)/S , 1;
			else if( i < 5 * S ) C <<   (i-4*S)/S,  0         , 1;
			else if( i < 6 * S ) C << 1          ,  0         ,1-(i-S*3)/S;
			m_img[4 * i  ] = (byte)(C[0] * 255);
			m_img[4 * i+1] = (byte)(C[1] * 255);
			m_img[4 * i+2] = (byte)(C[2] * 255);
		}

	}








	//should be wglMakeCurrent
	void unbindOgl()
	{
		if (m_oglName != -1 && glIsTexture(m_oglName) ) glDeleteTextures(1, &m_oglName);
		m_oglName = -1;
	}


	//should be wglMakeCurrent
	void bindOgl(bool bInterpolate = true)
	{
		if (m_bUpdated) 
		{
			unbindOgl();
			m_bUpdated = false;
		}

		if (m_oglName == -1 || !glIsTexture(m_oglName))
		{
			//generate textrue
			glGenTextures(1, &m_oglName);
			glBindTexture(GL_TEXTURE_1D, m_oglName);
			sendImageToGPU();
		}
		else
		{
			//use the previous texture on GPU
			glBindTexture(GL_TEXTURE_1D, m_oglName);
		}

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

		GLint param = bInterpolate ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, param);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, param);
	}

	inline GLubyte& operator[](int i)       { return m_img[i]; }
	inline GLubyte  operator[](int i) const { return m_img[i]; }
	
	inline void setZero();
	void setUpdated() { m_bUpdated = true; }
private:
	void sendImageToGPU();
};

inline void OglImage1D<CH_RGBA>::sendImageToGPU()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, m_N, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_img);
}
inline void OglImage1D<CH_INTES>::sendImageToGPU()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE8, m_N, 0, GL_LUMINANCE8, GL_UNSIGNED_BYTE, m_img);
}


inline void OglImage1D<CH_INTES>::setZero()
{
	m_bUpdated = true; 
	memset(m_img, 0, sizeof(byte) * m_N);
}

inline void OglImage1D<CH_RGBA>::setZero()
{
	m_bUpdated = true; 
	memset(m_img, 0, sizeof(byte) * m_N * 4);
}

typedef OglImage1D<CH_RGBA > OGLImage1D4;
typedef OglImage1D<CH_INTES> OGLImage1D1;
typedef OglImage2D<CH_RGBA > OGLImage2D4;
typedef OglImage2D<CH_INTES> OGLImage2D1;





template<class T>
void t_sobel3D(const int W, const int H, const int D, const T* vol, T* res)
{
	const T sblX[3][3][3]={ { {-1,  0, +1},
							  {-2,  0, +2},
							  {-1,  0, +1} }, { {-2,  0,  2},
												{-4,  0,  4},
												{-2,  0,  2} }, { {-1,  0,  1},
																  {-2,  0,  2}, 
																  {-1,  0,  1}} };
	static T sblY[3][3][3]={ {{-1, -2, -1},
							  { 0,  0,  0},
							  { 1,  2,  1}},   {{-2, -4, -2},
												{ 0,  0,  0},
												{ 2,  4,  2}},  {{-1, -2, -1},
																{ 0,  0,  0},
																{ 1,  2,  1}} };
	static T sblZ[3][3][3]={ {{-1, -2, -1},
							  {-2, -4, -2},
							  {-1, -2, -1}},   {{ 0,  0,  0},
								  				{ 0,  0,  0},
												{ 0,  0,  0}}, {{ 1,  2,  1},
																{ 2,  4,  2},
																{ 1,  2,  1}} };

	const int WH = W*H, WHD = WH*D;

#pragma omp parallel for
	for (int i = 0; i < WHD; ++i)
	{
		const int z = (i                 ) / WH;
		const int y = (i - z * WH        ) / W ;
		const int x = (i - z * WH - y * W);

		T gx = 0, gy = 0, gz = 0;

		for( int zz=-1; zz < 2; ++zz) if( 0<=z+zz && z+zz<D)
		for( int yy=-1; yy < 2; ++yy) if( 0<=y+yy && y+yy<H)
		for( int xx=-1; xx < 2; ++xx) if( 0<=x+xx && x+xx<W)
		{
			int I = i + xx + yy*W + zz*WH;
			gx += sblX[zz+1][yy+1][xx+1] * vol[ I ];
			gy += sblY[zz+1][yy+1][xx+1] * vol[ I ];
			gz += sblZ[zz+1][yy+1][xx+1] * vol[ I ];
		}

		//for boundary voxels
		if( x==0 || x==W-1) gx = 0; 
		if( y==0 || y==H-1) gy = 0; 
		if( z==0 || z==D-1) gz = 0; 
		res[i] = (T)sqrt( (double)gx * gx + gy * gy + gz * gz) / 16.0f;

		if( res[i] > 40000) fprintf( stderr, "%d %d %d\n",x,y,z);
	}

}



