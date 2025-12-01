#pragma once

#include "sandbox.h"

class Simulation {
	area_t area;
	bool (*updated)[WIDTH];
public:
	Simulation(area_t area);
	void tick();
	void resetParticle(olc::vi2d pos);
	void setParticle(olc::vi2d pos, Type type);

private:
	olc::vi2d updatePhysicsParticle(olc::vi2d pos);
	olc::vi2d updatePowder(olc::vi2d pos);
	olc::vi2d updateLiquid(olc::vi2d pos);
	olc::vi2d updateGas(olc::vi2d pos);
	olc::vf2d getLocalGravity(olc::vi2d pos);
	ParticleState* get(olc::vi2d pos);
	bool tryPlace(olc::vi2d pos, olc::vi2d newPos);
};

std::shared_ptr<Simulation> getSimulation();