#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <iomanip>
#include <sstream>

struct Button
{
    sf::RectangleShape shape;
    sf::Text text;
    bool state = false;

    Button(float w, float h, std::string label, sf::Font& font)
    {
        shape.setSize({w, h});
        shape.setFillColor(sf::Color(50, 50, 50));
        shape.setOutlineColor(sf::Color(100, 100, 100));
        shape.setOutlineThickness(1.0f);

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(13);
        text.setFillColor(sf::Color::White);
    }

    void setPosition(float x, float y)
    {
        shape.setPosition(x, y);

        sf::FloatRect bounds = text.getLocalBounds();
        float textX = std::floor(x + (shape.getSize().x - bounds.width) / 2.0f - bounds.left);
        float textY = std::floor(y + (shape.getSize().y - bounds.height) / 2.0f - bounds.top);
        text.setPosition(textX, textY);
    }

    bool isMouseOver(const sf::RenderWindow& window)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        return shape.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y);
    }

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            if (isMouseOver(window))
            {
                state = !state;
                return true;
            }
        }
        return false;
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(shape);
        window.draw(text);
    }
};

struct Slider
{
    sf::RectangleShape track;
    sf::RectangleShape thumb;
    sf::Text label;
    sf::Text valueText;

    bool isDragging = false;
    float minValue, maxValue, currentValue;
    float width, height;

    Slider(float w, float h, float minV, float maxV, float initialV, std::string name, sf::Font& font)
        : width(w), height(h), minValue(minV), maxValue(maxV), currentValue(initialV)
    {
        track.setSize({w, 4});
        track.setFillColor(sf::Color(80, 80, 80));

        thumb.setSize({12, h});
        thumb.setOrigin(6, 0);
        thumb.setFillColor(sf::Color(0, 180, 255));

        label.setFont(font);
        label.setString(name);
        label.setCharacterSize(13);
        label.setFillColor(sf::Color(220, 220, 220));

        valueText.setFont(font);
        valueText.setCharacterSize(13);
        valueText.setFillColor(sf::Color(255, 200, 0));
    }

    void setPosition(float x, float y)
    {
        label.setPosition(x, y);

        sf::FloatRect valBounds = valueText.getLocalBounds();
        valueText.setPosition(x + width - valBounds.width, y);

        track.setPosition(x, y + 22);
        updateThumbVisual();
    }

    void updateThumbVisual()
    {
        float ratio = (currentValue - minValue) / (maxValue - minValue);
        float x = track.getPosition().x + ratio * track.getSize().x;
        thumb.setPosition(x, track.getPosition().y - (thumb.getSize().y/2) + 2);
    }

    void updateValueFromPos(float mouseX)
    {
        float trackX = track.getPosition().x;
        float trackW = track.getSize().x;

        if (mouseX < trackX) mouseX = trackX;
        if (mouseX > trackX + trackW) mouseX = trackX + trackW;

        float ratio = (mouseX - trackX) / trackW;
        currentValue = minValue + ratio * (maxValue - minValue);

        updateThumbVisual();
        updateValueText();
    }

    void updateValueText()
    {
        std::stringstream ss;
        if (maxValue > 100) ss << (int)currentValue;
        else ss << std::fixed << std::setprecision(2) << currentValue;
        valueText.setString(ss.str());

        sf::FloatRect valBounds = valueText.getLocalBounds();
        valueText.setPosition(track.getPosition().x + width - valBounds.width, label.getPosition().y);
    }

    bool isMouseOver(const sf::RenderWindow& window)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect touchArea = track.getGlobalBounds();
        touchArea.top -= 12; touchArea.height += 24;
        return touchArea.contains((float)mousePos.x, (float)mousePos.y) || isDragging;
    }

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        bool interacted = false;
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            if (isMouseOver(window))
            {
                isDragging = true;
                updateValueFromPos((float)sf::Mouse::getPosition(window).x);
                interacted = true;
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            isDragging = false;
        }
        else if (event.type == sf::Event::MouseMoved && isDragging)
        {
            updateValueFromPos((float)sf::Mouse::getPosition(window).x);
            interacted = true;
        }
        return interacted || isDragging;
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(track);
        window.draw(thumb);
        window.draw(label);
        window.draw(valueText);
    }
};

