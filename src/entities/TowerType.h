#pragma once
#include <SFML/Graphics/Color.hpp>

// =============================================================
// TowerType - dostepne typy wiez
// Metadane wiez sa uzywane przez sklep i tworzenie obiektow wiez
// =============================================================
enum class TowerType { Antivirus, Firewall, Laser, DataCleaner, Overclock, Corruption, EMP, Count };

struct TowerMeta {
    const char* name;          // pelna nazwa
    const char* shortName;     // etykieta w sklepie
    const char* configCostKey; // klucz kosztu w config.txt
    int defaultCost;           // koszt domyslny (gdy brak w configu)
    sf::Color color;           // kolor motywu wiezy
};

inline const TowerMeta& towerMeta(TowerType t) {
    static const TowerMeta metas[] = {
        {"Antivirus",    "ANTIVIRUS", "antivirusTowerCost",   100, sf::Color(57, 255, 136)},
        {"Firewall",     "FIREWALL",  "firewallTowerCost",    120, sf::Color(64, 156, 255)},
        {"Laser",        "LASER",     "laserTowerCost",        150, sf::Color(255, 46, 147)},
        {"Data Cleaner", "CLEANER",   "dataCleanerTowerCost", 130, sf::Color(0, 229, 255)},
        {"Overclock",    "OVERCLOCK", "overclockTowerCost",   180, sf::Color(255, 176, 32)},
        {"Corruption",   "CORRUPT",   "corruptionTowerCost",  140, sf::Color(170, 255, 60)},
        {"EMP Burst",    "EMP",       "empTowerCost",          200, sf::Color(160, 110, 255)},
    };
    return metas[static_cast<int>(t)];
}
