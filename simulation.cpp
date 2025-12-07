#include <vector>

#include "olcPixelGameEngine.h"
#include "sandbox.h"
#include "particles.h"
#include "simulation.h"

void clip(olc::vi2d& pos) {
	pos.x = std::min(std::max(pos.x, 0), WIDTH - 1);
	pos.y = std::min(std::max(pos.y, 0), HEIGHT - 1);
}

void handleFriction(ParticleState* particle) {
	std::shared_ptr<ParticleProperties> properties = propertyLookup[particle->type];
	particle->velocity *= properties->frictionCoeff;
}

void Simulation::tick() {
	std::vector<ParticleState*> toUpdate = partArr;
	for (ParticleState* particle : toUpdate) {
		if (particle->dead) continue;
		std::shared_ptr<ParticleProperties> properties = getProps(particle->type);
		switch (properties->state) {
		case State::S_POWDER:
			updatePowder(particle);
			break;
		case State::S_LIQUID:
			updateLiquid(particle);
			break;
		case State::S_GAS:
			updateGas(particle);
			break;
		}
		properties->update(particle);
	}
	for (int i = 0; i < partArr.size(); i++) {
		ParticleState* particle = partArr[i];
		if (particle->dead) {
			partArr.erase(partArr.begin() + i--);
		}
	}
}

bool Simulation::tryPlace(ParticleState* particle, olc::vi2d newPos) {
	if (particle->pos == newPos) return false;
	std::shared_ptr<ParticleProperties> props = getProps(particle->type);

	if (inBounds(newPos)) {
		ParticleState* newParticle = partGrid[newPos.y][newPos.x];
		bool canSwap = false;
		if (newParticle == nullptr) {
			canSwap = true;
		} else {
			std::shared_ptr<ParticleProperties> newProps = getProps(newParticle->type);
			if (props->state < newProps->state) {
				canSwap = true;
			}
		}
		if (canSwap) {
			if (newParticle == nullptr) {
				partGrid[newPos.y][newPos.x] = particle;
				partGrid[particle->pos.y][particle->pos.x] = nullptr;
				particle->pos = newPos;
			} else {
				partGrid[newPos.y][newPos.x] = particle;
				partGrid[particle->pos.y][particle->pos.x] = newParticle;
				newParticle->pos = particle->pos;
				particle->pos = newPos;
			}
			return true;
		}
		return false;
	}
	return false;
}

void Simulation::updatePhysicsParticle(ParticleState* particle) {
	std::shared_ptr<ParticleProperties> properties = getProps(particle->type);

	particle->velocity += getLocalGravity(particle->pos) * getProps(particle->type)->mass;

	particle->delta += particle->velocity;
	olc::vf2d toMove = (olc::vi2d)particle->delta;
	olc::vf2d norm = toMove.norm();

	if (toMove.mag() > 0) {
		particle->delta -= toMove;

		olc::vi2d initial = particle->pos + toMove;

		if (tryPlace(particle, initial)) return;

		handleFriction(particle);

		olc::vf2d dir = particle->velocity.norm().polar();

		while (toMove.mag() >= 1) {
			toMove -= norm;

			olc::vi2d nextPos = particle->pos + toMove;

			if (tryPlace(particle, nextPos)) return;

			olc::vf2d centre = olc::vf2d((float)nextPos.x + 0.5, (float)nextPos.y + 0.5);
			
			if (properties->reposeAngle < 90) {
				for (int angle = 15; angle <= (90 - properties->reposeAngle); angle += 15) {
					bool choice = random() < 0.5;
					for (int i = 0; i < 2; i++) {
						if (random() < 0.2) continue; // Makes it not so uniform, stops some weird behaviour
						olc::vf2d sideDelta = olc::vf2d(1.42f, dir.y + toRads(choice ? angle : -angle));
						olc::vi2d check = centre + sideDelta.cart();
						if (tryPlace(particle, check)) {
							return;
						}
						choice = !choice;
					}
				}
			}
		}
		// It's just not able to move
		particle->velocity *= 0;
		particle->delta *= 0;
	}
}

void Simulation::updatePowder(ParticleState* particle) {
	return updatePhysicsParticle(particle);
}

void Simulation::updateLiquid(ParticleState* particle) {
	return updatePhysicsParticle(particle);
}

void Simulation::updateGas(ParticleState* particle) {
	updatePhysicsParticle(particle);

	if (random() < getProps(particle->type)->dispersion) {
		olc::vi2d valid[8];
		int numValid = 0;
		for (int dy = -1; dy <= 1; dy++) {
			for (int dx = -1; dx <= 1; dx++) {
				if (dx == 0 && dy == 0) continue;
				olc::vi2d checkPos = particle->pos + olc::vi2d(dx, dy);
				if (inBounds(checkPos)) {
					ParticleState* state = partGrid[checkPos.y][checkPos.x];
					if (state == nullptr) {
						valid[numValid++] = checkPos;
					}
				}
			}
		}
		if (numValid > 0) {
			olc::vi2d chosen = valid[rand() % numValid];
			tryPlace(particle, chosen);
		}
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

void Simulation::remove(ParticleState* state) {
	olc::vi2d pos = state->pos;
	partGrid[pos.y][pos.x] = nullptr;
	for (int i = 0; i < partArr.size(); i++) {
		if (state == partArr[i]) {
			partArr.erase(partArr.begin() + i);
			break;
		}
	}
	state->dead = true;
	for (int i = (int) partArr.size() - 1; i >= 0; i--) {
		if (partArr[i]->pos == pos) {
			partGrid[pos.y][pos.x] = partArr[i];
			break;
		}
	}
	state->dead = true;
}

ParticleState* Simulation::add(olc::vi2d pos, Type type) {
	for (int i = 0; i < MAX_PARTS; i++) {
		if (this->particlePool[i].dead) {
			ParticleState* state = &this->particlePool[i];
			state->type = type;
			state->deco = olc::Pixel(0, 0, 0, 0);
			state->pos = pos;
			state->dead = false;
			state->velocity = olc::vf2d();
			state->delta = olc::vf2d();
			for (int i = 0; i < 10; i++) {
				state->data[i] = 0;
			}
			getProps(type)->init(state);
			partArr.push_back(state);
			partGrid[pos.y][pos.x] = state;
			return state;
		}
	}
	return nullptr;
}