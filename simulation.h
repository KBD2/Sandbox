#pragma once

#include "sandbox.h"

class Simulation {
	area_t area;
public:
	Simulation(area_t area);
	void tick();
	void resetParticle(olc::vi2d pos);

private:
	void updatePhysicalParticle(olc::vi2d pos);
	olc::vf2d getLocalGravity(olc::vi2d pos);
	ParticleState* get(olc::vi2d pos);
};