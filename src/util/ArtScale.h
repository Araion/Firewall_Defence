#pragma once

// =============================================================
// ArtScale - stale okreslajace rozmiary grafik w grze
// Pozwalaja ustawic docelowe rozmiary sprite'ow niezaleznie od rozmiaru plikow PNG
// =============================================================
namespace Art {
// Docelowa szerokosc podstawy wiezy w pikselach
inline constexpr float kTowerBaseWidth = 58.f;

// Mnoznik rozmiaru sprite'a przeciwnika wzgledem jego promienia
inline constexpr float kEnemySizeMult = 2.8f;

// Dlugosc zwyklego pocisku w pikselach
inline constexpr float kShotLength = 30.f;

// Dlugosc pocisku obszarowego w pikselach
inline constexpr float kShotLengthSplash = 46.f;

// Docelowa szerokosc rdzenia serwera w pikselach
inline constexpr float kServerWidth = 120.f;

// Docelowa szerokosc logo w menu glownym w pikselach
inline constexpr float kLogoWidth = 600.f;
}