#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <iomanip>
#include <sstream>

// --- STRUKTUR SLIDER ---
struct Slider {
    sf::RectangleShape track;
    sf::RectangleShape thumb;
    sf::Text label;
    sf::Text valueText;
    
    bool isDragging = false;
    float minValue;
    float maxValue;
    float currentValue;

    Slider(float x, float y, float w, float h, float minV, float maxV, float initialV, std::string name, sf::Font& font) 
        : minValue(minV), maxValue(maxV), currentValue(initialV) 
    {
        track.setPosition(x, y + h/2 - 2);
        track.setSize({w, 4});
        track.setFillColor(sf::Color(100, 100, 100));

        thumb.setSize({10, h});
        thumb.setOrigin(5, 0); 
        thumb.setFillColor(sf::Color(0, 200, 255)); 
        
        label.setFont(font);
        label.setString(name);
        label.setCharacterSize(14);
        label.setFillColor(sf::Color::White);
        label.setPosition(x, y - 20);

        valueText.setFont(font);
        valueText.setCharacterSize(14);
        valueText.setFillColor(sf::Color::Yellow);
        valueText.setPosition(x + w + 10, y - 5);

        updateThumbPos();
        updateValueText();
    }

    void updateThumbPos() {
        float ratio = (currentValue - minValue) / (maxValue - minValue);
        float x = track.getPosition().x + ratio * track.getSize().x;
        thumb.setPosition(x, track.getPosition().y - (thumb.getSize().y/2) + 2);
    }

    void updateValueFromPos(float mouseX) {
        float trackX = track.getPosition().x;
        float trackW = track.getSize().x;
        
        if (mouseX < trackX) mouseX = trackX;
        if (mouseX > trackX + trackW) mouseX = trackX + trackW;

        float ratio = (mouseX - trackX) / trackW;
        currentValue = minValue + ratio * (maxValue - minValue);
        
        updateThumbPos();
        updateValueText();
    }

    void updateValueText() {
        std::stringstream ss;
        if (maxValue > 100) ss << (int)currentValue; 
        else ss << std::fixed << std::setprecision(2) << currentValue; 
        valueText.setString(ss.str());
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::FloatRect touchArea = track.getGlobalBounds();
            touchArea.top -= 10; touchArea.height += 20; 
            
            if (touchArea.contains((float)mousePos.x, (float)mousePos.y)) {
                isDragging = true;
                updateValueFromPos((float)mousePos.x);
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            isDragging = false;
        }
        else if (event.type == sf::Event::MouseMoved && isDragging) {
            updateValueFromPos((float)sf::Mouse::getPosition(window).x);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(track);
        window.draw(thumb);
        window.draw(label);
        window.draw(valueText);
    }
};

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

// --- QUADTREE OPTIMIZED ---
struct Rect {
    float x, y, w, h; 
    bool contains(const sf::Vector2f& point) {
        return (point.x >= x - w && point.x <= x + w &&
                point.y >= y - h && point.y <= y + h);
    }
    bool intersects(const Rect& other) {
        return !(other.x - other.w > x + w || other.x + other.w < x - w ||
                 other.y - other.h > y + h || other.y + other.h < y - h);
    }
};

class Quadtree {
    Rect boundary;
    int capacity;
    std::vector<int> ballIndices; 
    bool divided = false;
    std::unique_ptr<Quadtree> nw, ne, sw, se;

public:
    Quadtree(Rect b, int c) : boundary(b), capacity(c) {}

    void insert(int index, const std::vector<Ball>& balls) {
        if (!boundary.contains(balls[index].shape.getPosition())) return;
        if (ballIndices.size() < capacity && !divided) {
            ballIndices.push_back(index);
        } else {
            if (!divided) subdivide();
            nw->insert(index, balls); ne->insert(index, balls);
            sw->insert(index, balls); se->insert(index, balls);
        }
    }

    void subdivide() {
        float x = boundary.x; float y = boundary.y;
        float w = boundary.w / 2; float h = boundary.h / 2;
        nw = std::make_unique<Quadtree>(Rect{x - w, y - h, w, h}, capacity);
        ne = std::make_unique<Quadtree>(Rect{x + w, y - h, w, h}, capacity);
        sw = std::make_unique<Quadtree>(Rect{x - w, y + h, w, h}, capacity);
        se = std::make_unique<Quadtree>(Rect{x + w, y + h, w, h}, capacity);
        divided = true;
    }

