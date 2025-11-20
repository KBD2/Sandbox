#include <iostream>
#include <cstdlib>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "sandbox.h"
#include "simulation.h"
#include "renderer.h"
#include "particles.h"

Config CONFIG = {
	.gravType = VECTOR,
	.gravVec = olc::vf2d(0, 0.05f)
};

UIContext uiCtx = {
	.types = std::vector<UIParticleType>(),
	.selected = ParticleType::DUST
};

class Sandbox : public olc::PixelGameEngine {
	area_t area = new ParticleState[HEIGHT][WIDTH];
	float timeTillUpdate = 0;
	Simulation sim = Simulation(this->area);
	Renderer renderer = Renderer(this->area);
	bool ticking = true;

public:
	Sandbox() {
		this->sAppName = "Sandbox";
		std::srand(std::time(NULL));

		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				this->sim.resetParticle(olc::vi2d(x, y));
			}
		}

		for (int type = 0; type < NONE; type++) {
			UIParticleType uiType = {
				.uiPos = olc::vi2d(3 + 20 * type, HEIGHT + 4),
				.type = (ParticleType) type
			};
			uiCtx.types.push_back(uiType);
		}
	}

	bool OnUserCreate() override {
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		this->renderer.renderArea(this);
		this->renderer.renderUI(this);
		bool update = false;
		this->timeTillUpdate -= fElapsedTime;
		if (this->timeTillUpdate <= 0) {
			this->timeTillUpdate = TICK_DURATION + this->timeTillUpdate;
			if (ticking) {
				this->sim.tick();
			}
		}
		handleInput();
		return true;
	}

private:
	void handleInput() {
		static int lastX = -1;
		static int lastY = -1;
		if (IsFocused()) {
			int x = GetMouseX();
			int y = GetMouseY();
			if (inBounds(x, y)) {
				if (GetMouse(0).bHeld) {
					if (lastX != -1) {

						int dx = x - lastX;
						int dy = y - lastY;

						float length = std::max(1.0f, std::sqrtf(std::pow(dx, 2) + std::pow(dy, 2)));

						float mx = (float)dx / length;
						float my = (float)dy / length;

						for (int t = 0; t < length; t++) {
							int lx = (float)lastX + mx * t;
							int ly = (float)lastY + my * t;
							if (inBounds(lx, ly)) {
								ParticleState* particle = &this->area[ly][lx];
								if (particle->type == ParticleType::NONE) {
									this->sim.resetParticle(olc::vi2d(lx, ly));
									particle->type = uiCtx.selected;
									if (getProps(particle->type)->state == State::POWDER && rand() % 2 == 0) {
										particle->deco = olc::Pixel(rand() % 256, rand() % 256, rand() % 256, rand() % 20);
									}
								}
							}
						}
					}
					lastX = x;
					lastY = y;
				} else {
					lastX = -1;
				}
				if (GetMouse(1).bHeld) {
					this->sim.resetParticle(olc::vi2d(x, y));
				}
			} else {
				lastX = -1;
				if (GetMouse(0).bPressed) {
					for (auto& type : uiCtx.types) {
						int buttonX = type.uiPos.x;
						int buttonY = type.uiPos.y;
						if (x >= buttonX && x < buttonX + 15 && y >= buttonY && y < buttonY + 10) {
							uiCtx.selected = type.type;
						}
					}
				}
			}
			if (GetKey(olc::Key::G).bPressed) {
				switch (CONFIG.gravType) {
				case GravityType::VECTOR:
					CONFIG.gravType = GravityType::RADIAL;
					std::cout << "Gravity: Radial" << std::endl;
					break;
				case GravityType::RADIAL:
					CONFIG.gravType = GravityType::OFF;
					std::cout << "Gravity: Off" << std::endl;
					break;
				case GravityType::OFF:
				default:
					CONFIG.gravType = GravityType::VECTOR;
					std::cout << "Gravity: Vector" << std::endl;
				}
			}
			if (GetKey(olc::Key::C).bPressed) {
				for (int y = 0; y < HEIGHT; y++) {
					for (int x = 0; x < WIDTH; x++) {
						this->sim.resetParticle(olc::vi2d(x, y));
					}
				}
			}
			if (GetKey(olc::Key::SPACE).bPressed) {
				this->ticking = !this->ticking;
			}
			if (GetKey(olc::Key::F).bPressed) {
				this->ticking = false;
				this->sim.tick();
			}
		}
	}
};

int main() {
	Sandbox game;
	if (game.Construct(WIDTH, HEIGHT + 20, PIX_SIZE, PIX_SIZE)) {
		game.Start();
	}
	return 0;
}