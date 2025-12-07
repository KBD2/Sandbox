#pragma once

#include "olcPixelGameEngine.h"
#include "sandbox.h"

class Renderer {
public:
	void renderArea(olc::PixelGameEngine* ctx);

	void renderUI(olc::PixelGameEngine* ctx);

	olc::Pixel calculatePixel(ParticleState* particle);
};

uint8_t lerpCompAlpha(uint8_t a, uint8_t b, uint8_t alpha);
olc::Pixel lerpPixel(olc::Pixel a, olc::Pixel b, float t);