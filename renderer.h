#pragma once

#include "olcPixelGameEngine.h"
#include "sandbox.h"

class Renderer {
	area_t area;

public:
	Renderer(area_t area);

	void renderArea(olc::PixelGameEngine* ctx);

	void renderUI(olc::PixelGameEngine* ctx);

	olc::Pixel calculatePixel(ParticleState& state);
};

uint8_t lerpCompAlpha(uint8_t a, uint8_t b, uint8_t alpha);
olc::Pixel lerpPixel(olc::Pixel a, olc::Pixel b, float t);