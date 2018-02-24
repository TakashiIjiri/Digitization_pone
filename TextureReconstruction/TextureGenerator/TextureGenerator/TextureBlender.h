#pragma once

#include "../../Common/timage.h"

#include <vector>
using namespace std;

#define GRAPH_DATATERM_COEF 0.01

const byte BACK_COL[] = {26,26,26, 2};
const byte FORE_NAN[] = { 0, 0, 0, 1};





class TextureBlender
{
public:
	TextureBlender();
	~TextureBlender();


	static void BlendTexture(
		const vector<TImage2D  > &textures, 
		const vector<float*    > &textures_angle, 
		const int                *imgMapSeamPix,
		TImage2D &resultTexture);
};

