#include "entities/ServerCore.h"
#include "managers/ResourceManager.h"
#include "util/Theme.h"
#include "util/ArtScale.h"

namespace {
    constexpr int   kServerFrames = 5;     // liczba klatek w texturze (poziomo)
    constexpr float kFrameTime    = 0.15f; // czas jednej klatki [s] (5 klatek -> ~0.75 s petla)
}

ServerCore::ServerCore(ResourceManager& res, sf::Vector2f position) {
    m_position = position;

    // Laduje trzy warianty wygladu rdzenia zalezne od poziomu zdrowia
    const char* files[kVariants] = {
        "assets/textures/server_core_1_animated.png", // niebieski - zdrowy
        "assets/textures/server_core_2_animated.png", // pomaranczowy - sredni
        "assets/textures/server_core_3_animated.png", // czerwony - krytyczny
    };
    for (int v = 0; v < kVariants; ++v) {
        m_sheets[v] = res.getTexture(files[v]);
        if (!m_sheets[v]) continue;

        m_hasTexture = true;
        auto ts = m_sheets[v]->getSize();

        m_frameW[v] = static_cast<int>(ts.x) / kServerFrames; // szerokosc jednej klatki
        m_frameH[v] = static_cast<int>(ts.y);
        m_sprites[v].setTexture(*m_sheets[v]);
        m_sprites[v].setTextureRect(sf::IntRect(0, 0, m_frameW[v], m_frameH[v]));
        m_sprites[v].setOrigin(m_frameW[v] / 2.f, m_frameH[v] / 2.f);
        m_sprites[v].setPosition(m_position);

        // Skaluje sprite do docelowej szerokosci ustawionej w ArtScale
        m_baseScale[v] = m_frameW[v] > 0 ? Art::kServerWidth / static_cast<float>(m_frameW[v]) : 1.f;

        // Ustawia logiczny rozmiar rdzenia na podstawie przeskalowanej tekstury
        m_size = {
            Art::kServerWidth,
            static_cast<float>(m_frameH[v]) * m_baseScale[v]
        };
    }
}

void ServerCore::update(float dt) {
    if (!m_hasTexture)
        return;

    // Przelacza klatki animacji w rownych odstepach czasu
    m_frameTimer += dt;

    while (m_frameTimer >= kFrameTime) {
        m_frameTimer -= kFrameTime;
        m_frame = (m_frame + 1) % kServerFrames;
    }
}

sf::FloatRect ServerCore::getBounds() const {
    return sf::FloatRect(m_position.x - m_size.x / 2.f, m_position.y - m_size.y / 2.f, m_size.x, m_size.y);
}

void ServerCore::draw(sf::RenderWindow& window) {
    float hp = m_maxHealth > 0 ? static_cast<float>(m_health) / m_maxHealth : 0.f;

    // Kolor paska zdrowia zmienia sie razem ze stanem rdzenia
    sf::Color healthColor = hp > 0.5f
        ? Theme::lerp(Theme::Warn, Theme::Ok, (hp - 0.5f) * 2.f)
        : Theme::lerp(Theme::Danger, Theme::Warn, hp * 2.f);

    // Rysuje wariant sprite'a odpowiedni dla aktualnego zdrowia
    if (m_hasTexture) {
        int variant = hp >= 0.60f ? 0 : (hp >= 0.25f ? 1 : 2);

        // Jesli wybrany wariant nie zostal zaladowany, szuka najblizszego dostepnego
        while (variant > 0 && !m_sheets[variant])
            --variant;

        while (!m_sheets[variant] && variant < kVariants - 1)
            ++variant;

        sf::Sprite& sprite = m_sprites[variant];

        // Ustawia aktualna klatke animacji ze sprite-sheetu
        sprite.setTextureRect(sf::IntRect(
            m_frame * m_frameW[variant],
            0,
            m_frameW[variant],
            m_frameH[variant]
            ));

        sprite.setScale(m_baseScale[variant], m_baseScale[variant]);

        window.draw(sprite);
    }

    // Rysuje pasek zdrowia nad rdzeniem
    const float barW = 110.f;
    const float barH = 8.f;

    sf::RectangleShape background({barW, barH});
    background.setOrigin(barW / 2.f, barH / 2.f);
    background.setPosition(m_position.x, m_position.y - m_size.y / 2.f - 16.f);
    background.setFillColor(sf::Color(0, 0, 0, 160));
    background.setOutlineThickness(1.f);
    background.setOutlineColor(sf::Color(60, 80, 100));
    window.draw(background);

    sf::RectangleShape fill({barW * hp, barH});
    fill.setOrigin(0.f, barH / 2.f);
    fill.setPosition(m_position.x - barW / 2.f, m_position.y - m_size.y / 2.f - 16.f);
    fill.setFillColor(healthColor);
    window.draw(fill);
}
void ServerCore::takeDamage(int damage) {
    if (damage <= 0 || m_health <= 0) return;

    m_health -= damage;
    m_hitFlash = 0.2f; // Serwer podświetli się na ułamek sekundy (masz to już zaimplementowane w draw!)

    if (m_health < 0) {
        m_health = 0;
    }
}

bool ServerCore::isDestroyed() const {
    return m_health <= 0;
}