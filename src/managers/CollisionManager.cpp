#include "managers/CollisionManager.h"
#include "core/GameObject.h"
#include "entities/Projectile.h"
#include "entities/Enemy.h"
#include <vector>

void CollisionManager::resolve(PlayState& state,
                               std::vector<std::unique_ptr<GameObject>>& objects) {
    std::vector<Projectile*> projectiles;
    std::vector<Enemy*> enemies;

    // Tworzy listy obiektow bioracych udzial w kolizjach
    for (auto& object : objects) {
        if (!object->isAlive())
            continue;

        if (Projectile* projectile = dynamic_cast<Projectile*>(object.get()))
            projectiles.push_back(projectile);
        else if (Enemy* enemy = dynamic_cast<Enemy*>(object.get()))
            enemies.push_back(enemy);
    }

    // Sprawdza trafienia pociskow w przeciwnikow
    for (Projectile* projectile : projectiles) {
        if (!projectile->isAlive())
            continue;

        sf::FloatRect projectileBounds = projectile->getBounds();

        for (Enemy* enemy : enemies) {
            if (!enemy->isAlive())
                continue;

            if (!projectile->canHit(*enemy))
                continue;

            if (projectileBounds.intersects(enemy->getBounds())) {
                projectile->onHit(*enemy, state);

                if (!projectile->isAlive())
                    break;
            }
        }
    }
}