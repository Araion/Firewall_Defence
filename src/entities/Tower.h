#pragma once
#include "core/GameObject.h"
#include "util/ArtScale.h"
#include <memory>
#include <string>
#include <vector>

class ResourceManager;
class PlayState;
class Enemy;

// =============================================================
//  Tower - abstrakcyjna baza wszystkich wiez obronnych.
//  Odpowiada za: przeladowanie, wyszukanie celu (najdalej na
//  sciezce, w zasiegu), obrot lufy z predkoscia w stopniach/s oraz
//  ulepszanie i sprzedaz. Strzal definiuje klasa pochodna (attack()).
//  Cel to niewlasnosciowy wskaznik Enemy*, re-walidowany co klatke
//  z listy zywych wrogow (PlayState::enemies()).
// =============================================================
class Tower : public GameObject {
public:
    Tower(PlayState& state, ResourceManager& res);
    virtual ~Tower() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Rysowanie dwuwarstwowe: PlayState rysuje NAJPIERW wszystkie podstawy (drawBaseLayer),
    // a potem wszystkie glowki (drawHeadLayer), dzieki czemu glowka jednej wiezy nie jest
    // przykrywana przez podstawe sasiada postawionego pozniej. draw() = obie warstwy razem.
    virtual void drawBaseLayer(sf::RenderWindow& window); // podstawa + poswiata (+ pole wsparcia)
    virtual void drawHeadLayer(sf::RenderWindow& window); // glowka + wskazniki poziomu + status DDoS
    sf::FloatRect getBounds() const override;

    // Wykonanie strzalu do aktualnego celu (definiuje klasa pochodna).
    virtual void attack() = 0;

    // Ulepszenie wiezy (do 3 poziomow): rosna obrazenia/zasieg.
    void upgrade();
    bool canUpgrade() const { return m_level < kMaxLevel; }
    int  getUpgradeCost() const;
    int  getSellValue() const;       // ~70% wlozonych kredytow

    // Gettery do UI / zapisu
    int   getCost() const { return m_cost; }
    int   getCpuCost() const { return m_cpuCost; }
    int   getLevel() const { return m_level; }
    float getDamage() const { return m_damage; }
    float getRange() const { return m_range; }
    float getFireRate() const;       // strzaly/s (do wyswietlenia)
    bool  canShoot() const { return m_canShoot; }   // czy wieza w ogole strzela

    // Dodatkowa linia statystyk do panelu (np. "Przyspieszenie: +35%" dla Overclocka).
    virtual std::string statLine() const { return ""; }
    const std::string& displayName() const { return m_name; }
    sf::Color getColor() const { return m_color; }

    void setSelected(bool s) { m_selected = s; }

    // Mnoznik szybkostrzelnosci z aury OverclockTower (1 = brak). Resetowany co klatke.
    void setFireRateBoost(float b) { m_fireRateBoost = b; }
    void resetFireRateBoost() { m_fireRateBoost = 1.f; }
    float fireRateBoost() const { return m_fireRateBoost; }

    // Aura wsparcia (np. OverclockTower). Naliczana w OSOBNYM przebiegu PRZED
    // aktualizacja wiez, by dzialala niezaleznie od kolejnosci w kontenerze.
    virtual void applyAura(const std::vector<Tower*>& towers) { (void)towers; }

    // Status "DDoS" - wieza chwilowo wylaczona przez GlitchDrone.
    void disableFor(float seconds) { m_disabledTimer = seconds; }
    bool isDisabled() const { return m_disabledTimer > 0.f; }

    // Globalny mnoznik zasiegu z ulepszen systemowych (Draft).
    void setGlobalRangeMult(float m) { m_globalRangeMult = m; }
    float effectiveRange() const { return m_range * m_globalRangeMult; }

    // Do zapisu/wczytania: ustaw poziom wraz ze statystykami.
    void setLevelDirect(int level);

protected:
    static constexpr int kMaxLevel = 3;

    // Tryb ruchu glowki: Aim = obrot w strone celu, Bob = ruch gora-dol.
    enum class HeadMotion { Aim, Bob };

    Enemy* acquireTarget() const;
    void   rotateBarrelTowards(Enemy* target, float dt);
    sf::Vector2f barrelTip(float length) const;
    sf::Vector2f headPoint(float along, float perp) const;

    virtual void applyLevelStats();
    void   finalizeSetup();
    void   loadTowerArt(const std::string& key);

    PlayState& m_state;
    ResourceManager& m_res;

    // Ustawienia ulozenia grafiki wiezy
    sf::Vector2f m_headPivot{0.5f, 0.5f};
    sf::Vector2f m_headOffset{0.f, 0.f};
    float m_headScaleMult = 1.f;
    float m_baseSizePx = Art::kTowerBaseWidth;

    // Dzwiek strzalu (throttlowany)
    std::string m_shootSound;
    float m_shootSoundVol = 70.f;
    float m_shootSoundGap = 0.f;

    std::shared_ptr<sf::Texture> m_baseTex, m_headTex;
    sf::Sprite m_baseSprite, m_headSprite;
    bool m_hasArt = false;
    float m_artScale = 1.f;
    float m_headScale = 1.f;
    HeadMotion m_headMotion = HeadMotion::Aim;
    bool  m_canShoot = true;
    float m_bobAmp = 4.f;

    std::string m_name = "Tower";
    sf::Color m_color{200, 200, 200};

    // Statystyki bazowe (poziom 1)
    int   m_cost = 100;
    int   m_cpuCost = 8;
    float m_baseDamage = 20.f;
    float m_baseRange = 150.f;
    float m_fireCooldown = 0.75f;
    float m_rotationSpeed = 240.f;

    // Statystyki biezace (po uwzglednieniu poziomu)
    float m_damage = 20.f;
    float m_range = 150.f;

    int   m_level = 1;
    float m_globalRangeMult = 1.f;
    int   m_invested = 0;
    float m_cooldownTimer = 0.f;
    float m_shootSoundTimer = 0.f;
    float m_barrelAngle = 0.f;
    float m_fireRateBoost = 1.f;
    float m_disabledTimer = 0.f;
    float m_animTime = 0.f;
    bool  m_selected = false;

    Enemy* m_target = nullptr;

    // Pomocnicze rysowanie
    void drawRangeCircle(sf::RenderWindow& window) const;
    void drawBaseBody(sf::RenderWindow& window) const;
    virtual void drawBarrel(sf::RenderWindow& window) const;
};