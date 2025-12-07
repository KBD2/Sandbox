#pragma once

#include <map>
#include <string>

#include "olcPixelGameEngine.h"
#include "sandbox.h"
#include "renderer.h"
#include "simulation.h"

enum State {
	S_SOLID,
	S_POWDER,
	S_LIQUID,
	S_GAS
};

class ParticleProperties {
public:
	std::string name = "UNWN";
	State state = State::S_SOLID;

	double mass = 0;
	double frictionCoeff = 0;
	uint8_t reposeAngle = 0;

	bool flammable = false;

	double dispersion = 0;

	olc::Pixel colour = olc::MAGENTA;

	virtual void init(ParticleState* particle) {
	}
	virtual void update(ParticleState* particle) {
	}
	virtual olc::Pixel render(ParticleState* particle) {
		return this->colour;
	};
};

extern std::map<Type, std::shared_ptr<ParticleProperties>> propertyLookup;

inline std::shared_ptr<ParticleProperties> getProps(Type type) {
	return propertyLookup[type];
}

class ParticleDust : public ParticleProperties {
public:
	ParticleDust() {
		this->name = "DUST";
		this->state = State::S_POWDER;
		this->mass = 0.4;
		this->frictionCoeff = 0.5;
		this->reposeAngle = 45;
		this->colour = olc::Pixel(0xff, 0xe0, 0xa0);
		this->flammable = true;
	}
};

class ParticleWater : public ParticleProperties {
public:
	ParticleWater() {
		this->name = "WATR";
		this->state = State::S_LIQUID;
		this->mass = 1;
		this->frictionCoeff = 0.95;
		this->reposeAngle = 0;
		this->colour = olc::Pixel(0x00, 0x00, 0xff);
	}

	olc::Pixel render(ParticleState* particle) override {
		if (particle->data[0] > 0) {
			olc::Pixel brightest = olc::Pixel(0x88, 0x88, 0xff);
			olc::Pixel dimmest = olc::Pixel(0x55, 0x55, 0xff);
			return lerpPixel(this->colour, lerpPixel(dimmest, brightest, random()), (float)particle->data[0] / 1000);
		}
		return this->colour;
	}

	void update(ParticleState* state) override {
		if (state->velocity.mag2() > 1) {
			state->data[0] = std::min(state->data[0] + 10, 1000);
		} else {
			state->data[0] = std::max(state->data[0] - 20, 0);
		}
	}
};

class ParticleBrick : public ParticleProperties {
public:
	ParticleBrick() {
		this->name = "BRCK";
		this->state = State::S_SOLID;
		this->colour = olc::Pixel(0xaa, 0xaa, 0xaa);
	}
};

class ParticleAnar : public ParticleProperties {
public:
	ParticleAnar() {
		this->name = "ANAR";
		this->state = State::S_POWDER;
		this->mass = -0.4;
		this->frictionCoeff = 0.5f;
		this->colour = olc::Pixel(0xff, 0xff, 0xff);
	}
};

class ParticleGas : public ParticleProperties {
public:
	ParticleGas() {
		this->name = "GAS";
		this->state = State::S_GAS;
		this->mass = 0.0;
		this->frictionCoeff = 0.95f;
		this->colour = olc::Pixel(0xe4, 0xff, 0x35);
		this->flammable = true;
		this->dispersion = 1;
	}
};

class ParticleFire : public ParticleProperties {
public:
	ParticleFire() {
		this->name = "FIRE";
		this->state = State::S_GAS;
		this->mass = -0.3;
		this->frictionCoeff = 0.95f;
		this->colour = olc::Pixel(0xff, 0x48, 0x30);
		this->dispersion = 0.1;
	}

	void init(ParticleState* particle) override {
		particle->data[0] = random() * 100 + 100;
	}

	void update(ParticleState* particle) override {
		olc::vi2d pos = particle->pos;
		for (int dy = -1; dy <= 1; dy++) {
			for (int dx = -1; dx <= 1; dx++) {
				if (dx == 0 && dy == 0) continue;
				olc::vi2d checkPos = pos + olc::vi2d(dx, dy);
				if (inBounds(checkPos)) {
					ParticleState* check = partGrid[checkPos.y][checkPos.x];
					if (check == nullptr) continue;
					if (check->type != Type::NONE && getProps(check->type)->flammable) {
						getSimulation()->remove(check);
						getSimulation()->add(checkPos, Type::FIRE);
					}
				}
			}
		}
		if (--particle->data[0] == 0) {
			getSimulation()->remove(particle);
		}
	}

	olc::Pixel render(ParticleState* particle) override {
		olc::Pixel brightest = olc::Pixel(0xff, 0x00, 0x00);
		olc::Pixel dimmest = olc::Pixel(0x00, 0x00, 0x00);
		return lerpPixel(dimmest, brightest, (float)particle->data[0] / 100);
	}
};
