#include "MyForm.h"
#include <stdio.h>
#include <string>

#include "timageloader.h"
#include <list>
using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

using namespace ThickenTexAtlas;


using namespace std;





inline bool isBlack( const int i4, unsigned char* rgba)
{
	return rgba[i4] == 0 && rgba[i4+1] == 0 && rgba[i4+2] == 0;
}
inline bool isGray26( const int i4, unsigned char* rgba)
{
	return rgba[i4  ] == 26 && 
		   rgba[i4+1] == 26 && 
		   rgba[i4+2] == 26 ;
}


[STAThreadAttribute]
int main()
{
	OpenFileDialog^ dlg = gcnew OpenFileDialog();
	dlg->Filter = "3d dcm files(*.dcm;*)|*.dcm;*";
	if (dlg->ShowDialog() == System::Windows::Forms::DialogResult::Cancel) return 0;
	IntPtr mptr = Marshal::StringToHGlobalAnsi(dlg->FileName);
	string fname = static_cast<const char*>(mptr.ToPointer());

	int W,H;
	unsigned char *img;
	t_loadImage(fname.c_str(), W,H, img);

	const int WH = W*H;
	int *flg = new int[WH];
	memset(flg, 0, sizeof(int)*WH);
	for( int i=0; i < WH; ++i) flg[i] = isGray26(4*i, img) ? 1 : 0;


	//fill background (26,26,26) into black
	list<int> Q;
	Q.push_back(0);
	flg[0] = 0;

	//Q.push_back(W-1 );
	//Q.push_back(H-1 );
	//Q.push_back(WH-1);

	while(!Q.empty())
	{
		int i = Q.front();
		Q.pop_front();
		img[4*i+0] = img[4*i+1] = img[4*i+2] = 0;
		int x = i % W; 
		int y = i / W;
		//printf( "(%d, %d)", x,y);

		if(x>0   && flg[i-1] ) {Q.push_back(i-1);flg[i-1] = false;}
		if(x<W-1 && flg[i+1] ) {Q.push_back(i+1);flg[i+1] = false;}
		if(y>0   && flg[i-W] ) {Q.push_back(i-W);flg[i-W] = false;}
		if(y<H-1 && flg[i+W] ) {Q.push_back(i+W);flg[i+W] = false;}
	}

	for( int kkk = 0; kkk < 5; ++kkk)
	{
	
		memset(flg, 0, sizeof(int)*WH);

		for( int y = 1; y < H-1; ++y)
		for( int x = 1; x < W-1; ++x)
		{
			int i= x + y * W;
			if( isBlack(4*i, img) && 
				( !isBlack(4*(i-1), img) || !isBlack(4*(i-W), img) || 
				  !isBlack(4*(i+1), img) || !isBlack(4*(i+W), img) ) ) { flg[i] = 1;}
		}


		for( int y = 1; y < H-1; ++y)
		for( int x = 1; x < W-1; ++x)
		{
			int i= x + y * W;	
			if( flg[i] == 0 ) continue;
			int i4 = i*4;
			int K;
			K = i-1; if( !isBlack(4*K, img) ) memcpy( &img[i4], &img[4*K], sizeof(unsigned char)*4);
			K = i+1; if( !isBlack(4*K, img) ) memcpy( &img[i4], &img[4*K], sizeof(unsigned char)*4);
			K = i-W; if( !isBlack(4*K, img) ) memcpy( &img[i4], &img[4*K], sizeof(unsigned char)*4);
			K = i+W; if( !isBlack(4*K, img) ) memcpy( &img[i4], &img[4*K], sizeof(unsigned char)*4);
		}

	}



	string out = fname.substr(0,fname.length()-4) + string( "thicken.bmp");
	t_saveImage(out.c_str(), W,H,img);
	
	
	MyForm ^a = gcnew MyForm();
	a->ShowDialog();


	return 0;
}

