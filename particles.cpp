#include <map>

#include "particles.h"

std::map<ParticleType, std::shared_ptr<ParticleProperties>> propertyLookup {
	{ ParticleType::DUST, std::make_shared<ParticleDust>() },
	{ ParticleType::WATER, std::make_shared<ParticleWater>() },
	{ ParticleType::BRICK, std::make_shared<ParticleBrick>() },
	{ ParticleType::ANAR, std::make_shared<ParticleAnar>() }
};