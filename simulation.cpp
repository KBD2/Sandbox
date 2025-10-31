#include <vector>

#include "olcPixelGameEngine.h"
#include "sandbox.h"
#include "particles.h"
#include "simulation.h"

const olc::vf2d deltas[][2] = {
	{olc::vf2d(0, 1).polar(), olc::vf2d(0, -1).polar()},
	{olc::vf2d(-1, 1).polar(), olc::vf2d(-1, -1).polar()},
	{olc::vf2d(-1, 2).polar(), olc::vf2d(-1, -2).polar()}
};

void clip(olc::vi2d& pos) {
	pos.x = std::min(std::max(pos.x, 0), WIDTH - 1);
	pos.y = std::min(std::max(pos.y, 0), HEIGHT - 1);
}

void handleFriction(ParticleState& particle) {
	ParticleProperties& properties = propertyLookup[particle.type];
	particle.velocity *= properties.frictionCoeff;
}

Simulation::Simulation(area_t area) {
	this->area = area;
}

void Simulation::tick() {
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

void Simulation::updatePhysicalParticle(olc::vi2d pos) {
	ParticleState& particle = this->area[pos.y][pos.x];
	ParticleProperties& properties = propertyLookup[particle.type];
	olc::vf2d* velocity = &particle.velocity;

	olc::vf2d localGravity = getLocalGravity(pos);

	*velocity += localGravity * propertyLookup[particle.type].mass;
	olc::vf2d* delta = &particle.delta;
	*delta += *velocity;
	olc::vf2d toMove = (olc::vi2d) *delta;

	// TODO: Better liquid spreading, currently only spreads sideways when gravity would move it down
	/*if (properties.state == State::LIQUID && localGravity.mag() > 0) {
		olc::vf2d dir = localGravity.norm().polar();
		olc::vf2d brownian = olc::vf2d(0, random() * 0.2 - 0.1).polar();
		brownian.y += dir.y;
		std::cout << brownian.cart() << std::endl;
		*velocity += brownian.cart();
	}*/

	if (toMove.mag() > 0) {
		*delta -= toMove;

		olc::vi2d initial = pos + toMove;

		if (inBounds(initial) && get(initial)->type == ParticleType::NONE) {
			*get(initial) = *get(pos);
			resetParticle(pos);
			return;
		}

		handleFriction(particle);

		olc::vf2d dir = toMove.norm().polar();

		while (toMove.mag() >= 1) {
			olc::vi2d newPos = pos + toMove;

			if (inBounds(newPos) && get(newPos)->type == ParticleType::NONE) {
				*get(newPos) = *get(pos);
				resetParticle(pos);
				return;
			}

			olc::vf2d centre = olc::vf2d((float) newPos.x + 0.5, (float) newPos.y + 0.5);

			int layers = properties.state == State::SOLID ? 1 : 3;
			for (int i = 0; i < layers; i++) {
				const olc::vf2d* layer = deltas[i];
				bool choice = random() < 0.5;
				for (int i = 0; i < 2; i++) {
					olc::vf2d delta = choice ? layer[1] : layer[0];
					choice = !choice;
					delta.y += dir.y; // Transform to global
					olc::vi2d check = centre + delta.cart();
					if (inBounds(check) && get(check)->type == ParticleType::NONE) {
						*get(check) = *get(pos);
						resetParticle(pos);
						return;
					}
				}
			}
			toMove -= toMove.norm();
		}
		// It's just not able to move
		*velocity *= 0;
		*delta *= 0;
	}
}

olc::vf2d Simulation::getLocalGravity(olc::vi2d pos) {
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

ParticleState* Simulation::get(olc::vi2d pos) {
	return &this->area[pos.y][pos.x];
}

void Simulation::resetParticle(olc::vi2d pos) {
	ParticleState* particle = &this->area[pos.y][pos.x];
	particle->type = ParticleType::NONE;
	particle->velocity = olc::vf2d(0, 0);
	particle->delta = olc::vf2d(0, 0);
}