#include "extend.hpp"

// --- IMPLEMENTASI AUDIO SYSTEM ---
AudioSystem::AudioSystem() {
    if (buffer.loadFromFile("pop.wav")) {
        for(int i=0; i<MAX_VOICES; i++) {
            sf::Sound s; 
            s.setBuffer(buffer); 
            s.setVolume(30.0f); 
            voices.push_back(s);
        }
    } else {
        std::cout << "[ERROR] pop.wav not found!" << std::endl;
    }
}

void AudioSystem::playPop() {
    // Logika random pitch dan pooling dipindah ke sini
    float pitch = 0.8f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.4f));
    for (auto& s : voices) {
        if (s.getStatus() != sf::Sound::Playing) {
            s.setPitch(pitch); 
            s.play(); 
            return;
        }
    }
}

// --- IMPLEMENTASI UI (SLIDER, BTN, HUD) ---
// (Sama persis seperti sebelumnya, hanya copy-paste implementasinya)

MySlider::MySlider(float x, float y, float w, float min, float max, float start, std::string s) 
    : px(x), py(y), pw(w), minV(min), maxV(max), curV(start) 
{
    if (!font.loadFromFile("arial.ttf")) {}
    bar.setPosition(px, py + 20); bar.setSize({pw, 5}); bar.setFillColor(sf::Color(100, 100, 100));
    box.setSize({15, 20}); box.setFillColor(sf::Color(0, 200, 255)); box.setOrigin(7.5f, 7.5f); sync();
    lbl.setFont(font); lbl.setString(s); lbl.setCharacterSize(14); lbl.setFillColor(sf::Color::White); lbl.setPosition(px, py);
    val.setFont(font); val.setCharacterSize(14); val.setFillColor(sf::Color::Yellow); val.setPosition(px + pw + 10, py + 15); txt();
}
void MySlider::input(const sf::Event& e, const sf::RenderWindow& win) {
    sf::Vector2i m = sf::Mouse::getPosition(win);
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::FloatRect area = bar.getGlobalBounds(); area.top -= 15; area.height += 30;
        if (area.contains((float)m.x, (float)m.y)) { drag = true; calc(m.x); }
    } else if (e.type == sf::Event::MouseButtonReleased) drag = false;
    else if (e.type == sf::Event::MouseMoved && drag) calc(m.x);
}
void MySlider::draw(sf::RenderWindow& win) { win.draw(bar); win.draw(box); win.draw(lbl); win.draw(val); }
int MySlider::get() const { return (int)curV; }
void MySlider::calc(float mx) {
    if (mx < px) mx = px; if (mx > px + pw) mx = px + pw;
    float r = (mx - px) / pw; curV = minV + r * (maxV - minV); sync(); txt();
}
void MySlider::sync() { float r = (curV - minV) / (maxV - minV); box.setPosition(px + r * pw, py + 22); }
void MySlider::txt() { val.setString(std::to_string((int)curV)); }

MinimizeBtn::MinimizeBtn(float x, float y) {
    if (!font.loadFromFile("arial.ttf")) {}
    box.setSize({20, 20}); box.setPosition(x, y); box.setFillColor(sf::Color::White);
    box.setOutlineColor(sf::Color::Black); box.setOutlineThickness(1);
    symbol.setFont(font); symbol.setString("-"); symbol.setCharacterSize(20);
    symbol.setFillColor(sf::Color::Black); symbol.setPosition(x + 5, y - 4);
}
bool MinimizeBtn::isClicked(const sf::Event& e, const sf::RenderWindow& win) {
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i m = sf::Mouse::getPosition(win);
        if (box.getGlobalBounds().contains((float)m.x, (float)m.y)) return true;
    }
    return false;
}
void MinimizeBtn::draw(sf::RenderWindow& win) { win.draw(box); win.draw(symbol); }
void MinimizeBtn::setText(std::string s) { symbol.setString(s); }

Hud::Hud() { 
    font.loadFromFile("arial.ttf"); txt.setFont(font); 
    txt.setCharacterSize(16); txt.setFillColor(sf::Color::White); txt.setPosition(20, 130); 
}
void Hud::set(bool qt, bool g, bool grid, float fps) {
    std::string m = qt ? "Quadtree" : "Brute Force";
    std::string gr = g ? "ON" : "OFF";
    std::string k = grid ? "SHOW" : "HIDE";
    std::stringstream ss; ss << std::fixed << std::setprecision(1) << fps;
    txt.setString("Mode [Space]: " + m + "\nGravity [G]: " + gr + "\nGrid [K]: " + k + "\nFPS: " + ss.str());
}
void Hud::draw(sf::RenderWindow& w) { w.draw(txt); }