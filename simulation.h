#pragma once

#include "sandbox.h"

class Simulation {
public:
	Simulation() {
		this->particlePool = new ParticleState[MAX_PARTS * 5];
		for (int i = 0; i < MAX_PARTS; i++) {
			this->particlePool[i].dead = true;
		}
	}

	void tick();
	void remove(ParticleState* state);
	ParticleState* add(olc::vi2d pos, Type type);

private:
	ParticleState* particlePool;

	void updatePhysicsParticle(ParticleState* particle);
	void updatePowder(ParticleState* particle);
	void updateLiquid(ParticleState* particle);
	void updateGas(ParticleState* particle);
	olc::vf2d getLocalGravity(olc::vi2d pos);
	bool tryPlace(ParticleState* particle, olc::vi2d newPos);
};

std::shared_ptr<Simulation> getSimulation();