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

// =============================================================
// PlayState - Główny stan rozgrywki.
// Obsługuje WaveManager (fale), ekonomię, limit CPU, temperaturę
// (przegrzanie), ulepszenia systemowe (Draft) oraz aktywne moce.
// =============================================================
class PlayState : public GameState {
public:
    explicit PlayState(Game& game);
    ~PlayState() override;

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // --- API dla obiektów gry ---
    void spawn(std::unique_ptr<GameObject> obj);
    void spawnExplosion(sf::Vector2f pos, sf::Color color, float scale = 1.f);
    const std::vector<const Path*>& paths() const { return m_pathPtrs; }
    const std::vector<Enemy*>& enemies() const { return m_frameEnemies; }
    const std::vector<Tower*>& towers() const { return m_frameTowers; }
    class ResourceManager& resources();

    // --- Ekonomia i Komunikaty ---
    void addCredits(int c) { m_credits += c; }
    bool spendCredits(int c);
    int  credits() const { return m_credits; }
    void addScore(int s) { m_score += s; }
    void showMessage(const std::string& msg, sf::Color color);
    void playSfx(const std::string& name, float volume = 100.f);
    void playMusic(const std::string& name);
    bool enemyAbilities() const { return m_enemyAbilities; }

    bool canPlaceAt(sf::Vector2f pos) const;
    Tower* towerAt(sf::Vector2f pos) const;

    // --- Modyfikatory (Draft) ---
    bool  heuristics() const { return m_heuristics; }
    float bountyMult() const { return m_bountyMult; }
    float heatReduction() const { return m_heatReduction; }
    float slowStrength() const { return m_slowStrength; }

    // --- Zapis / wczytanie stanu (SaveManager) ---
    struct TowerSave { std::string type; int level; float x, y; };
    struct EnemySave { std::string type; float hp; int pathIndex; float distance; };
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

    // SaveManager: buildBoard
    void buildBoard(unsigned seed, int difficulty);
    bool placeTowerFromSave(const std::string& type, int level, sf::Vector2f pos);
    void addEnemyFromSave(const std::string& type, float hp, int pathIndex, float distance);
    void applyLoadedUpgrades(float range, float bounty, float heat, bool heur, float slow, int cpuCap);

    // Przeciazenie dla SaveManagera
    void applyLoadedMeta(int waveNum, int credits, int serverHp, float heat, int scoreVal);
    void applyLoadedMeta(int credits, int serverHp, int scoreVal);
    void finishLoad();

    bool saveGame();
    bool loadGame();

    // --- Wektory Ataku (Breach) ---
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

    Tower* m_selectedTower = nullptr;
    sf::Vector2f m_mouse;

    std::string m_message;
    sf::Color m_messageColor{255, 255, 255};
    float m_messageTimer = 0.f;

    bool  m_paused = false;
    float m_speed = 1.f;
    float m_uiTime = 0.f;

    sf::Text m_txtWave, m_txtCredits, m_txtHealth, m_txtHint, m_txtScore;
    Button m_btnUpgrade, m_btnSell;
    Button m_btnSpeed, m_btnSaveGame, m_btnMenu;
};