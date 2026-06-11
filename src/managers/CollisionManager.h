#pragma once
#include <vector>
#include <memory>

class GameObject;
class PlayState;

// =============================================================
// CollisionManager - wykrywa kolizje pociskow z przeciwnikami
// Przy trafieniu wywoluje reakcje pocisku na trafienie
// =============================================================
class CollisionManager {
public:
    void resolve(PlayState& state, std::vector<std::unique_ptr<GameObject>>& objects);
};