#pragma once
#include <random>

// =============================================================
//  Rng - centralny generator liczb losowych (Mersenne Twister).
// =============================================================
class Rng {
public:
    // Inicjalizacja generatora podanym ziarnem (domyslnie z urzadzenia losowego).
    static void seed(unsigned s);
    static void seedRandom();

    // Losowa liczba calkowita z przedzialu [min, max] (wlacznie).
    static int range(int min, int max);

    // Losowa liczba zmiennoprzecinkowa z przedzialu [min, max).
    static float rangef(float min, float max);

    // Zwraca true z prawdopodobienstwem p (0.0 - 1.0).
    static bool chance(float p);

private:
    static std::mt19937 s_engine;
};
