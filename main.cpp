#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Latihan 1: Bola Memantul");
    window.setFramerateLimit(60);

    float radius = 20.0f;
    sf::CircleShape ball(radius);
    ball.setFillColor(sf::Color::Cyan);
    ball.setPosition(400.0f, 300.0f);

    sf::Vector2f velocity(5.0f, 5.0f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        ball.move(velocity);

        sf::Vector2f pos = ball.getPosition();

        if (pos.x < 0 || pos.x + (radius * 2) > 800) {
            velocity.x = -velocity.x;
        }

        if (pos.y < 0 || pos.y + (radius * 2) > 600) {
            velocity.y = -velocity.y;
        }

        window.clear();
        window.draw(ball);
        window.display();
    }

    return 0;
}


