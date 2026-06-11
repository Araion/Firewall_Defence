#include "entities/ExplosionEffect.h"
#include "util/Rng.h"
#include <cmath>
#include <algorithm>

namespace {
constexpr float kExplosionLife = 0.55f;
constexpr int kParticleCount = 16;
}

ExplosionEffect::ExplosionEffect(sf::Vector2f pos, sf::Color color, float scale)
    : Effect(pos, kExplosionLife), m_color(color), m_scale(scale) {
    m_particles.reserve(kParticleCount);

    // Tworzy czasteczki rozlatujace sie od srodka eksplozji
    for (int i = 0; i < kParticleCount; ++i) {
        float angle = Rng::rangef(0.f, 2.f * 3.14159265f);
        float speed = Rng::rangef(60.f, 220.f) * scale;

        Particle particle;
        particle.pos = pos;
        particle.vel = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        particle.size = Rng::rangef(2.f, 5.f) * scale;

        m_particles.push_back(particle);
    }
}

void ExplosionEffect::update(float dt) {
    advance(dt);

    float friction = std::max(0.f, 1.f - 4.f * dt);

    // Spowalnia czasteczki w trakcie zanikania eksplozji
    for (auto& particle : m_particles) {
        particle.pos += particle.vel * dt;
        particle.vel *= friction;
    }
}

void ExplosionEffect::draw(sf::RenderWindow& window) {
    float t = progress();

    // Krotki jasny blysk na poczatku eksplozji
    if (t < 0.4f) {
        float flashRadius = 18.f * m_scale * (1.f - t / 0.4f);

        sf::CircleShape flash(flashRadius);
        flash.setOrigin(flashRadius, flashRadius);
        flash.setPosition(m_position);
        flash.setFillColor(sf::Color(
            255,
            255,
            255,
            static_cast<sf::Uint8>(180 * (1.f - t / 0.4f))
            ));

        window.draw(flash);
    }

    // Rysuje zanikajace czasteczki eksplozji
    sf::Uint8 alpha = static_cast<sf::Uint8>(255 * (1.f - t));

    for (const auto& particle : m_particles) {
        float size = particle.size * (1.f - 0.5f * t);

        sf::CircleShape circle(size);
        circle.setOrigin(size, size);
        circle.setPosition(particle.pos);
        circle.setFillColor(sf::Color(m_color.r, m_color.g, m_color.b, alpha));

        window.draw(circle);
    }
}