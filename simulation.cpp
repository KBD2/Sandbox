#include <vector>

#include "olcPixelGameEngine.h"
#include "sandbox.h"
#include "particles.h"
#include "simulation.h"

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
		ParticleProperties& properties = getProps(particle.type);
		if (properties.state != State::SOLID) {
			olc::vi2d placed = updateMovableParticle(pos);
		}
	}
}

bool Simulation::tryPlace(olc::vi2d pos, olc::vi2d newPos) {
	if (pos == newPos) return false;
	ParticleState* state = get(pos);
	ParticleProperties& props = getProps(state->type);

	if (inBounds(newPos)) {
		ParticleState* newState = get(newPos);
		bool canSwap = false;
		if (newState->type == ParticleType::NONE) canSwap = true;
		else {
			ParticleProperties& newProps = getProps(newState->type);
			if (props.state < newProps.state) {
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

olc::vi2d Simulation::updateMovableParticle(olc::vi2d pos) {
	ParticleState& particle = this->area[pos.y][pos.x];
	ParticleProperties& properties = getProps(particle.type);
	olc::vf2d* velocity = &particle.velocity;

	olc::vf2d localGravity = getLocalGravity(pos);

	*velocity += localGravity * getProps(particle.type).mass;
	olc::vf2d* delta = &particle.delta;
	*delta += *velocity;
	olc::vf2d toMove = (olc::vi2d) *delta;

	if (toMove.mag() > 0) {
		*delta -= toMove;

		olc::vi2d initial = pos + toMove;

		if (tryPlace(pos, initial)) return initial;

		handleFriction(particle);

		olc::vf2d dir = toMove.norm().polar();

		while (toMove.mag() >= 1) {
			toMove -= toMove.norm();

			olc::vi2d nextPos = pos + toMove;

			if (tryPlace(pos, nextPos)) return nextPos;

			olc::vf2d centre = olc::vf2d((float) nextPos.x + 0.5, (float) nextPos.y + 0.5);

			int sideSpread = properties.state == State::POWDER ? 45 : 90;
			for (int angle = 0; angle <= sideSpread; angle += 5) {
				bool choice = rand() % 2 == 0;
				for (int i = 0; i < 2; i++) {
					if (rand() % 5 == 0) continue; // Makes it not so uniform, stops some weird behaviour
					olc::vf2d sideDelta = olc::vf2d(0.8f, dir.y + toRads(choice ? angle : -angle));
					olc::vi2d check = centre + sideDelta.cart();
					if (tryPlace(pos, check)) {
						olc::vf2d velocityPolar = (*velocity).polar();
						velocityPolar.y = sideDelta.polar().y;
						*velocity = velocityPolar.cart();
						return check;
					}
					choice = !choice;
				}
			}
		}
		// It's just not able to move
		*velocity *= 0;
		*delta *= 0;
	}

	return pos;
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