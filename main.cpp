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

inline float toRads(float degrees) {
	return degrees * (PI / 180);
}

enum GravityType {
	VECTOR,
	RADIAL,
	OFF
};

struct {
	GravityType gravType = VECTOR;
	olc::vf2d gravVec = olc::vf2d(0, 0.05f);
} CONFIG;

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
	DUST
} ParticleType;

typedef struct {
	ParticleType type;
	olc::vf2d velocity;
	olc::vf2d delta;
} ParticleState;

typedef ParticleState (*area_t)[WIDTH];

class ParticleProperties {
public:
	float mass;
	float frictionCoeff;
	olc::Pixel colour;
};

class ParticleDust : public ParticleProperties {
public:
	ParticleDust() {
		this->mass = 0.2f;
		this->frictionCoeff = 0.5f;
		this->colour = olc::Pixel(0xff, 0xe0, 0xa0);
	}
};

static std::map<ParticleType, ParticleProperties> propertyLookup {
	{ParticleType::DUST, ParticleDust()}
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
	}

	void updatePhysicalParticle(olc::vi2d pos) {
		ParticleState& particle = this->area[pos.y][pos.x];
		olc::vf2d* velocity = &particle.velocity;
		
		olc::vf2d localGravity = getLocalGravity(pos);
		
		*velocity += localGravity * propertyLookup[particle.type].mass;
		olc::vf2d* delta = &particle.delta;
		*delta += *velocity;
		olc::vf2d toMove = (olc::vi2d) *delta;

		if (toMove.mag() > 0) {
			*delta -= toMove;

			olc::vi2d initial = pos + toMove;

			if (!inBounds(initial)) {
				clip(initial);
				handleFriction(particle);
				if (initial == pos) return;
			}
			if (get(initial)->type == ParticleType::NONE) {
				*get(initial) = *get(pos);
				resetParticle(pos);
				return;
			}

			handleFriction(particle);

			olc::vf2d normal = toMove.norm() * -1;
			olc::vf2d normalPolar = normal.polar();
			while (toMove.mag() >= 1) {
				olc::vi2d newPos = pos + toMove;
				clip(newPos);
				if (get(newPos)->type == ParticleType::NONE) {
					*velocity *= 0;
					*get(newPos) = *get(pos);
					resetParticle(pos);
					return;
				}
				float slideAngle = 90;
				olc::vf2d centre = olc::vf2d((float) newPos.x + 0.5, (float) newPos.y + 0.5);
				while (slideAngle > 20) {
					bool flipped = random() < 0.5;
					for (int i = 0; i < 2; i++) {
						olc::vf2d angled = olc::vf2d(1, normalPolar.y + toRads(flipped ? slideAngle : -slideAngle)).cart();
						olc::vi2d check = centre + angled;
						clip(check);
						if (get(check)->type == ParticleType::NONE) {
							*get(check) = *get(pos);
							resetParticle(pos);
							// We only want the velocity along the vector the particle deflects
							float component = (((olc::vf2d) newPos + normal - check)).norm().dot(normal);
							*velocity *= component;
							return;
						}
						flipped = !flipped;
					}
					slideAngle -= 90;
				}
				toMove -= toMove.norm();
			}
			// It's just not able to move
			*velocity *= 0;
		}
	}

	olc::vf2d getLocalGravity(olc::vi2d pos) {
		switch (CONFIG.gravType) {
		case GravityType::VECTOR:
			return CONFIG.gravVec;
			break;
		case GravityType::RADIAL:
		{
			olc::vf2d toCentre = olc::vi2d(WIDTH / 2, HEIGHT / 2) - pos;
			if (toCentre.mag() == 0) return olc::vf2d(0, 0);
			else return toCentre.norm() * std::min(0.05f, 20 / toCentre.mag());
			break;
		}
		case GravityType::OFF:
		default:
			return olc::vf2d(0, 0);
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

	void clip(olc::vi2d& pos) {
		pos.x = std::min(std::max(pos.x, 0), WIDTH - 1);
		pos.y = std::min(std::max(pos.y, 0), HEIGHT - 1);
	}

	void handleFriction(ParticleState& particle) {
		ParticleProperties properties = propertyLookup[particle.type];
		particle.velocity *= properties.frictionCoeff;
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

	olc::Pixel calculatePixel(ParticleState& data) {
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
					particle->type = ParticleType::DUST;
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