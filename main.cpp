#include <iostream>
#include <vector>
#include <map>
#include <random>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

constexpr int WIDTH = 320;
constexpr int HEIGHT = 240;
constexpr float PI = 3.141592f;
float TICK_DURATION = 1.0f / 60.0f;
olc::vf2d GRAVITY = olc::vf2d(0, 0.05f);

static bool inBounds(int x, int y) {
	return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

static bool inBounds(olc::vi2d pos) {
	return pos.x >= 0 && pos.x < WIDTH && pos.y >= 0 && pos.y < HEIGHT;
}

static float random() {
	static std::random_device rd;
	static std::mt19937 e2(rd());
	static std::uniform_real_distribution<> dist(0, 1);
	return dist(e2);
}

typedef enum {
	NONE,
	SAND
} ParticleType;

typedef struct {
	ParticleType type;
	olc::vf2d velocity;
	olc::vf2d delta;
} ParticleState;

typedef ParticleState (*area_t)[WIDTH];

class ParticleProperties {
public:
	float mass = 0;
	olc::Pixel colour = olc::Pixel(0, 0, 0);
};

class ParticleSand : public ParticleProperties {
public:
	ParticleSand() {
		this->mass = 1.0;
		this->colour = olc::Pixel(255, 255, 0);
	}
};

static std::map<ParticleType, ParticleProperties> propertyLookup {
	{ParticleType::SAND, ParticleSand()}
};

class Simulation {
	area_t area;

public:
	Simulation(area_t area) {
		this->area = area;
	}

	void tick() {
		std::vector<olc::vi2d> toUpdate = std::vector<olc::vi2d>();
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				if (this->area[y][x].type != ParticleType::NONE) {
					toUpdate.push_back(olc::vi2d(x, y));
				}
			}
		}
		for (olc::vi2d& pos : toUpdate) {
			updatePhysicalParticle(pos);
		}

		olc::vf2d polar = GRAVITY.polar();
		polar.y += 0.01;
		GRAVITY = polar.cart();
	}

	void updatePhysicalParticle(olc::vi2d pos) {
		ParticleState& data = this->area[pos.y][pos.x];
		olc::vf2d* velocity = &data.velocity;
		*velocity += GRAVITY * propertyLookup[data.type].mass;
		olc::vf2d* delta = &data.delta;
		*delta += *velocity;
		olc::vf2d toMove = (olc::vi2d) *delta;
		if (toMove.mag() > 0) {
			*delta -= toMove;

			olc::vi2d initial = pos + toMove;

			if (!inBounds(initial)) {
				clip(initial);
				*delta *= 0;
				*velocity *= 0;
				if (initial == pos) return;
			}
			if (get(initial)->type == ParticleType::NONE) {
				*get(initial) = *get(pos);
				resetParticle(pos);
				return;
			}

			*delta *= 0;
			*velocity *= 0;

			while (toMove.mag() >= 1) {
				olc::vi2d newPos = pos + toMove;
				clip(newPos);
				if (get(newPos)->type == ParticleType::NONE) {
					*get(newPos) = *get(pos);
					resetParticle(pos);
					return;
				}
				toMove -= toMove.norm();

				olc::vi2d next = pos + toMove;
				olc::vf2d nextDelta = ((olc::vf2d) (newPos - next)).polar();
				nextDelta.x += 0.5;
				for (float angle = 0.25f * PI; angle > 0; angle -= 0.25 * PI) {
					olc::vf2d nextDeltaCheck = nextDelta;
					if (random() < 0.5) {
						nextDeltaCheck.y += angle;
					} else {
						nextDeltaCheck.y -= angle;
					}
					olc::vi2d angularCheck = next + nextDeltaCheck.cart();
					clip(angularCheck);
					if (get(angularCheck)->type == ParticleType::NONE) {
						*get(angularCheck) = *get(pos);
						resetParticle(pos);
						return;
					}
				}
			}
		}
	}

	ParticleState* get(olc::vi2d pos) {
		return &this->area[pos.y][pos.x];
	}

	void resetParticle(olc::vi2d pos) {
		ParticleState* particle = &this->area[pos.y][pos.x];
		particle->type = ParticleType::NONE;
		particle->velocity = olc::vf2d(0, 0);
		particle->delta = olc::vf2d(0, 0);
	}

	void clip(olc::vi2d &pos) {
		pos.x = std::min(std::max(pos.x, 0), WIDTH - 1);
		pos.y = std::min(std::max(pos.y, 0), HEIGHT - 1);
	}
};

class Renderer {
	area_t area;

public:
	Renderer(area_t area) {
		this->area = area;
	}

	void render(olc::PixelGameEngine* ctx) {
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

	olc::Pixel calculatePixel(ParticleState data) {
		ParticleProperties properties = propertyLookup[data.type];
		return properties.colour;
	}
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
					particle->type = ParticleType::SAND;
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