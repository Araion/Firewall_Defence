#pragma once
#include "core/GameState.h"
#include "core/GameObject.h"
#include "util/Path.h"
#include "util/Button.h"
#include "util/LevelMap.h"
#include "managers/CollisionManager.h"
#include "entities/TowerType.h"
#include <vector>
#include <memory>
#include <string>

class Enemy;
class ServerCore;
class Tower;
class WaveManager;
class TutorialDirector;

// =============================================================
//  PlayState - wlasciwa rozgrywka. Trzyma JEDEN wspolny kontener
//  std::vector<std::unique_ptr<GameObject>> ze wszystkimi obiektami
//  (wieze, wrogowie, pociski, efekty, build pady, serwer). Petla
//  klatki aktualizuje je polimorficznie, rozwiazuje kolizje
//  (CollisionManager), usuwa martwe i rysuje plansze, HUD, sklep
//  oraz panel wiezy. Obejmuje ekonomie i limit CPU.
// =============================================================
class PlayState : public GameState {
public:
    explicit PlayState(Game& game, bool tutorial = false);
    ~PlayState() override; // zdefiniowany w .cpp (WaveManager jest tam kompletny)

    bool isTutorial() const { return m_tutorial; }

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // --- API dla obiektow gry ---
    void spawn(std::unique_ptr<GameObject> obj);          // dodanie obiektu (odlozone)
    void spawnExplosion(sf::Vector2f pos, sf::Color color, float scale = 1.f);
    const std::vector<const Path*>& paths() const { return m_pathPtrs; }
    const std::vector<Enemy*>& enemies() const { return m_frameEnemies; } // cache zywych wrogow
    const std::vector<Tower*>& towers() const { return m_frameTowers; }   // cache zywych wiez
    class ResourceManager& resources();

    // --- Ekonomia ---
    void addCredits(int c) { m_credits += c; }
    bool spendCredits(int c);
    int  credits() const { return m_credits; }
    void addScore(int s) { m_score += s; }
    void showMessage(const std::string& msg, sf::Color color);
    void playSfx(const std::string& name, float volume = 100.f);
    void playMusic(const std::string& name);
    bool enemyAbilities() const { return m_enemyAbilities; } // czy zdolnosci specjalne wlaczone

    // --- API dla samouczka (TutorialDirector) ---
    void unlockTower(int idx);                 // odblokowuje typ wiezy w sklepie
    void scriptOpenBreach() { openBreach(); }  // recznie otwiera tunel breach
    sf::Vector2f mousePos() const { return m_mouse; }
    // Czy w danym punkcie wolno postawic wieze (poza sciezka, poza innymi
    // wiezami, w polu gry). Uzywane przez budowanie i samouczek.
    bool canPlaceAt(sf::Vector2f pos) const;
    // Czy w poblizu punktu stoi juz jakas wieza (samouczek: zaliczenie kroku).
    bool towerNear(sf::Vector2f pos, float radius) const;
    // Bramka samouczka: ogranicza akcje gracza. target = wskazane miejsce budowy
    // (gdy hasTarget), radius = dozwolony promien wokol niego.
    void setTutorialGate(bool buildLocked, int onlySlot,
                         sf::Vector2f target = {}, bool hasTarget = false, float radius = 70.f);

    Tower* towerAt(sf::Vector2f pos) const;       // wieza pod kursorem (do wyboru)

    // --- Modyfikatory z ulepszen systemowych (System Update) ---
    bool  heuristics() const { return m_heuristics; }       // wykrywanie zaszyfrowanych
    float bountyMult() const { return m_bountyMult; }       // mnoznik kredytow za wrogow
    float heatReduction() const { return m_heatReduction; } // mnoznik generowanego heatu
    float slowStrength() const { return m_slowStrength; }   // sila spowolnienia firewalla

    // --- Zapis / wczytanie stanu (SaveManager) ---
    struct TowerSave { std::string type; int level; float x, y; };
    struct EnemySave { std::string type; float hp; int pathIndex; float distance; int gen = 0; };
    struct BreachSave { int index; int wavesLeft; };
    std::vector<TowerSave> snapshotTowers() const;
    std::vector<EnemySave> snapshotEnemies() const;
    std::vector<BreachSave> snapshotBreaches() const;
    void activateBreachFromSave(int index, int wavesLeft);
    int   wave() const { return m_wave; }
    int   serverHealth() const { return m_serverHealth; }
    float heat() const { return m_heat; }
    int   score() const { return m_score; }
    int   pathCount() const { return static_cast<int>(m_pathPtrs.size()); }
    float globalRangeMult() const { return m_globalRangeMult; }
    int   cpuCapacity() const { return m_cpuCapacity; }
    unsigned mapSeed() const { return m_mapSeed; }
    int   difficultyLevel() const { return m_difficultyLevel; }

    void  applyLoadedUpgrades(float range, float bounty, float heat, bool heur,
                             float slow, int cpuCap);

    void buildBoard(unsigned seed, int difficulty); // dopasowane do M3
    bool placeTowerFromSave(const std::string& type, int level, sf::Vector2f pos);
    void addEnemyFromSave(const std::string& type, float hp, int pathIndex, float distance, int gen = 0);
    void applyLoadedMeta(int waveNum, int credits, int serverHp, float heat, int scoreVal);
    void applyLoadedMeta(int credits, int serverHp, int scoreVal); // przeciazenie dla starego SaveManager
    void finishLoad();

