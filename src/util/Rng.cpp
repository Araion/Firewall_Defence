#include "util/Rng.h"
#include <algorithm>

std::mt19937 Rng::s_engine{std::random_device{}()};

void Rng::seed(unsigned s) {
    s_engine.seed(s);
}

void Rng::seedRandom() {
    std::random_device rd;
    s_engine.seed(rd());
}

int Rng::range(int min, int max) {
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(s_engine);
}

float Rng::rangef(float min, float max) {
    if (min > max) std::swap(min, max);
    std::uniform_real_distribution<float> dist(min, max);
    return dist(s_engine);
}

bool Rng::chance(float p) {
    if (p <= 0.f) return false;
    if (p >= 1.f) return true;
    std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(s_engine) < p;
}
