#pragma once
class TextureSynthesis
{
	TextureSynthesis();
public:
	~TextureSynthesis();

	static TextureSynthesis* getInst(){ static TextureSynthesis p; return &p;}
};




void NonparametricTextureSynthesis
(
	const int tW,
	const int tH,
	const byte *tRGBA,
	const int W, 
	const int H,
	byte *RGBA
);