#pragma once

#include <map>

#include "olcPixelGameEngine.h"
#include "sandbox.h"

enum State {
	SOLID,
	POWDER,
	LIQUID,
	GAS
};

class ParticleProperties {
public:
	State state;
	float mass;
	float frictionCoeff;
	olc::Pixel colour;
};

class ParticleDust : public ParticleProperties {
public:
	ParticleDust() {
		this->state = State::POWDER;
		this->mass = 0.4f;
		this->frictionCoeff = 0.5f;
		this->colour = olc::Pixel(0xff, 0xe0, 0xa0);
	}
};

class ParticleWater : public ParticleProperties {
public:
	ParticleWater() {
		this->state = State::LIQUID;
		this->mass = 1;
		this->frictionCoeff = 0.0f;
		this->colour = olc::Pixel(0x00, 0x00, 0xff);
	}
};

class ParticleBrick : public ParticleProperties {
public:
	ParticleBrick() {
		this->state = State::SOLID;
		this->colour = olc::Pixel(0xaa, 0xaa, 0xaa);
	}
};

extern std::map<ParticleType, ParticleProperties> propertyLookup;

inline ParticleProperties& getProps(ParticleType type) {
	return propertyLookup[type];
}