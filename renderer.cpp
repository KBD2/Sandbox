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
			if (particle.type != Type::NONE) {
				ctx->Draw(x, y, calculatePixel(particle));
			}
		}
	}
}

uint8_t lerpCompAlpha(uint8_t a, uint8_t b, uint8_t alpha) {
	return (uint32_t)b * alpha / 255 + (uint32_t)a * (255 - alpha) / 255;
}

olc::Pixel lerpPixel(olc::Pixel a, olc::Pixel b, float t) {
	uint8_t tScaled = 255 * std::max(0.0f, std::min(1.0f, t));
	olc::Pixel composited;
	composited.a = lerpCompAlpha(a.a, b.a, tScaled);
	composited.r = lerpCompAlpha(a.r, b.r, tScaled);
	composited.g = lerpCompAlpha(a.g, b.g, tScaled);
	composited.b = lerpCompAlpha(a.b, b.b, tScaled);
	return composited;
}

olc::Pixel Renderer::calculatePixel(ParticleState& state) {
	std::shared_ptr<ParticleProperties> properties = propertyLookup[state.type];
	olc::Pixel colour = properties->render(state);
	olc::Pixel& deco = state.deco;
	olc::Pixel composited;
	composited.a = 0xff;
	composited.r = lerpCompAlpha(colour.r, deco.r, deco.a);
	composited.g = lerpCompAlpha(colour.g, deco.g, deco.a);
	composited.b = lerpCompAlpha(colour.b, deco.b, deco.a);
	return composited;
}

void Renderer::renderUI(olc::PixelGameEngine* ctx) {
	int windowHeight = ctx->GetDrawTargetHeight();
	ctx->FillRect(0, HEIGHT, WIDTH, windowHeight - HEIGHT + 1, olc::BLANK);
	ctx->DrawLine(olc::vi2d(0, HEIGHT), olc::vi2d(WIDTH - 1, HEIGHT), olc::GREY);
	for (auto& type : uiCtx.types) {
		std::shared_ptr<ParticleProperties> properties = propertyLookup[type.type];
		ctx->FillRect(type.uiPos, olc::vi2d(15, 10), properties->colour);
		if (type.type == uiCtx.selected) {
			ctx->DrawRect(olc::vi2d(type.uiPos.x - 1, type.uiPos.y - 1), olc::vi2d(16, 11), olc::RED);
		}
	}
	if (!CONFIG.ticking) {
		ctx->FillRect(WIDTH - 15, windowHeight - 10, 4, 8, olc::WHITE);
		ctx->FillRect(WIDTH - 9, windowHeight - 10, 4, 8, olc::WHITE);
	}
}