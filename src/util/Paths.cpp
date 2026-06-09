#include "util/Paths.h"
#include <filesystem>

namespace fs = std::filesystem;

std::string Paths::s_base = "";

void Paths::init() {
    try {
        fs::path dir = fs::current_path();

        // Szuka folderu z data lub assets, zaczynajac od katalogu uruchomienia
        for (int i = 0; i < 6; ++i) {
            if (fs::exists(dir / "data") || fs::exists(dir / "assets")) {
                s_base = dir.string();
                return;
            }

            if (!dir.has_parent_path())
                break;

            dir = dir.parent_path();
        }

        // Jesli nie znaleziono katalogu bazowego, uzywa katalogu uruchomienia
        s_base = fs::current_path().string();
    } catch (const fs::filesystem_error&) {
        // W razie problemu uzywa sciezek wzglednych
        s_base = "";
    }
}

std::string Paths::resolve(const std::string& relative) {
    if (s_base.empty()) return relative;
    fs::path p = fs::path(s_base) / relative;
    return p.string();
}
