#include "sandbox.h"
#include "particles.h"
#include "renderer.h"

Renderer::Renderer(area_t area) {
	this->area = area;
}

void Renderer::render(olc::PixelGameEngine* ctx) {
	ctx->FillRect(0, 0, WIDTH, HEIGHT, olc::BLANK);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			ParticleState& particle = this->area[y][x];
			if (particle.type != ParticleType::NONE) {
				ctx->Draw(x, y, calculatePixel(particle));
			}
		}
	}
}

olc::Pixel Renderer::calculatePixel(ParticleState& data) {
	ParticleProperties properties = propertyLookup[data.type];
	return properties.colour;
}
