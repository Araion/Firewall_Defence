#include "entities/VirusEnemy.h"
#include <cmath>

VirusEnemy::VirusEnemy(ResourceManager& res, const Path* path)
    : Enemy(res, "assets/textures/virus.png", path, 15.f, 50.f)
{
    m_bodyColor = Theme::NeonMagenta;
    m_bodyRadius = 14.f;
    m_serverDamage = 1; // Ile HP zabiera serwerowi po dotarciu na koniec
}