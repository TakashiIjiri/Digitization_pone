#pragma once



#include "../../Common/timage.h"
#include "../../Common/CameraInfo.h"



class TextureHoleFill
{
public:
	TextureHoleFill();
	~TextureHoleFill();

	static void fillHoleByDilation(
		const TImage2D  &texture  , 
		const int       *polyIdImg,
		const int       *seamMapImg,
		const TTexMesh  &mesh     ,
		TImage2D &resTex    
	);

	static void fillHoleBySynthesis(){}//YET!!

};

