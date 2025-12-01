# FinalProject-StrukturData
Implementasi Algoritma Deteksi Tabrakan
Bagian ini adalah inti dari simulasi, terletak di dalam loop utama (while window.isOpen()) pada bagian Logic collision.
A. Algoritma Brute Force (Naif)
Algoritma ini dijalankan ketika useQuadtree == false.
Lokasi dalam kode:
code
C++
if (!useQuadtree)
{
    for (size_t i = 0; i < balls.size(); ++i)
        for (size_t j = i + 1; j < balls.size(); ++j)
            solveBallCollision(balls[i], balls[j]);
}
Penjelasan:
Menggunakan Nested Loop (perulangan bersarang).
Setiap bola i dicek terhadap setiap bola j.
Kompleksitas: 
O
(
N
2
)
O(N 
2
 )
. Jika ada 1.000 bola, sistem melakukan hampir 500.000 pengecekan per frame. Ini penyebab penurunan FPS drastis pada jumlah partikel banyak.
