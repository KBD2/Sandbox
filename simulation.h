#pragma once

#include "sandbox.h"

class Simulation {
	area_t area;
	bool (*updated)[WIDTH];
public:
	Simulation(area_t area);
	void tick();
	void resetParticle(olc::vi2d pos);

private:
	olc::vi2d updateMovableParticle(olc::vi2d pos);
	olc::vf2d getLocalGravity(olc::vi2d pos);
	ParticleState* get(olc::vi2d pos);
	bool tryPlace(olc::vi2d pos, olc::vi2d newPos);
	bool surrounded(olc::vi2d pos);
};