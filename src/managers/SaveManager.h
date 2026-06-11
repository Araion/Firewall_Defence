#pragma once
#include <string>

class PlayState;

// =============================================================
// SaveManager - zapis i odczyt stanu planszy w prostym formacie
// tekstowym (data/savegame.txt). Zapisuje: kredyty, wynik,
// zdrowie serwera, ziarno i trudnosc mapy oraz liste wiez
// (typ, poziom, pozycja) i zywych wrogow (typ, HP, sciezka,
// dystans). Mape odtwarza z seedu
// =============================================================
class SaveManager {
public:
    bool save(PlayState& state, const std::string& relativePath = "data/savegame.txt");
    bool load(PlayState& state, const std::string& relativePath = "data/savegame.txt");
};
