#pragma once
#include <string>

// =============================================================
// Paths - pomaga tworzyc poprawne sciezki do plikow gry
// Szuka katalogu bazowego z folderami assets lub data
// =============================================================
class Paths {
public:
    // Szuka katalogu bazowego gry
    static void init();

    // Tworzy pelna sciezke z podanej sciezki wzglednej
    static std::string resolve(const std::string& relative);

private:
    static std::string s_base; // katalog bazowy gry
};
