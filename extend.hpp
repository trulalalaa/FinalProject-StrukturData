#ifndef EXTEND_HPP
#define EXTEND_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iostream>

// --- KONFIGURASI GLOBAL ---
const int W = 1200;
const int H = 800;
const float R = 1.0f;       // Bola Kecil
const int MAX_LVL = 7;      // Quadtree Dalam (untuk 15k bola)
const int MAX_VOICES = 32;

// --- AUDIO MANAGER (Baru: Biar main.cpp bersih dari setup audio) ---
class AudioSystem {
    sf::SoundBuffer buffer;
    std::vector<sf::Sound> voices;
public:
    AudioSystem();
    void playPop(); // Panggil ini aja kalau mau bunyi
};

// --- UI CLASSES ---
class MySlider {
    sf::RectangleShape bar, box;
    sf::Text lbl, val;
    sf::Font font;
    bool drag = false;
    float minV, maxV, curV, px, py, pw;

public:
    MySlider(float x, float y, float w, float min, float max, float start, std::string s);
    void input(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);
    int get() const;
private:
    void calc(float mx);
    void sync();
    void txt();
};

class MinimizeBtn {
    sf::RectangleShape box;
    sf::Text symbol;
    sf::Font font;
public:
    MinimizeBtn(float x, float y);
    bool isClicked(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);
    void setText(std::string s);
};

class Hud {
    sf::Font font;
    sf::Text txt;
public:
    Hud();
    void set(bool qt, bool g, bool grid, float fps);
    void draw(sf::RenderWindow& w);
};

#endif