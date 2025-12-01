#include "extend.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>
#include <random>

using namespace std;

// --- STRUKTUR DATA ---
// Kita gabungkan Fisika (Particle) dan Gambar (Shape) jadi satu paket
struct Object {
    Particle p;
    sf::CircleShape shape;
};

// --- KELAS SIMULASI UTAMA ---
class Simulation {
private:
    sf::RenderWindow window;
    vector<Object> objects; // List semua bola ada disini
    
    sf::Font font;
    sf::Text textInfo;
    
    // Random generator
    mt19937 gen;

public:
    Simulation() : gen(random_device{}()) {
        // 1. Setup Window
        window.create(sf::VideoMode::getDesktopMode(), "Simulasi Tabrakan", sf::Style::Fullscreen);
        window.setFramerateLimit(60);

        // 2. Setup Font & Text
        if (!font.loadFromFile("assets/font/MontserratBlack-3zOvZ.ttf")) {
            cerr << "Gagal load font, pastikan path benar.\n";
        }
        textInfo.setFont(font);
        textInfo.setCharacterSize(18);
        textInfo.setFillColor(sf::Color::White);
        textInfo.setPosition(10, 10);

        // 3. Tambah partikel awal
        addParticles(500);
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            processEvents();
            
            // Hitung delta time (dt)
            double dt = clock.restart().asSeconds();
            if (dt > 0.1) dt = 0.1; // Cegah lag spike

            update(dt);
            render(1.0 / dt); // Kirim FPS ke render
        }
    }

private:
    // --- FUNGSI INPUT ---
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) window.close();
                
                // Tambah Partikel (+)
                if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
                    addParticles(100);
                    cout << "Total: " << objects.size() << endl;
                }
                // Hapus Partikel (-)
                if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Hyphen) {
                    removeParticles(100);
                    cout << "Total: " << objects.size() << endl;
                }
            }
        }
    }

    // --- FUNGSI LOGIKA / FISIKA ---
    void update(double dt) {
        double winW = window.getSize().x;
        double winH = window.getSize().y;

        // 1. Update Posisi & Cek Tabrakan Dinding
        for (auto& obj : objects) {
            obj.p.update(dt, winW, winH); // Update fisika
            obj.shape.setPosition(obj.p.getX(), obj.p.getY()); // Update gambar
        }

        // 2. Cek Tabrakan Antar Bola (Brute Force O(n^2))
        for (size_t i = 0; i < objects.size(); ++i) {
            for (size_t j = i + 1; j < objects.size(); ++j) {
                if (objects[i].p.isColliding(objects[j].p)) {
                    objects[i].p.resolveCollision(objects[j].p);
                }
            }
        }
    }

    // --- FUNGSI GAMBAR ---
    void render(double fps) {
        window.clear(sf::Color(30, 30, 30)); // Warna background abu gelap

        // Gambar semua bola
        for (const auto& obj : objects) {
            window.draw(obj.shape);
        }

        // Update Text Info
        textInfo.setString(
            "FPS: " + to_string((int)fps) + "\n" +
            "Jumlah: " + to_string(objects.size()) + "\n" +
            "Kontrol: [+] Tambah  [-] Hapus  [ESC] Keluar"
        );
        window.draw(textInfo);

        window.display();
    }

    // --- HELPER: MEMBUAT PARTIKEL ---
    void addParticles(int count) {
        double winW = window.getSize().x;
        double winH = window.getSize().y;
        
        // Setup distribusi random
        uniform_real_distribution<double> xDist(50, winW - 50);
        uniform_real_distribution<double> yDist(50, winH - 50);
        uniform_real_distribution<double> vDist(-100, 100);
        uniform_real_distribution<double> massDist(0.1, 5.0); // Massa 0.1 - 5.0

        for (int i = 0; i < count; ++i) {
            double mass = massDist(gen);
            double x = xDist(gen);
            double y = yDist(gen);

            // Buat objek Particle baru
            Particle newP(x, y, mass);
            newP.setVelocity(vDist(gen), vDist(gen));

            // Buat objek Shape baru (Visual)
            sf::CircleShape newShape(newP.getRadius());
            newShape.setOrigin(newP.getRadius(), newP.getRadius());
            newShape.setPosition(x, y);

            // Warnai berdasarkan urutan (biar warna-warni)
            int hue = objects.size() + i;
            newShape.setFillColor(sf::Color((hue * 50) % 255, (hue * 80) % 255, (hue * 30) % 255));

            // Simpan ke list
            objects.push_back({newP, newShape});
        }
    }

    void removeParticles(int count) {
        // Hapus dari belakang (LIFO), jauh lebih cepat & simpel daripada sorting tanggal
        for (int i = 0; i < count && !objects.empty(); ++i) {
            objects.pop_back();
        }
    }
};

// --- MAIN FUNCTION ---
int main() {
    // Kode utama jadi sangat bersih
    Simulation sim;
    sim.run();
    return 0;
}