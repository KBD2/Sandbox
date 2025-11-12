#pragma once

#include "olcPixelGameEngine.h"
#include "sandbox.h"

class Renderer {
	area_t area;

public:
	Renderer(area_t area);

	void renderArea(olc::PixelGameEngine* ctx);

	void renderUI(olc::PixelGameEngine* ctx);

	olc::Pixel calculatePixel(ParticleState& data);
};