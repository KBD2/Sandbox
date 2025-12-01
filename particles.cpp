#include <map>

#include "particles.h"

std::map<Type, std::shared_ptr<ParticleProperties>> propertyLookup {
	{ Type::DUST, std::make_shared<ParticleDust>() },
	{ Type::WATER, std::make_shared<ParticleWater>() },
	{ Type::BRICK, std::make_shared<ParticleBrick>() },
	{ Type::ANAR, std::make_shared<ParticleAnar>() },
	{ Type::GAS, std::make_shared<ParticleGas>() },
	{ Type::FIRE, std::make_shared<ParticleFire>() }
};