#include "sandbox.h"
#include "particles.h"
#include "renderer.h"

void Renderer::renderArea(olc::PixelGameEngine* ctx) {
	ctx->FillRect(0, 0, WIDTH, HEIGHT, olc::BLANK);
	for (int y = 0; y < PIX_Y; y++) {
		for (int x = 0; x < PIX_X; x++) {
			ParticleState* particle = partGrid[y][x];
			if (particle != nullptr) {
				ctx->FillRect(x * PIX_SIZE, y * PIX_SIZE, PIX_SIZE, PIX_SIZE, calculatePixel(particle));
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

olc::Pixel Renderer::calculatePixel(ParticleState* particle) {
	std::shared_ptr<ParticleProperties> properties = propertyLookup[particle->type];
	olc::Pixel colour = properties->render(particle);
	olc::Pixel& deco = particle->deco;
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
	ctx->FillRect(0, HEIGHT, WIDTH, 4, olc::GREY);
	for (auto& type : uiCtx.types) {
		std::shared_ptr<ParticleProperties> properties = propertyLookup[type.type];
		ctx->FillRect(type.uiPos, olc::vi2d(30, 20), properties->colour);
		if (type.type == uiCtx.selected) {
			ctx->DrawRect(olc::vi2d(type.uiPos.x - 1, type.uiPos.y - 1), olc::vi2d(31, 21), olc::RED);
			ctx->DrawRect(olc::vi2d(type.uiPos.x - 2, type.uiPos.y - 2), olc::vi2d(33, 23), olc::RED);
		}
	}
	if (!CONFIG.ticking) {
		ctx->FillRect(WIDTH - 15, windowHeight - 20, 4, 14, olc::WHITE);
		ctx->FillRect(WIDTH - 9, windowHeight - 20, 4, 14, olc::WHITE);
	}
}