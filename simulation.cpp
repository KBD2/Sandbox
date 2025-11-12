#include <vector>

#include "olcPixelGameEngine.h"
#include "sandbox.h"
#include "particles.h"
#include "simulation.h"

const olc::vf2d deltasSolid[][2] = {
	{olc::vf2d(0, 1).polar(), olc::vf2d(0, -1).polar()}
};

const olc::vf2d deltasLiquid[][2] = {
	{olc::vf2d(0, 1).polar(), olc::vf2d(0, -1).polar()},
	{olc::vf2d(-1, 1).polar(), olc::vf2d(-1, -1).polar()},
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
	this->updated = new bool[HEIGHT][WIDTH];
	this->area = area;
}

void Simulation::tick() {

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			this->updated[y][x] = false;
		}
	}

	std::vector<olc::vi2d> toUpdate = std::vector<olc::vi2d>();
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if (this->area[y][x].type != ParticleType::NONE) {
				toUpdate.push_back(olc::vi2d(x, y));
			}
		}
	}
	for (olc::vi2d& pos : toUpdate) {
		if (this->updated[pos.y][pos.x]) continue;
		ParticleState& particle = this->area[pos.y][pos.x];
		ParticleProperties& properties = propertyLookup[particle.type];
		updatePhysicalParticle(pos);
	}
}

bool Simulation::tryPlace(olc::vi2d pos, olc::vi2d newPos) {
	ParticleState* state = get(pos);
	ParticleProperties& props = propertyLookup[state->type];

	if (inBounds(newPos)) {
		ParticleState* newState = get(newPos);
		bool canSwap = false;
		if (newState->type == ParticleType::NONE) canSwap = true;
		else {
			ParticleProperties& newProps = propertyLookup[newState->type];
			if (props.state == State::SOLID && newProps.state == State::LIQUID) {
				canSwap = true;
			}
		}
		if (canSwap) {
			if (newState->type == ParticleType::NONE) {
				*newState = *state;
				resetParticle(pos);
			} else {
				ParticleState temp = *newState;
				*newState = *state;
				*state = temp;
			}
			this->updated[newPos.y][newPos.x] = true;
			return true;
		}
		return false;
	}
	return false;
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

	if (toMove.mag() > 0) {
		*delta -= toMove;

		olc::vi2d initial = pos + toMove;

		if (tryPlace(pos, initial)) return;

		handleFriction(particle);

		olc::vf2d dir = toMove.norm().polar();

		while (toMove.mag() >= 1) {
			olc::vi2d newPos = pos + toMove;

			if (tryPlace(pos, newPos)) return;

			olc::vf2d centre = olc::vf2d((float) newPos.x + 0.5, (float) newPos.y + 0.5);

			int layers = properties.state == State::SOLID ? 1 : 2;
			auto deltas = properties.state == State::SOLID ? deltasSolid : deltasLiquid;
			for (int i = 0; i < layers; i++) {
				const olc::vf2d* layer = deltas[i];
				bool choice = random() < 0.5;
				for (int i = 0; i < 2; i++) {
					olc::vf2d delta = choice ? layer[1] : layer[0];
					choice = !choice;
					delta.y += dir.y; // Transform to global
					olc::vi2d check = centre + delta.cart();
					if (tryPlace(pos, check)) return;
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
	particle->deco = olc::Pixel(0, 0, 0, 0);
}