struct Ball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;
const int BORDER_SIZE = 20;
const int SIM_X = BORDER_SIZE;
const int SIM_Y = BORDER_SIZE + 30;
const int SIM_W = WINDOW_WIDTH - (BORDER_SIZE * 2);
const int SIM_H = WINDOW_HEIGHT - (BORDER_SIZE + SIM_Y);

const float BALL_RADIUS = 5.0f;
const std::string FONT_PATH = "arial.ttf";

struct Rect
{
    float x, y, w, h;
    bool contains(const sf::Vector2f& point)
    {
        return (point.x >= x - w && point.x <= x + w &&
                point.y >= y - h && point.y <= y + h);
    }
    bool intersects(const Rect& other)
    {
        return !(other.x - other.w > x + w || other.x + other.w < x - w ||
                 other.y - other.h > y + h || other.y + other.h < y - h);
    }
};

class Quadtree
{
    Rect boundary;
    int capacity;
    std::vector<int> ballIndices;
    bool divided = false;
    std::unique_ptr<Quadtree> nw, ne, sw, se;

public:
    Quadtree(Rect b, int c) : boundary(b), capacity(c) {}

    void insert(int index, const std::vector<Ball>& balls)
    {
        if (!boundary.contains(balls[index].shape.getPosition())) return;
        if (ballIndices.size() < capacity && !divided)
        {
            ballIndices.push_back(index);
        }
        else
        {
            if (!divided) subdivide();
            nw->insert(index, balls); ne->insert(index, balls);
            sw->insert(index, balls); se->insert(index, balls);
        }
    }

    void subdivide()
    {
        float x = boundary.x; float y = boundary.y;
        float w = boundary.w / 2; float h = boundary.h / 2;
        nw = std::make_unique<Quadtree>(Rect{x - w, y - h, w, h}, capacity);
        ne = std::make_unique<Quadtree>(Rect{x + w, y - h, w, h}, capacity);
        sw = std::make_unique<Quadtree>(Rect{x - w, y + h, w, h}, capacity);
        se = std::make_unique<Quadtree>(Rect{x + w, y + h, w, h}, capacity);
        divided = true;
    }

    void query(Rect range, std::vector<int>& found)
    {
        if (!boundary.intersects(range)) return;
        for (int index : ballIndices) found.push_back(index);
        if (divided)
        {
            nw->query(range, found); ne->query(range, found);
            sw->query(range, found); se->query(range, found);
        }
    }
};

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

void adjustBallCount(std::vector<Ball>& balls, int targetCount, const std::vector<sf::Color>& colors)
{
    int currentCount = balls.size();
    if (currentCount < targetCount)
    {
        int diff = targetCount - currentCount;
        std::uniform_real_distribution<float> distPosX(SIM_X + BALL_RADIUS, SIM_X + SIM_W - BALL_RADIUS);
        std::uniform_real_distribution<float> distPosY(SIM_Y + BALL_RADIUS, SIM_Y + SIM_H - BALL_RADIUS);
        std::uniform_real_distribution<float> distVel(-3.0f, 3.0f);
        std::uniform_int_distribution<size_t> distColor(0, colors.size() - 1);

        balls.reserve(targetCount);
        for(int i=0; i<diff; i++)
        {
            Ball newBall;
            newBall.shape.setRadius(BALL_RADIUS);
            newBall.shape.setOrigin(BALL_RADIUS, BALL_RADIUS);
            newBall.shape.setPosition(distPosX(rng), distPosY(rng));
            newBall.shape.setFillColor(colors[distColor(rng)]);
            newBall.velocity = {distVel(rng), distVel(rng)};
            balls.push_back(newBall);
        }
    }
    else if (currentCount > targetCount)
    {
        balls.resize(targetCount);
    }
}

