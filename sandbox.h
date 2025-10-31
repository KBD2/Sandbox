#pragma once

#include <random>

#include "olcPixelGameEngine.h"

static constexpr int WIDTH = 320;
static constexpr int HEIGHT = 240;
static constexpr float PI = 3.141592f;
static float TICK_DURATION = 1.0f / 60.0f;

enum GravityType {
	VECTOR,
	RADIAL,
	OFF
};

typedef struct {
	GravityType gravType;
	olc::vf2d gravVec;
} Config;

extern Config CONFIG;

typedef enum {
	NONE,
	DUST,
	WATER
} ParticleType;

typedef struct {
	ParticleType type;
	olc::vf2d velocity;
	olc::vf2d delta;
} ParticleState;

typedef ParticleState(*area_t)[WIDTH];

static float random() {
	static std::random_device rd;
	static std::mt19937 e2(rd());
	static std::uniform_real_distribution<> dist(0, 1);
	return dist(e2);
}

static inline float toRads(float degrees) {
	return degrees * (PI / 180);
}

static bool inBounds(int x, int y) {
	return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

static bool inBounds(olc::vi2d pos) {
	return pos.x >= 0 && pos.x < WIDTH && pos.y >= 0 && pos.y < HEIGHT;
}