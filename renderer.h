#pragma once

#include "olcPixelGameEngine.h"
#include "sandbox.h"

class Renderer {
	area_t area;

public:
	Renderer(area_t area);

	void render(olc::PixelGameEngine* ctx);

	olc::Pixel calculatePixel(ParticleState& data);
};