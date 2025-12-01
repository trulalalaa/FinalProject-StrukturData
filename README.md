# 2D Particle Physics Simulation: Brute Force vs Quadtree

Proyek ini adalah simulasi fisika partikel 2D interaktif yang dibangun menggunakan C++ dan SFML. Tujuan utamanya adalah memvisualisasikan dan membandingkan performa antara metode deteksi tabrakan **Brute Force** dengan metode optimasi spasial menggunakan **Quadtree**.

## 📋 Fitur Utama

*   **Simulasi Real-time**: Menangani ribuan partikel (bola) dengan fisika tumbukan elastis.
*   **Dual Algorithm Mode**:
    *   **Brute Force ($O(N^2)$)**: Metode naif yang mengecek semua kemungkinan pasangan bola.
    *   **Quadtree ($O(N \log N)$)**: Metode optimasi yang membagi area menjadi kuadran untuk mengurangi jumlah pengecekan secara drastis.
*   **Interactive UI**: Panel kontrol custom (tanpa library GUI eksternal) untuk mengatur jumlah bola, kecepatan, dan gravitasi.
*   **Visual Debugging**: Fitur untuk melihat garis grid Quadtree secara real-time.

---

## 🛠️ Analisis Struktur Kode

Kode ini ditulis dalam satu file (`main.cpp`) dengan pendekatan *prosedural* yang dibantu oleh *struct/class* untuk pengelompokan data.

### 1. Struktur Data & Class

Berikut adalah komponen utama penyusun program:

#### A. Komponen Logika & Fisika
*   **`struct Ball`**:
    *   Merepresentasikan objek partikel.
    *   Menyimpan properti visual (`sf::CircleShape`) dan fisika (`sf::Vector2f velocity`).
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

#### A. Algoritma Brute Force (Slow)
Dijalankan saat `useQuadtree = false`.

```cpp
if (!useQuadtree) {
    for (size_t i = 0; i < balls.size(); ++i)
        for (size_t j = i + 1; j < balls.size(); ++j)
            solveBallCollision(balls[i], balls[j]);
}
