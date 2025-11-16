#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <iostream>
#include <deque>

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    std::deque<sf::Vector2f> trail;
};

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1200;
const float BALL_RADIUS = 30.0f;
const int NUMBER_OF_BALLS = 20;
const float MAX_VELOCITY = 4.0f;
const size_t TRAIL_LENGTH = 15;
const float TRAIL_ALPHA_DECAY = 0.8f;
const float TRAIL_SCALE_DECAY = 0.9f;
const std::string FONT_PATH = "arial.ttf"; 

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Project Strukdat - Bola Berekor");
    window.setFramerateLimit(120);

    sf::Clock clock;
    sf::Text fpsText;
    sf::Font font;

    if (!font.loadFromFile(FONT_PATH)) {
        std::cerr << "Error: Gagal memuat font dari " << FONT_PATH << std::endl;
        return EXIT_FAILURE; 
    }

    fpsText.setFont(font);
    fpsText.setCharacterSize(24);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition(10.f, 10.f);

    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distPos(BALL_RADIUS, static_cast<float>(WINDOW_WIDTH - BALL_RADIUS));
    std::uniform_real_distribution<float> distVel(-MAX_VELOCITY, MAX_VELOCITY);
    
    std::vector<sf::Color> brightColors = {
        sf::Color::Red, sf::Color::Blue, sf::Color::Green,
        sf::Color::Yellow, sf::Color::Cyan, sf::Color::Magenta
    };
    std::uniform_int_distribution<size_t> distColorIndex(0, brightColors.size() - 1);

    std::vector<Ball> balls;
    balls.reserve(NUMBER_OF_BALLS);

    for (int i = 0; i < NUMBER_OF_BALLS; ++i) {
        Ball newBall;
        newBall.shape.setRadius(BALL_RADIUS);
        newBall.shape.setOrigin(BALL_RADIUS, BALL_RADIUS); 
        newBall.shape.setPosition(distPos(rng), distPos(rng));
        newBall.shape.setFillColor(brightColors[distColorIndex(rng)]);

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

        for (auto& ball : balls) {
            ball.shape.move(ball.velocity);

            ball.trail.push_front(ball.shape.getPosition());
            if (ball.trail.size() > TRAIL_LENGTH) {
                ball.trail.pop_back(); 
            }

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
        
        window.clear(sf::Color::Black);
        
        for (const auto& ball : balls) {
            float currentAlpha = 255.0f;
            float currentScale = 1.0f;
            
            for (size_t k = 0; k < ball.trail.size(); ++k) {
                sf::CircleShape trailSegment = ball.shape;
                
                currentAlpha *= TRAIL_ALPHA_DECAY;
                currentScale *= TRAIL_SCALE_DECAY;

                sf::Color originalColor = ball.shape.getFillColor();
                trailSegment.setFillColor(sf::Color(
                    originalColor.r,
                    originalColor.g,
                    originalColor.b,
                    static_cast<sf::Uint8>(currentAlpha)
                ));
                trailSegment.setRadius(BALL_RADIUS * currentScale);
                trailSegment.setOrigin(BALL_RADIUS * currentScale, BALL_RADIUS * currentScale);
                trailSegment.setPosition(ball.trail[k]);
                
                window.draw(trailSegment);
            }
            
            window.draw(ball.shape);
        }
        
        window.draw(fpsText); 
        
        window.display();
    }

    return 0;
}