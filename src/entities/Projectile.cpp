#include "entities/Projectile.h"
#include "entities/Enemy.h"

void Projectile::onHit(Enemy& enemy, PlayState& state) {
    (void)state;
    // zadaj obrazenia i znikaj
    enemy.takeDamage(m_damage);
    kill();
}
