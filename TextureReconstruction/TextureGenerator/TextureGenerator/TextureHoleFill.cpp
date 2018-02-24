#include "stdafx.h"
#include "TextureHoleFill.h"

#include <list>
#include <map>

TextureHoleFill::TextureHoleFill()
{
}


TextureHoleFill::~TextureHoleFill()
{
}


static int Neibor[8][2] = 
{
	{-1,-1}, { 0,-1}, {+1,-1},
	{-1, 0},          {+1, 0},
	{-1,+1}, { 0,+1}, {+1,+1}
};

//texture   : alpha val = 0:hasVal, 1:foreButNoVal , 2:back
//polyIdImg : -1:back, >=0 : poly id 
void TextureHoleFill::fillHoleByDilation(
	const TImage2D  &texture   , 
	const int       *polyIdImg ,
	const int       *seamMapImg,
	const TTexMesh  &mesh      ,
	TImage2D &resTex    
)
{
	const int W = texture.m_W;
	const int H = texture.m_H;
	
	resTex.AllocateImage( texture, 0 );

	//dilate 1-ring pixel at each iteration

	int count = 0;
	while (count < 500)
	{
		count++;

		list<pair<EVec3i, EVec3i>> Q; //pix & neighbor Pix

		for( int y = 0; y < H; ++y)
		for( int x = 0; x < W; ++x)
		{
			int i = x + y * W;
			if ( resTex[4 * i + 3] != 1) continue;

			if(      x > 0   && resTex[4 * (i-1) + 3] == 0) Q.push_back( make_pair(EVec3i(x,y,i), EVec3i(x-1,y  ,i-1)) );
			else if( x < W-1 && resTex[4 * (i+1) + 3] == 0) Q.push_back( make_pair(EVec3i(x,y,i), EVec3i(x+1,y  ,i+1)) );
			else if( y > 0   && resTex[4 * (i-W) + 3] == 0) Q.push_back( make_pair(EVec3i(x,y,i), EVec3i(x  ,y-1,i-W)) );
			else if( y < H-1 && resTex[4 * (i+W) + 3] == 0) Q.push_back( make_pair(EVec3i(x,y,i), EVec3i(x  ,y+1,i+W)) );
			else if( seamMapImg[i] >= 0 && resTex[4 * seamMapImg[i] + 3] ==0 )
			{
				int xx =  seamMapImg[i] % W;
				int yy =  seamMapImg[i] / W;
				 Q.push_back( make_pair(EVec3i(x,y,i), EVec3i(xx  ,yy,seamMapImg[i])) );
			}
		}
		fprintf( stderr, "%d %d\n", count,(int) Q.size());

		for (const auto p : Q)
		{
			int tgtI =p.first [2] ;
			int srcI =p.second[2] ;
			memcpy( &resTex.m_RGBA[4 * tgtI],  &resTex.m_RGBA[4 * srcI], sizeof(byte) * 4 );

			//resTex.m_RGBA[4 * tgtI] = 255;
		}

		if( Q.size() ==0 ) break;
	}

}


