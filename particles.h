#pragma once

#include <map>
#include <string>

#include "olcPixelGameEngine.h"
#include "sandbox.h"
#include "renderer.h"

enum State {
	SOLID,
	POWDER,
	LIQUID,
	GAS
};

class ParticleProperties {
public:
	std::string name = "UNWN";
	State state = State::SOLID;
	float mass = 0;
	float frictionCoeff = 0;
	olc::Pixel colour = olc::MAGENTA;
	virtual olc::Pixel render(ParticleState& state) {
		return this->colour;
	};
	virtual void update(ParticleState& state, area_t area) {
	}
};

class ParticleDust : public ParticleProperties {
public:
	ParticleDust() {
		this->name = "DUST";
		this->state = State::POWDER;
		this->mass = 0.4f;
		this->frictionCoeff = 0.5f;
		this->colour = olc::Pixel(0xff, 0xe0, 0xa0);
	}
};

class ParticleWater : public ParticleProperties {
public:
	ParticleWater() {
		this->name = "WATR";
		this->state = State::LIQUID;
		this->mass = 1;
		this->frictionCoeff = 0.95f;
		this->colour = olc::Pixel(0x00, 0x00, 0xff);
	}

	olc::Pixel render(ParticleState& state) override {
		if (state.data[0] > 0) {
			olc::Pixel brightest = olc::Pixel(0x88, 0x88, 0xff);
			olc::Pixel dimmest = olc::Pixel(0x55, 0x55, 0xff);
			return lerpPixel(this->colour, lerpPixel(dimmest, brightest, random()), (float)state.data[0] / 1000);
		}
		return this->colour;
	}

	void update(ParticleState& state, area_t area) override {
		if (state.velocity.mag2() > 1) {
			state.data[0] = std::min(state.data[0] + 10, 1000);
		} else {
			state.data[0] = std::max(state.data[0] - 20, 0);
		}
	}
};

class ParticleBrick : public ParticleProperties {
public:
	ParticleBrick() {
		this->name = "BRCK";
		this->state = State::SOLID;
		this->colour = olc::Pixel(0xaa, 0xaa, 0xaa);
	}
};

class ParticleAnar : public ParticleProperties {
public:
	ParticleAnar() {
		this->name = "ANAR";
		this->state = State::POWDER;
		this->mass = -0.4f;
		this->frictionCoeff = 0.5f;
		this->colour = olc::Pixel(0xff, 0xff, 0xff);
	}
};

extern std::map<ParticleType, std::shared_ptr<ParticleProperties>> propertyLookup;

inline std::shared_ptr<ParticleProperties> getProps(ParticleType type) {
	return propertyLookup[type];
}