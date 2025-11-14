#include <map>

#include "particles.h"

std::map<ParticleType, ParticleProperties> propertyLookup {
	{ ParticleType::DUST, ParticleDust() },
	{ ParticleType::WATER, ParticleWater() },
	{ ParticleType::BRICK, ParticleBrick() }
};