#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <iostream>

// dos2unix main.cpp
// g++ main.cpp -o bola_sfml -lsfml-graphics -lsfml-window -lsfml-system -std=c++17
// ./bola_sfml

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float BALL_RADIUS = 15.0f;
const int NUMBER_OF_BALLS = 100;
const float MAX_VELOCITY = 4.0f;
const std::string FONT_PATH = "arial.ttf"; 

// --- FUNGSI: Menangani Tumbukan Antar Bola ---
void solveBallCollision(Ball& b1, Ball& b2) {
    sf::Vector2f pos1 = b1.shape.getPosition();
    sf::Vector2f pos2 = b2.shape.getPosition();
    sf::Vector2f delta = pos1 - pos2;
    
    float distSq = delta.x * delta.x + delta.y * delta.y;
    float minDist = BALL_RADIUS * 2.0f;

    // Cek apakah jarak kurang dari diameter (tabrakan terjadi)
    if (distSq < minDist * minDist) {
        float dist = std::sqrt(distSq);

        if (dist == 0.0f) return; 

        // 1. KOREKSI POSISI
        sf::Vector2f normal = delta / dist; 
        float overlap = minDist - dist;
        
        b1.shape.move(normal * (overlap * 0.5f));
        b2.shape.move(-normal * (overlap * 0.5f));

        // 2. RESPON FISIKA
        float tx = -normal.y;
        float ty = normal.x;

        float dpTan1 = b1.velocity.x * tx + b1.velocity.y * ty;
        float dpTan2 = b2.velocity.x * tx + b2.velocity.y * ty;

        float dpNorm1 = b1.velocity.x * normal.x + b1.velocity.y * normal.y;
        float dpNorm2 = b2.velocity.x * normal.x + b2.velocity.y * normal.y;

        float m1 = dpNorm2; 
        float m2 = dpNorm1;

        b1.velocity.x = tx * dpTan1 + normal.x * m1;
        b1.velocity.y = ty * dpTan1 + normal.y * m1;
        
        b2.velocity.x = tx * dpTan2 + normal.x * m2;
        b2.velocity.y = ty * dpTan2 + normal.y * m2;
    }
}
// --------------------------------------------------

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Project Strukdat - Bola Pastel");
    window.setFramerateLimit(120);

    sf::Clock clock;
    sf::Text fpsText;
    sf::Font font;

    if (!font.loadFromFile(FONT_PATH)) {
        // Error handling
    }

    fpsText.setFont(font);
    fpsText.setCharacterSize(24);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition(10.f, 10.f);

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distPos(BALL_RADIUS, static_cast<float>(WINDOW_WIDTH - BALL_RADIUS));
    std::uniform_real_distribution<float> distVel(-MAX_VELOCITY, MAX_VELOCITY);
    
    // --- UBAH BAGIAN INI: DEFINISI WARNA PASTEL ---
    // Warna pastel memiliki nilai RGB yang tinggi (mendekati 255) agar terlihat lembut
    std::vector<sf::Color> pastelColors = {
        sf::Color(255, 179, 186), // Pastel Red / Cherry
        sf::Color(255, 223, 186), // Pastel Orange
        sf::Color(255, 255, 186), // Pastel Yellow
        sf::Color(186, 255, 201), // Pastel Green / Mint
        sf::Color(186, 225, 255), // Pastel Blue
        sf::Color(223, 186, 255), // Pastel Purple / Lavender
        sf::Color(255, 204, 229), // Pastel Pink
        sf::Color(204, 255, 255), // Pastel Cyan
        sf::Color(230, 230, 250), // Lavender Mist
        sf::Color(255, 240, 245)  // Lavender Blush
    };
    
    // Update distribusi ke vector pastelColors
    std::uniform_int_distribution<size_t> distColorIndex(0, pastelColors.size() - 1);

    std::vector<Ball> balls;
    balls.reserve(NUMBER_OF_BALLS);

    for (int i = 0; i < NUMBER_OF_BALLS; ++i) {
        Ball newBall;
        newBall.shape.setRadius(BALL_RADIUS);
        newBall.shape.setOrigin(BALL_RADIUS, BALL_RADIUS); 
        newBall.shape.setPosition(distPos(rng), distPos(rng));
        
        // --- GUNAKAN WARNA PASTEL DI SINI ---
        newBall.shape.setFillColor(pastelColors[distColorIndex(rng)]);

        sf::Vector2f velocity;
        do {
            velocity = sf::Vector2f(distVel(rng), distVel(rng));
        } while (std::abs(velocity.x) < 0.5f && std::abs(velocity.y) < 0.5f);

        newBall.velocity = velocity;
        balls.push_back(newBall);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Time elapsedTime = clock.restart(); 
        float fps = 1.0f / elapsedTime.asSeconds();
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));

        // 1. Update Posisi & Cek Dinding
        for (auto& ball : balls) {
            ball.shape.move(ball.velocity);

            sf::Vector2f ballPosition = ball.shape.getPosition();
            float radius = BALL_RADIUS;

            if (ballPosition.x - radius < 0 || ballPosition.x + radius > WINDOW_WIDTH) {
                ball.velocity.x *= -1; 
                if (ballPosition.x - radius < 0) ball.shape.setPosition(radius, ballPosition.y);
                else ball.shape.setPosition(WINDOW_WIDTH - radius, ballPosition.y);
            }

            if (ballPosition.y - radius < 0 || ballPosition.y + radius > WINDOW_HEIGHT) {
                ball.velocity.y *= -1;
                if (ballPosition.y - radius < 0) ball.shape.setPosition(ballPosition.x, radius);
                else ball.shape.setPosition(ballPosition.x, WINDOW_HEIGHT - radius);
            }
        }

        // 2. Update Tumbukan Antar Bola
        for (size_t i = 0; i < balls.size(); ++i) {
            for (size_t j = i + 1; j < balls.size(); ++j) {
                solveBallCollision(balls[i], balls[j]);
            }
        }
        
        // Ubah warna background jadi abu-abu sangat gelap agar warna pastel menonjol
        // (Warna hitam pekat juga oke, tapi abu gelap (30,30,30) biasanya lebih nyaman di mata)
        window.clear(sf::Color(30, 30, 30)); 
        
        for (const auto& ball : balls) {
            window.draw(ball.shape);
        }
        
        window.draw(fpsText); 
        
        window.display();
    }

    return 0;
}