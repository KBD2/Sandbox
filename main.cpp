#include <iostream>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "sandbox.h"
#include "simulation.h"
#include "renderer.h"

Config CONFIG = {
	.gravType = VECTOR,
	.gravVec = olc::vf2d(0, 0.05f)
};

class Sandbox : public olc::PixelGameEngine {
	area_t area = new ParticleState[HEIGHT][WIDTH]{ ParticleType::NONE };
	float timeTillUpdate = 0;
	Simulation sim = Simulation(this->area);
	Renderer renderer = Renderer(this->area);

public:
	Sandbox() {
		this->sAppName = "Sandbox";
	}

	bool OnUserCreate() override {
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		this->renderer.render(this);
		bool update = false;
		this->timeTillUpdate -= fElapsedTime;
		if (this->timeTillUpdate <= 0) {
			this->timeTillUpdate = TICK_DURATION + this->timeTillUpdate;
			this->sim.tick();
		}
		handleInput();
		return true;
	}

private:
	void handleInput() {
		if (IsFocused()) {
			if (GetMouse(0).bHeld) {
				int x = GetMouseX();
				int y = GetMouseY();
				ParticleState* particle = &this->area[y][x];
				if (particle->type == ParticleType::NONE) {
					this->sim.resetParticle(olc::vi2d(x, y));
					particle->type = ParticleType::DUST;
				}
			}
			if (GetMouse(1).bHeld) {
				int x = GetMouseX();
				int y = GetMouseY();
				ParticleState* particle = &this->area[y][x];
				if (particle->type == ParticleType::NONE) {
					this->sim.resetParticle(olc::vi2d(x, y));
					particle->type = ParticleType::WATER;
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
		}
	}
};

int main() {
	Sandbox game;
	if (game.Construct(WIDTH, HEIGHT, 4, 4)) {
		game.Start();
	}
	return 0;
}