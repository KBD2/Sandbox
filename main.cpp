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
	.gravVec = olc::vf2d(0, 0.05f),
	.ticking = true
};

std::vector<ParticleState*> partArr = {};
area_t partGrid = new ParticleState*[PIX_Y][PIX_X];

static std::shared_ptr<Simulation> simulation;

std::shared_ptr<Simulation> getSimulation() {
	return simulation;
}

UIContext uiCtx = {
	.types = std::vector<UIParticleType>(),
	.selected = Type::DUST
};

class Sandbox : public olc::PixelGameEngine {
	float timeTillUpdate = 0;
	std::shared_ptr<Simulation> sim = std::make_shared<Simulation>();
	Renderer renderer = Renderer();

public:
	Sandbox() {
		this->sAppName = "Sandbox";
		std::srand(std::time(NULL));

		for (int y = 0; y < PIX_Y; y++) {
			for (int x = 0; x < PIX_X; x++) {
				partGrid[y][x] = nullptr;
			}
		}

		/*for (int y = 0; y < 50; y++) {
			for (int x = 0; x < 50; x++) {
				this->sim->add(olc::vi2d(x, y), Type::DUST);
			}
		}
		CONFIG.ticking = false;*/

		for (int type = 0; type < NONE; type++) {
			UIParticleType uiType = {
				.uiPos = olc::vi2d(4 + 40 * type, HEIGHT + 8),
				.type = (Type) type
			};
			uiCtx.types.push_back(uiType);
		}

		simulation = this->sim;
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
			this->timeTillUpdate = std::max(0.0f, TICK_DURATION + this->timeTillUpdate);
			if (CONFIG.ticking) {
				this->sim->tick();
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
			int pixX = x / PIX_SIZE;
			int pixY = y / PIX_SIZE;
			if (inBounds(pixX, pixY)) {
				if (GetMouse(0).bHeld || GetMouse(1).bHeld) {
					if (lastX != -1) {

						int dx = pixX - lastX;
						int dy = pixY - lastY;

						float length = std::max(1.0f, std::sqrtf(std::pow(dx, 2) + std::pow(dy, 2)));

						float mx = (float)dx / length;
						float my = (float)dy / length;

						for (int t = 0; t < length; t++) {
							int lx = (float)lastX + mx * t;
							int ly = (float)lastY + my * t;
							if (inBounds(lx, ly)) {
								ParticleState* particle = partGrid[ly][lx];
								if (GetMouse(0).bHeld && particle == nullptr) {
									particle = this->sim->add(olc::vi2d(lx, ly), uiCtx.selected);
									if (particle != nullptr && getProps(uiCtx.selected)->state == State::S_POWDER && rand() % 2 == 0) {
										particle->deco = olc::Pixel(rand() % 256, rand() % 256, rand() % 256, rand() % 20);
									}
								} else if (GetMouse(1).bHeld && particle != nullptr) {
									this->sim->remove(particle);
								}
							}
						}
					}
					lastX = pixX;
					lastY = pixY;
				} else {
					lastX = -1;
				}
			} else {
				lastX = -1;
				if (GetMouse(0).bPressed) {
					for (auto& type : uiCtx.types) {
						int buttonX = type.uiPos.x;
						int buttonY = type.uiPos.y;
						if (x >= buttonX && x < buttonX + 30 && y >= buttonY && y < buttonY + 20) {
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
				while (partArr.size() > 0) {
					this->sim->remove(partArr.back());
				}
			}
			if (GetKey(olc::Key::SPACE).bPressed) {
				CONFIG.ticking = !CONFIG.ticking;
			}
			if (GetKey(olc::Key::F).bPressed) {
				CONFIG.ticking = false;
				this->sim->tick();
			}
		}
	}
};

int main() {
	Sandbox game;
	if (game.Construct(WIDTH, HEIGHT + 40, 1, 1)) {
		game.Start();
	}
	return 0;
}