void solveBallCollision(Ball& b1, Ball& b2)
{
    sf::Vector2f pos1 = b1.shape.getPosition();
    sf::Vector2f pos2 = b2.shape.getPosition();
    sf::Vector2f delta = pos1 - pos2;
    float distSq = delta.x * delta.x + delta.y * delta.y;
    float minDist = BALL_RADIUS * 2.0f;

    if (distSq < minDist * minDist)
    {
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

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Physics Sim - Draggable Panel", sf::Style::None);
    window.setFramerateLimit(0);

    sf::Cursor arrowCursor, handCursor, moveCursor;
    if (arrowCursor.loadFromSystem(sf::Cursor::Arrow)) window.setMouseCursor(arrowCursor);
    handCursor.loadFromSystem(sf::Cursor::Hand);
    moveCursor.loadFromSystem(sf::Cursor::SizeAll);

    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) { }

    sf::Vector2f panelPos(SIM_X + 20, SIM_Y + 20);
    sf::Vector2f panelSizeNormal(280.0f, 260.0f);
    sf::Vector2f panelSizeMini(280.0f, 35.0f);

    sf::RectangleShape panelBg;
    panelBg.setFillColor(sf::Color(30, 30, 30, 230));
    panelBg.setOutlineColor(sf::Color(100, 100, 100));
    panelBg.setOutlineThickness(1.0f);
    panelBg.setSize(panelSizeNormal);
    panelBg.setPosition(panelPos);

    sf::Text panelHeader;
    panelHeader.setFont(font);
    panelHeader.setString("CONTROL PANEL");
    panelHeader.setCharacterSize(14);
    panelHeader.setFillColor(sf::Color::White);
    panelHeader.setStyle(sf::Text::Bold);

    Button btnMinimize(20, 20, "-", font);
    btnMinimize.shape.setFillColor(sf::Color::White);
    btnMinimize.shape.setOutlineColor(sf::Color::White);
    btnMinimize.text.setFillColor(sf::Color::Black);
    btnMinimize.text.setStyle(sf::Text::Bold);

    Slider ballSlider(240, 15, 0, 10000, 500, "Ball Count", font);
    Slider speedSlider(240, 15, 0.0f, 5.0f, 1.0f, "Simulation Speed", font);
    Button gravityBtn(240, 30, "Gravity: OFF", font);

    sf::Text toggleText("[SPACE] Mode: Brute Force", font, 12);
    toggleText.setFillColor(sf::Color(180, 180, 180));
    sf::Text fpsText("FPS: 0", font, 12);
    fpsText.setFillColor(sf::Color::Green);

    sf::RectangleShape closeBtn(sf::Vector2f(40, 30));
    closeBtn.setFillColor(sf::Color(200, 50, 50));
    closeBtn.setPosition(WINDOW_WIDTH - 40, 0);
    sf::Text closeText("X", font, 18);
    closeText.setPosition(WINDOW_WIDTH - 26, 3);

    sf::RectangleShape simArea(sf::Vector2f(SIM_W, SIM_H));
    simArea.setFillColor(sf::Color(30, 30, 30));
    simArea.setPosition(SIM_X, SIM_Y);

    std::vector<sf::Color> pastelColors = {
        sf::Color(255, 179, 186), sf::Color(255, 223, 186), sf::Color(255, 255, 186),
        sf::Color(186, 255, 201), sf::Color(186, 225, 255), sf::Color(223, 186, 255)
    };
    std::vector<Ball> balls;
    adjustBallCount(balls, (int)ballSlider.currentValue, pastelColors);

    bool useQuadtree = false;
    bool isDraggingWindow = false;
    bool isDraggingPanel = false;
    bool isPanelMinimized = false;
    sf::Vector2i dragOffset;
    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();

            bool uiInteracted = false;

            panelBg.setPosition(panelPos);

            sf::FloatRect hBounds = panelHeader.getLocalBounds();
            panelHeader.setPosition(std::floor(panelPos.x + (panelSizeNormal.x - hBounds.width)/2.0f), panelPos.y + 10);

            btnMinimize.setPosition(panelPos.x + panelSizeNormal.x - 30, panelPos.y + 8);

            if (!isPanelMinimized)
            {
                float startContentY = panelPos.y + 45;
                ballSlider.setPosition(panelPos.x + 20, startContentY);
                speedSlider.setPosition(panelPos.x + 20, startContentY + 50);
                gravityBtn.setPosition(panelPos.x + 20, startContentY + 100);

                toggleText.setPosition(panelPos.x + 20, startContentY + 150);
                fpsText.setPosition(panelPos.x + 20, startContentY + 170);
            }

            if (btnMinimize.handleEvent(event, window))
            {
                isPanelMinimized = !isPanelMinimized;
                panelBg.setSize(isPanelMinimized ? panelSizeMini : panelSizeNormal);
                uiInteracted = true;
            }

            if (!isPanelMinimized)
            {
                uiInteracted |= ballSlider.handleEvent(event, window);
                uiInteracted |= speedSlider.handleEvent(event, window);

                if (gravityBtn.handleEvent(event, window))
                {
                    gravityBtn.text.setString(gravityBtn.state ? "Gravity: ON" : "Gravity: OFF");
                    gravityBtn.setPosition(gravityBtn.shape.getPosition().x, gravityBtn.shape.getPosition().y);
                    gravityBtn.shape.setFillColor(gravityBtn.state ? sf::Color(0, 180, 0) : sf::Color(50, 50, 50));
                    uiInteracted = true;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mPos = sf::Mouse::getPosition(window);

                if (uiInteracted)
                {
                }
                else if (closeBtn.getGlobalBounds().contains((float)mPos.x, (float)mPos.y))
                {
                    window.close();
                }
                else if (panelBg.getGlobalBounds().contains((float)mPos.x, (float)mPos.y))
                {
                    isDraggingPanel = true;
                    dragOffset = mPos - sf::Vector2i((int)panelPos.x, (int)panelPos.y);
                }
                else if (!simArea.getGlobalBounds().contains((float)mPos.x, (float)mPos.y))
                {
                    isDraggingWindow = true;
                    dragOffset = mPos;
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                isDraggingWindow = false;
                isDraggingPanel = false;
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                sf::Vector2i mPos = sf::Mouse::getPosition();

                if (isDraggingPanel)
                {
                    sf::Vector2i winPos = window.getPosition();
                    sf::Vector2i localMouse = mPos - winPos;

                    panelPos = sf::Vector2f((float)localMouse.x - dragOffset.x, (float)localMouse.y - dragOffset.y);

                    if(panelPos.x < 0) panelPos.x = 0;
                    if(panelPos.y < 0) panelPos.y = 0;
                    if(panelPos.x + panelBg.getSize().x > WINDOW_WIDTH) panelPos.x = WINDOW_WIDTH - panelBg.getSize().x;
                    if(panelPos.y + panelBg.getSize().y > WINDOW_HEIGHT) panelPos.y = WINDOW_HEIGHT - panelBg.getSize().y;
                }
                else if (isDraggingWindow)
                {
                    window.setPosition(mPos - dragOffset);
                }
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                useQuadtree = !useQuadtree;
                toggleText.setString(useQuadtree ? "[SPACE] Mode: Quadtree" : "[SPACE] Mode: Brute Force");
                toggleText.setFillColor(useQuadtree ? sf::Color::Cyan : sf::Color(180, 180, 180));
            }
        }

        bool hoverPanel = panelBg.getGlobalBounds().contains((float)sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y);
        bool hoverSim = simArea.getGlobalBounds().contains((float)sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y);
        bool hoverClose = closeBtn.getGlobalBounds().contains((float)sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y);

        if (hoverClose || btnMinimize.isMouseOver(window))
        {
            window.setMouseCursor(handCursor);
        }
        else if (!isPanelMinimized && (ballSlider.isMouseOver(window) || speedSlider.isMouseOver(window) || gravityBtn.isMouseOver(window)))
        {
            window.setMouseCursor(handCursor);
        }
        else if (hoverPanel)
        {
            window.setMouseCursor(moveCursor);
        }
        else if (!hoverSim)
        {
            window.setMouseCursor(moveCursor);
        }
        else
        {
            window.setMouseCursor(arrowCursor);
        }

        int targetBalls = (int)ballSlider.currentValue;
        if (balls.size() != targetBalls) adjustBallCount(balls, targetBalls, pastelColors);
        float speedMult = speedSlider.currentValue;

        for (auto& ball : balls)
        {
            if (gravityBtn.state) ball.velocity.y += 0.15f * speedMult;

            ball.shape.move(ball.velocity * speedMult);
            sf::Vector2f pos = ball.shape.getPosition();

            if (pos.x < BALL_RADIUS) { ball.velocity.x = std::abs(ball.velocity.x); ball.shape.setPosition(BALL_RADIUS, pos.y); }
            if (pos.x > WINDOW_WIDTH - BALL_RADIUS) { ball.velocity.x = -std::abs(ball.velocity.x); ball.shape.setPosition(WINDOW_WIDTH - BALL_RADIUS, pos.y); }
            if (pos.y < BALL_RADIUS) { ball.velocity.y = std::abs(ball.velocity.y); ball.shape.setPosition(pos.x, BALL_RADIUS); }

            if (pos.y > WINDOW_HEIGHT - BALL_RADIUS)
            {
                if (gravityBtn.state)
                {
                    ball.velocity.y = -std::abs(ball.velocity.y) * 0.70f;
                    ball.velocity.x *= 0.96f;
                    if (std::abs(ball.velocity.y) < 1.0f) ball.velocity.y = 0;
                }
                else
                {
                    ball.velocity.y = -std::abs(ball.velocity.y);
                }
                ball.shape.setPosition(pos.x, WINDOW_HEIGHT - BALL_RADIUS);
            }
        }

        if (!useQuadtree)
        {
            for (size_t i = 0; i < balls.size(); ++i)
                for (size_t j = i + 1; j < balls.size(); ++j)
                    solveBallCollision(balls[i], balls[j]);
        }
        else
        {
            Rect boundary = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
            Quadtree qt(boundary, 4);
            for (size_t i = 0; i < balls.size(); ++i) qt.insert(i, balls);
            for (size_t i = 0; i < balls.size(); ++i)
            {
                std::vector<int> candidates;
                candidates.reserve(32);
                Rect range = {balls[i].shape.getPosition().x, balls[i].shape.getPosition().y, BALL_RADIUS * 2, BALL_RADIUS * 2};
                qt.query(range, candidates);
                for (int j : candidates)
                {
                    if (i < j) solveBallCollision(balls[i], balls[j]);
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(simArea);
        for (const auto& ball : balls) window.draw(ball.shape);

        window.draw(panelBg);
        window.draw(panelHeader);
        btnMinimize.draw(window);

        if (!isPanelMinimized)
        {
            ballSlider.draw(window);
            speedSlider.draw(window);
            gravityBtn.draw(window);
            window.draw(toggleText);

            sf::Time elapsedTime = clock.restart();
            float fps = 1.0f / elapsedTime.asSeconds();
            fpsText.setString("FPS: " + std::to_string((int)fps));
            window.draw(fpsText);
        }

        window.draw(closeBtn);
        window.draw(closeText);

        window.display();
    }

    return 0;
}