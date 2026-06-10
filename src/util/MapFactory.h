#pragma once
#include "util/LevelMap.h"

namespace MapFactory {
LevelMap generate(int difficulty, unsigned seed); // difficulty: 0=latwy,1=normalny,2=trudny
}