    void query(Rect range, std::vector<int>& found) {
        if (!boundary.intersects(range)) return;
        for (int index : ballIndices) found.push_back(index);
        if (divided) {
            nw->query(range, found); ne->query(range, found);
            sw->query(range, found); se->query(range, found);
        }
    }
};

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1200;
const float BALL_RADIUS = 5.0f; // Diperbesar sedikit agar lebih sering tabrakan
const std::string FONT_PATH = "arial.ttf"; 

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

void adjustBallCount(std::vector<Ball>& balls, int targetCount, const std::vector<sf::Color>& colors) {
    int currentCount = balls.size();
    if (currentCount < targetCount) {
        int diff = targetCount - currentCount;
        std::uniform_real_distribution<float> distPosX(BALL_RADIUS, WINDOW_WIDTH - BALL_RADIUS);
        std::uniform_real_distribution<float> distPosY(BALL_RADIUS, WINDOW_HEIGHT - BALL_RADIUS);
        std::uniform_real_distribution<float> distVel(-3.0f, 3.0f);
        std::uniform_int_distribution<size_t> distColor(0, colors.size() - 1);

        balls.reserve(targetCount); 
        for(int i=0; i<diff; i++) {
            Ball newBall;
            newBall.shape.setRadius(BALL_RADIUS);
            newBall.shape.setOrigin(BALL_RADIUS, BALL_RADIUS);
            newBall.shape.setPosition(distPosX(rng), distPosY(rng));
            newBall.shape.setFillColor(colors[distColor(rng)]);
            newBall.velocity = {distVel(rng), distVel(rng)};
            balls.push_back(newBall);
        }
    } else if (currentCount > targetCount) {
        balls.resize(targetCount);
    }
}

void solveBallCollision(Ball& b1, Ball& b2) {
    sf::Vector2f pos1 = b1.shape.getPosition();
    sf::Vector2f pos2 = b2.shape.getPosition();
    sf::Vector2f delta = pos1 - pos2;
    float distSq = delta.x * delta.x + delta.y * delta.y;
    float minDist = BALL_RADIUS * 2.0f;

    if (distSq < minDist * minDist) {
        float dist = std::sqrt(distSq);
        if (dist == 0.0f) return; 

        sf::Vector2f normal = delta / dist; 
        float overlap = minDist - dist;
        b1.shape.move(normal * (overlap * 0.5f));
        b2.shape.move(-normal * (overlap * 0.5f));

        float tx = -normal.y; float ty = normal.x;
        float dpTan1 = b1.velocity.x * tx + b1.velocity.y * ty;
        float dpTan2 = b2.velocity.x * tx + b2.velocity.y * ty;
        float dpNorm1 = b1.velocity.x * normal.x + b1.velocity.y * normal.y;
        float dpNorm2 = b2.velocity.x * normal.x + b2.velocity.y * normal.y;

        b1.velocity.x = tx * dpTan1 + normal.x * dpNorm2;
        b1.velocity.y = ty * dpTan1 + normal.y * dpNorm2;
        b2.velocity.x = tx * dpTan2 + normal.x * dpNorm1;
        b2.velocity.y = ty * dpTan2 + normal.y * dpNorm1;
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Bola toingtoing");
    window.setFramerateLimit(0); 

    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) { /* Error Handling */ }

    // --- SETUP SLIDER ---
    // Max 20.000 Bola. Angka ini sudah cukup membuat i9 sekalipun menangis di Brute Force.
    Slider ballSlider(30, 60, 200, 20, 0, 20000, 500, "Ball Count (Max 20k)", font); 
    Slider speedSlider(30, 110, 200, 20, 0.0f, 5.0f, 1.0f, "Sim Speed", font);

    sf::RectangleShape panelBg(sf::Vector2f(350.0f, 180.0f)); 
    panelBg.setFillColor(sf::Color(20, 20, 20, 220)); 
    panelBg.setPosition(10.0f, 10.0f);
    panelBg.setOutlineColor(sf::Color::White);
    panelBg.setOutlineThickness(1.0f);

    sf::Text toggleText;
    toggleText.setFont(font);
    toggleText.setString("[SPACE] Mode: Brute Force");
    toggleText.setCharacterSize(16);
    toggleText.setFillColor(sf::Color::White);
    toggleText.setPosition(30, 150);

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(14);
    fpsText.setFillColor(sf::Color::Green);
    fpsText.setPosition(250, 150);

    std::vector<sf::Color> pastelColors = {
        sf::Color(255, 179, 186), sf::Color(255, 223, 186), sf::Color(255, 255, 186), 
        sf::Color(186, 255, 201), sf::Color(186, 225, 255), sf::Color(223, 186, 255)
    };

    std::vector<Ball> balls;
    adjustBallCount(balls, (int)ballSlider.currentValue, pastelColors);

    bool useQuadtree = false;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            
            ballSlider.handleEvent(event, window);
            speedSlider.handleEvent(event, window);

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                useQuadtree = !useQuadtree;
                toggleText.setString(useQuadtree ? "[SPACE] Mode: Quadtree" : "[SPACE] Mode: Brute Force");
                toggleText.setFillColor(useQuadtree ? sf::Color::Cyan : sf::Color::White);
            }
        }

        // Logic Slider
        int targetBalls = (int)ballSlider.currentValue;
        if (balls.size() != targetBalls) {
            adjustBallCount(balls, targetBalls, pastelColors);
        }
        float speedMult = speedSlider.currentValue;

        // Move Balls
        for (auto& ball : balls) {
            ball.shape.move(ball.velocity * speedMult);

            sf::Vector2f pos = ball.shape.getPosition();
            if (pos.x < BALL_RADIUS) { ball.velocity.x = std::abs(ball.velocity.x); ball.shape.setPosition(BALL_RADIUS, pos.y); }
            if (pos.x > WINDOW_WIDTH - BALL_RADIUS) { ball.velocity.x = -std::abs(ball.velocity.x); ball.shape.setPosition(WINDOW_WIDTH - BALL_RADIUS, pos.y); }
            if (pos.y < BALL_RADIUS) { ball.velocity.y = std::abs(ball.velocity.y); ball.shape.setPosition(pos.x, BALL_RADIUS); }
            if (pos.y > WINDOW_HEIGHT - BALL_RADIUS) { ball.velocity.y = -std::abs(ball.velocity.y); ball.shape.setPosition(pos.x, WINDOW_HEIGHT - BALL_RADIUS); }
        }

        // Collision Logic
        if (!useQuadtree) {
            // BRUTE FORCE TANPA LIMIT (Siap-siap LAG jika bola > 2000)
            for (size_t i = 0; i < balls.size(); ++i) {
                for (size_t j = i + 1; j < balls.size(); ++j) {
                    solveBallCollision(balls[i], balls[j]);
                }
            }
        } else {
            // QUADTREE
            Rect boundary = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
            Quadtree qt(boundary, 4);
            for (size_t i = 0; i < balls.size(); ++i) qt.insert(i, balls);

            for (size_t i = 0; i < balls.size(); ++i) {
                std::vector<int> candidates;
                candidates.reserve(32); // Optimasi alokasi vector
                Rect range = {balls[i].shape.getPosition().x, balls[i].shape.getPosition().y, BALL_RADIUS * 2, BALL_RADIUS * 2};
                qt.query(range, candidates);
                for (int j : candidates) {
                    if (i < j) solveBallCollision(balls[i], balls[j]);
                }
            }
        }

        // Render
        window.clear(sf::Color(30, 30, 30)); 
        for (const auto& ball : balls) window.draw(ball.shape);

        // GUI
        window.draw(panelBg);
        ballSlider.draw(window);
        speedSlider.draw(window);
        window.draw(toggleText);

        sf::Time elapsedTime = clock.restart(); 
        float fps = 1.0f / elapsedTime.asSeconds();
        fpsText.setString("FPS: " + std::to_string((int)fps));
        window.draw(fpsText);

        window.display();
    }

    return 0;
}