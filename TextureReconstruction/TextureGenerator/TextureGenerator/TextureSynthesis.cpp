#include "stdafx.h"
#include "TextureSynthesis.h"


#include "tqueue.h"
#include "TOGL.h"
TextureSynthesis::TextureSynthesis()
{
}


TextureSynthesis::~TextureSynthesis()
{
}




byte t_randB()
{
	return (byte)( rand() / (double) RAND_MAX * 255.0);
}






static inline void t_setRGBA(const int i, const byte r, const byte g, const byte b, const byte a, byte* rgba)
{
	rgba[4 *i  ] = r;
	rgba[4 *i+1] = g;
	rgba[4 *i+2] = b;
	rgba[4 *i+3] = a;
}

static inline float t_diffRGBA(const byte *rgba1, const byte *rgba2)
{
	float f = 
		(rgba1[0] - rgba2[0]) * (rgba1[0] - rgba2[0]) + 
		(rgba1[1] - rgba2[1]) * (rgba1[1] - rgba2[1]) + 
		(rgba1[2] - rgba2[2]) * (rgba1[2] - rgba2[2]) ;
	return sqrt( f );
}



void NonparametricTextureSynthesis
(
	const int tW,
	const int tH,
	const byte *tRGBA,
	const int W,
	const int H,
	byte *RGBA
)
{

	if( W < 10 || H < 10 ) return;

	byte *flg = new byte[W*H];
	memset( flg, 0, sizeof( byte)*W*H);


	//initialize seed -------------------------------------
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			const int I = W/2 + x + (H/2 + y) * W;
			flg[I] = 2;
			t_setRGBA( I, t_randB(), t_randB(), t_randB(),0, RGBA);
		}
	}


	//initialize fronteer --------------------------------
	TQueue<EVec3i> Q;
	for (int y = 1; y < H - 1; ++y)
	{
		for (int x = 1; x < W-1; ++x)
		{
			const int I = x + y * W; 
			if( flg[I] ==0 && (flg[I-1]==2 || flg[I+1]==2 || flg[I-W]==2 || flg[I+W]==2) )
			{
				Q.push_back(EVec3i(x,y,I));
				flg[I] = 1;
			}
		}
	}


	fprintf( stderr, "%d %d %d %d %d\n", tW, tH, W, H, Q.size());


	//grow ----------------------------------------------
	const double THRESH_RATE = 1.1;
	const int    WINR    = 3           ;
	const int    WINL    = 2 * WINR + 1;
	const int    tWH     = tW*tH;

	while (!Q.empty())
	{

		EVec3i xyi = Q.front();
		Q.pop_front();

		if (flg[xyi[2]] != 1)
		{
			fprintf( stderr, "strange deque\n");
			continue;
		}

		//local window of trgt pixel(alpha ch indicate existance flg) 
		byte  *localW = new byte[4 * WINL * WINL];
		memset( localW, 0, sizeof( byte ) * 4 * WINL * WINL );

		for (int yy = -WINR; yy <= WINR; ++yy) if( 0 <= xyi[1] + yy && xyi[1] + yy < H )
		{
			for (int xx = -WINR; xx <= WINR; ++xx) if( 0 <= xyi[0] + xx && xyi[0] + xx < W )
			{
				const int I  = xx+xyi[0] + (yy+xyi[1]) * W   ;
				const int wI = xx+WINR   + (yy+WINR  ) * WINL;
				if( flg[I] != 2 ) continue;
				t_setRGBA( wI, RGBA[4*I+0], RGBA[4*I+1], RGBA[4*I+2], 255, localW);
			}
		}




		//gen diff map
		float *diffMap = new float[tWH];
		for( int i=0; i< tWH; ++i) diffMap [i] = FLT_MAX;

#pragma omp parallel for
		for (int y = WINR; y < tH - WINR; ++y)
		for (int x = WINR; x < tW - WINR; ++x)
		{
			const int I = x + y * tW;
			diffMap[I] = 0;
			for( int yy = -WINR; yy <= WINR; ++yy)
			for (int xx = -WINR; xx <= WINR; ++xx)
			{
				const int tI = xx+x    + (yy + y ) * tW  ;
				const int wI = xx+WINR + (yy+WINR) * WINL;

				if( localW[4 * wI + 3]  == 0) continue;
				diffMap[I] += t_diffRGBA( &localW[4 * wI], &tRGBA[4*tI] );
			}
		}

		float  minDiff = FLT_MAX;
		EVec3i minPix; 
		for (int y = WINR; y < tH - WINR; ++y)
		for (int x = WINR; x < tW - WINR; ++x)
		{
			const int I = x + y * tW;

			if (diffMap[I] < minDiff)
			{
				minDiff = diffMap[I];
				minPix << x,y,I;
			}
		}

		//fprintf( stderr, "found ! %d %d %d\n", minPix[0], minPix[1], minPix[2]);
		
		//GROW : select one pixel by using diff map 
		t_setRGBA(xyi[2], tRGBA[4*minPix[2]], tRGBA[4*minPix[2]+1], tRGBA[4*minPix[2]+2],0, RGBA);
		flg[ xyi[2] ] = 2;

		if( xyi[0] > 0  && flg[xyi[2]-1] == 0 ) {Q.push_back( EVec3i(xyi[0]-1, xyi[1]  , xyi[2]-1) ); flg[xyi[2]-1] = 1;}
		if( xyi[0] <W-1 && flg[xyi[2]+1] == 0 ) {Q.push_back( EVec3i(xyi[0]+1, xyi[1]  , xyi[2]+1) ); flg[xyi[2]+1] = 1;}
		if( xyi[1] > 0  && flg[xyi[2]-W] == 0 ) {Q.push_back( EVec3i(xyi[0]  , xyi[1]-1, xyi[2]-W) ); flg[xyi[2]-W] = 1;}
		if( xyi[1] <H-1 && flg[xyi[2]+W] == 0 ) {Q.push_back( EVec3i(xyi[0]  , xyi[1]+1, xyi[2]+W) ); flg[xyi[2]+W] = 1;}

		delete[] diffMap;
		delete[] localW ;

		static int count = 0;
		count++;
		if( count % 1000 == 0 ) fprintf( stderr, "%d\n", count);
	}



	delete[] flg;
}