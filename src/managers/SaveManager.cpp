#include "managers/SaveManager.h"
#include "states/PlayState.h"
#include "util/Paths.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

bool SaveManager::save(PlayState& state, const std::string& relativePath) {
    std::ofstream out(Paths::resolve(relativePath));
    if (!out.is_open()) return false;

    // Sekcja meta - liczby potrzebne do odtworzenia rozgrywki
    out << "[meta]\n";
    out << "credits=" << state.credits() << "\n";
    out << "score=" << state.score() << "\n";
    out << "serverHealth=" << state.serverHealth() << "\n";
    out << "mapSeed=" << state.mapSeed() << "\n";
    out << "difficulty=" << state.difficultyLevel() << "\n";

    // Wieze: typ poziom x y
    out << "[towers]\n";
    for (const auto& t : state.snapshotTowers())
        out << t.type << " " << t.level << " " << t.x << " " << t.y << "\n";

    // Wrogowie: typ hp pathIndex distance
    out << "[enemies]\n";
    for (const auto& e : state.snapshotEnemies())
        out << e.type << " " << e.hp << " " << e.pathIndex << " " << e.distance << "\n";

    return true;
}

bool SaveManager::load(PlayState& state, const std::string& relativePath) {
    std::ifstream in(Paths::resolve(relativePath));
    if (!in.is_open()) return false;

    int credits = 300, score = 0, serverHp = 20, difficulty = 1;
    unsigned mapSeed = 0;

    struct TowerRec { std::string type; int level; float x, y; };
    struct EnemyRec { std::string type; float hp; int pathIndex; float distance; };
    std::vector<TowerRec> towers;
    std::vector<EnemyRec> enemies;

    std::string section, line;
    bool any = false;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') { section = line; any = true; continue; }

        if (section == "[meta]") {
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq), val = line.substr(eq + 1);
            try {
                if (key == "credits") credits = std::stoi(val);
                else if (key == "score") score = std::stoi(val);
                else if (key == "serverHealth") serverHp = std::stoi(val);
                else if (key == "mapSeed") mapSeed = static_cast<unsigned>(std::stoul(val));
                else if (key == "difficulty") difficulty = std::stoi(val);
            } catch (...) { /* uszkodzone linie ignorujemy */ }
        } else if (section == "[towers]") {
            std::stringstream ss(line);
            TowerRec r;
            if (ss >> r.type >> r.level >> r.x >> r.y) towers.push_back(r);
        } else if (section == "[enemies]") {
            std::stringstream ss(line);
            EnemyRec r;
            if (ss >> r.type >> r.hp >> r.pathIndex >> r.distance) enemies.push_back(r);
        }
    }

    if (!any) { std::cout << "[SaveManager] Pusty/niepoprawny plik zapisu\n"; return false; }

    // Najpierw odbuduj plansze z zapisanego ziarna (te same sciezki), potem meta i obiekty
    state.buildBoard(mapSeed, difficulty);
    state.applyLoadedMeta(credits, serverHp, score);
    for (const auto& t : towers) state.placeTowerFromSave(t.type, t.level, {t.x, t.y});
    for (const auto& e : enemies) state.addEnemyFromSave(e.type, e.hp, e.pathIndex, e.distance);
    state.finishLoad();
    return true;
}
