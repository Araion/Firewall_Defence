#pragma once
#include <random>

// =============================================================
// Rng - wspolny generator liczb losowych dla calej gry
// Uzywany do losowania liczb, typow przeciwnikow i szans na zdarzenia
// =============================================================
class Rng {
public:
    // Ustawia ziarno generatora
    static void seed(unsigned s);

    // Ustawia losowe ziarno generatora
    static void seedRandom();

    // Losuje liczbe calkowita z przedzialu min-max
    static int range(int min, int max);

    // Losuje liczbe zmiennoprzecinkowa z przedzialu min-max
    static float rangef(float min, float max);

    // Zwraca true z podanym prawdopodobienstwem 0-1
    static bool chance(float p);
private:
    static std::mt19937 s_engine;
};
