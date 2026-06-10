#pragma once
#include "entities/Enemy.h"
#include <string>

class ResourceManager;
class Path;

class VirusEnemy : public Enemy {
public:
    VirusEnemy(ResourceManager& res, const Path* path);

    std::string getTypeName() const override { return "VirusEnemy"; }
};
