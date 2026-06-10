#pragma once
#include "entities/Enemy.h"
#include <string>

// =============================================================
//  VirusEnemy - podstawowy, slaby wrog. Maly i szybki, bez
//  zdolnosci specjalnych. Statystyki: HP 35, 90 px/s, 10 kredytow,
//  10 punktow (zgodnie z tabela konceptu).
// =============================================================
class ResourceManager;
class Path;

class VirusEnemy : public Enemy {
public:
    VirusEnemy(ResourceManager& res, const Path* path);

    std::string getTypeName() const override { return "VirusEnemy"; }
};
