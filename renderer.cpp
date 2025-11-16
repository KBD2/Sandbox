#include "sandbox.h"
#include "particles.h"
#include "renderer.h"

Renderer::Renderer(area_t area) {
	this->area = area;
}

void Renderer::renderArea(olc::PixelGameEngine* ctx) {
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
	ParticleProperties& properties = propertyLookup[data.type];
	olc::Pixel& colour = properties.colour;
	olc::Pixel& deco = data.deco;
	olc::Pixel composited;
	composited.a = 0xff;
	composited.r = (uint32_t) deco.r * deco.a / 255 + (uint32_t) colour.r * (255 - deco.a) / 255;
	composited.g = (uint32_t) deco.g * deco.a / 255 + (uint32_t) colour.g * (255 - deco.a) / 255;
	composited.b = (uint32_t) deco.b * deco.a / 255 + (uint32_t) colour.b * (255 - deco.a) / 255;
	return composited;
}

void Renderer::renderUI(olc::PixelGameEngine* ctx) {
	ctx->FillRect(0, HEIGHT, WIDTH, ctx->GetDrawTargetHeight() - HEIGHT + 1, olc::BLANK);
	ctx->DrawLine(olc::vi2d(0, HEIGHT), olc::vi2d(WIDTH - 1, HEIGHT), olc::GREY);
	for (auto type : uiCtx.types) {
		ParticleProperties& properties = propertyLookup[type.type];
		ctx->FillRect(type.uiPos, olc::vi2d(15, 10), properties.colour);
		if (type.type == uiCtx.selected) {
			ctx->DrawRect(olc::vi2d(type.uiPos.x - 1, type.uiPos.y - 1), olc::vi2d(16, 11), olc::RED);
		}
	}
}