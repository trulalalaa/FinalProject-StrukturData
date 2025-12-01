# Simulasi Partikel dengan menggunakan Brute Force dan QuadTree

Proyek ini adalah simulasi partikel 2D interaktif yang dibangun menggunakan C++ dan SFML. Tujuan utamanya adalah memvisualisasikan dan membandingkan performa antara metode deteksi tabrakan **Brute Force** dengan metode **Quadtree**.

---

## Fitur Utama

*   **Simulasi**: Mensimulasikan ratusan hingga ribuan partikel yang saling bertumbukan
*   **Dual Algorithm Mode**:
    *   **Brute Force (O(N^2))**: Metode yang mengecek semua kemungkinan pasangan bola satu sama lain.
    *   **Quadtree (O(N \log N))**: Metode yang mengoptimasi yang membagi area menjadi kuadran untuk mengurangi jumlah pengecekan secara drastis.
*   **Control Panel**: Berfungsi untuk mengatur jumlah bola, kecepatan, dan gravitasi.
  
---

## Struktur Kode

Kode ini ditulis dalam satu file (`main.cpp`) 

### 1. Struktur Data & Class

Berikut adalah komponen utama penyusun program:

#### A. Komponen Logika & Fisika
*   **`struct Ball`**:
    *   Merepresentasikan objek partikel.
    *   Menyimpan properti visual (`sf::CircleShape`) dan fisik (`sf::Vector2f velocity`).
*   **`struct Rect`**:
    *   Helper geometry untuk mendefinisikan area persegi (*Axis-Aligned Bounding Box*).
    *   Digunakan oleh Quadtree untuk menentukan apakah sebuah bola masuk dalam wilayahnya (`contains`) atau apakah wilayahnya bersinggungan dengan wilayah lain (`intersects`).
*   **`class Quadtree`**:
    *   Struktur data pohon spasial.
    *   **Logika Utama**: Membagi layar menjadi 4 node anak (NW, NE, SW, SE) jika kapasitas node penuh.
    *   **Fungsi `query()`**: Mengambil daftar bola potensial di area tertentu, sehingga program tidak perlu mengecek bola yang jauh.

#### B. Komponen User Interface (UI)
*   **`struct Button`**: Menangani rendering tombol dan deteksi klik mouse sederhana.
*   **`struct Slider`**: Mengonversi posisi mouse (x) menjadi nilai numerik (float) untuk mengatur parameter simulasi.

---

### 2. Implementasi Algoritma

Perbedaan utama performa terletak pada bagaimana program mendeteksi tabrakan di dalam *Main Loop*.

#### A. Algoritma Brute Force 
Dijalankan saat `useQuadtree = false`. 

```cpp
if (!useQuadtree) {
    for (size_t i = 0; i < balls.size(); ++i)
        for (size_t j = i + 1; j < balls.size(); ++j)
            solveBallCollision(balls[i], balls[j]);
}
```
#### B. Algoritma QuadTree 
Dijalankan saat `useQuadtree = true`. 

```cpp
else {
    Rect boundary = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    Quadtree qt(boundary, 4); 

    // Memasukkan bola 
    for (size_t i = 0; i < balls.size(); ++i) qt.insert(i, balls);

    // Query & Check Phase
    for (size_t i = 0; i < balls.size(); ++i)
    {
        std::vector<int> candidates;
        candidates.reserve(32); // Optimasi alokasi memori
        
        // Buat area pencarian kecil di sekitar bola saat ini
        Rect range = {balls[i].shape.getPosition().x, balls[i].shape.getPosition().y, BALL_RADIUS * 2, BALL_RADIUS * 2}; 
        
        // Minta tree memberikan daftar bola yang hanya ada di area tersebut
        qt.query(range, candidates);

        // Cek tabrakan hanya dengan bola dalam area 
        for (int j : candidates)
        {
            if (i < j) solveBallCollision(balls[i], balls[j]);
        }
    }
}
```