    bool saveGame();
    bool loadGame();

    // Dla WaveManager
    bool anyBreachActive() const;
    bool isBreachPath(const Path* p) const;
    bool breachWormOnly() const { return m_breachWormOnly; }

private:
    void buildBoardFromSeed(unsigned seed);

    void initAbilities();
    void drawAbilities(sf::RenderWindow& window);
    int  abilityAt(sf::Vector2f pos) const;
    void activateAbility(int idx);

    void rebuildCaches();
    void updateHeat(float dt);
    void updateHover();
    void handleDeaths();
    void removeDead();
    void flushPending();

    void onLeftClick(sf::Vector2f mouse);
    sf::FloatRect shopSlotRect(int i) const;
    void selectTower(Tower* t);
    void deselectAll();
    void setBuildSelection(int idx);
    bool tryBuildAt(TowerType type, sf::Vector2f pos);
    void sellSelected();
    void upgradeSelected();
    std::unique_ptr<Tower> createTower(TowerType type);
    int  pathIndexOf(const Path* p) const;

    void onWaveChanged(int newWave);
    void maybeTriggerDraft(int completedWave);
    void openDraft();
    void applyUpgrade(int card);
    void drawDraft(sf::RenderWindow& window);
    void openBreach();
    void tickBreaches();

    void drawBackground(sf::RenderWindow& window);
    void drawPaths(sf::RenderWindow& window);
    void drawHud(sf::RenderWindow& window);
    void drawBar(sf::RenderWindow& window, float x, float y, float w, float h,
                 float frac, sf::Color color, const std::string& label);
    void drawShop(sf::RenderWindow& window);
    void drawTowerPanel(sf::RenderWindow& window);
    void drawMessage(sf::RenderWindow& window);
    void drawControls(sf::RenderWindow& window);
    void drawPauseOverlay(sf::RenderWindow& window);
    void drawTutorialHints(sf::RenderWindow& window);
    void drawBuildPreview(sf::RenderWindow& window);
    static void drawThickLine(sf::RenderWindow& window, sf::Vector2f a, sf::Vector2f b,
                              float thickness, sf::Color color);

    static constexpr float kTowerFootprint = 22.f;

    std::vector<std::unique_ptr<Path>> m_paths;
    std::vector<const Path*> m_pathPtrs;
    std::vector<std::unique_ptr<GameObject>> m_objects;
    std::vector<std::unique_ptr<GameObject>> m_pendingSpawns;
    ServerCore* m_server = nullptr;
    std::vector<Enemy*> m_frameEnemies;
    std::vector<Tower*> m_frameTowers;

    CollisionManager m_collision;
    std::unique_ptr<WaveManager> m_waves;
    std::unique_ptr<TutorialDirector> m_tutorialDirector;

    int m_serverHealth = 20;
    int m_serverMaxHealth = 20;
    int m_credits = 300;
    int m_score = 0;
    int m_wave = 0;
    int m_totalWaves = 20;

    int   m_cpuUsage = 0;
    int   m_cpuCapacity = 100;
    int   m_towerCostPercent = 100;
    float m_heat = 0.f;
    float m_heatSoftThreshold = 0.7f;
    float m_overclockHeatPerSec = 8.f;
    float m_heatFreezeTimer = 0.f;
    bool  m_enemyAbilities = true;

    float m_bountyMult = 1.f;
    float m_heatReduction = 1.f;
    bool  m_heuristics = false;
    float m_globalRangeMult = 1.f;
    float m_slowStrength = 0.35f;

    bool  m_draftActive = false;
    std::vector<int> m_draftCards;
    int   m_systemUpgradeInterval = 5;

    struct BreachLane {
        std::unique_ptr<Path> path;
        bool active = false;
        int  wavesLeft = 0;
    };
    std::vector<BreachLane> m_breaches;
    bool  m_breachWormOnly = false;
    float m_breachChance = 0.25f;

    bool     m_tutorial = false;
    unsigned m_mapSeed = 0;
    int      m_difficultyLevel = 1;

    struct Ability {
        std::string name;
        int kind = 0;
        sf::Vector2f pos;
        float radius = 30.f;
        float cooldown = 0.f;
        float cooldownMax = 45.f;
        sf::Color color;
    };
    std::vector<Ability> m_abilities;
    bool m_abilitiesEnabled = true;

    bool m_towerUnlocked[static_cast<int>(TowerType::Count)];
    int  m_buildSelection = -1;
    std::unique_ptr<Tower> m_preview;

    bool m_tutBuildLocked = false;
    int  m_tutOnlySlot = -1;
    bool m_tutHasTarget = false;
    sf::Vector2f m_tutTarget;
    float m_tutTargetRadius = 70.f;
    Tower* m_selectedTower = nullptr;
    sf::Vector2f m_mouse;

    std::string m_message;
    sf::Color m_messageColor{255, 255, 255};
    float m_messageTimer = 0.f;

    bool  m_paused = false;
    float m_speed = 1.f;
    float m_uiTime = 0.f;
    float m_devTutTimer = 0.f;

    sf::Text m_txtWave, m_txtCredits, m_txtHealth, m_txtHint, m_txtScore;
    Button m_btnUpgrade, m_btnSell;
    Button m_btnSpeed, m_btnSaveGame, m_btnMenu;
};