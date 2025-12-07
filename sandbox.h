#pragma once

#include <random>
#include <vector>

#include "olcPixelGameEngine.h"

static constexpr int WIDTH = 640;
static constexpr int HEIGHT = 480;
static constexpr int PIX_SIZE = 2;
static constexpr int PIX_X = WIDTH / PIX_SIZE;
static constexpr int PIX_Y = HEIGHT / PIX_SIZE;
static constexpr float PI = 3.141592f;
static constexpr float TICK_DURATION = 1.0f / 60.0f;
static constexpr int MAX_PARTS = 100000;

enum GravityType {
	VECTOR,
	RADIAL,
	OFF
};

typedef struct {
	GravityType gravType;
	olc::vf2d gravVec;
	bool ticking;
} Config;

extern Config CONFIG;

typedef enum {
	DUST,
	WATER,
	BRICK,
	ANAR,
	GAS,
	FIRE,
	NONE
} Type;

typedef struct {
	Type type;
	olc::vi2d pos;
	olc::vf2d velocity;
	olc::vf2d delta;
	olc::Pixel deco;
	int32_t data[10];
	bool dead;
} ParticleState;

typedef struct {
	olc::vi2d uiPos;
	Type type;
} UIParticleType;

typedef struct {
	std::vector<UIParticleType> types;
	Type selected;
} UIContext;

extern UIContext uiCtx;

typedef ParticleState*(*area_t)[PIX_X];

extern std::vector<ParticleState*> partArr;
extern area_t partGrid;


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
	return x >= 0 && x < PIX_X && y >= 0 && y < PIX_Y;
}

static bool inBounds(olc::vi2d pos) {
	return pos.x >= 0 && pos.x < PIX_X && pos.y >= 0 && pos.y < PIX_Y;
}