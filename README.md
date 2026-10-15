# Aplikasi Perpustakaan Sekolah

Aplikasi desktop pengelolaan perpustakaan sekolah.
Dibuat dengan **C++**, **Qt 6 (Widgets)**, dan **SQL Server**.

## Fitur

- **Login** dengan kata sandi yang di-hash (SHA-256)
- **Dashboard**: statistik, grafik aktivitas 7 hari, status peminjaman, buku terpopuler, anggota teraktif, dan pengingat jatuh tempo
- **Data Buku** dan **Data Anggota**: tambah, ubah, hapus, cari, ekspor CSV
- **Peminjaman dan pengembalian** dengan denda otomatis (Rp 1.000 per hari, maksimal 3 buku per anggota)
- **Laporan PDF** berkop surat: peminjaman, keterlambatan, daftar buku, daftar anggota
- **Kartu anggota** ber-barcode (simpan sebagai PNG atau PDF siap cetak)

## Kebutuhan

- Qt 6.10 (MinGW 64-bit) dan Qt Creator
- SQL Server (contoh: instance `localhost\SQLEXPRESS07`)
- ODBC Driver for SQL Server

## Cara menjalankan

1. Clone repo ini, lalu buka `CMakeLists.txt` di Qt Creator.
2. Pilih kit **Desktop Qt 6.x MinGW 64-bit**.
3. Jika nama server SQL berbeda, ubah baris `server` di `Database.h`.
4. Build dan Run. Database `PerpustakaanDB` beserta semua tabel dibuat otomatis.

## Login

Kata sandi awal: `admin123`. Username bebas, dan nama yang diketik tampil di dashboard.

## Struktur file

| File | Fungsi |
|---|---|
| `Database.h` | Koneksi SQL Server, pembuatan tabel, login |
| `Tema.h` | Desain (QSS), grafik, dan fungsi bantu tampilan |
| `LoginDialog.h` | Layar login |
| `HalamanDashboard.h` | Dashboard |
| `HalamanBuku.h`, `HalamanAnggota.h`, `HalamanPinjam.h` | Halaman data dan transaksi |
| `HalamanLaporan.h` | Laporan PDF dan kartu anggota |
| `mainwindow.h`, `mainwindow.cpp`, `main.cpp` | Jendela utama dan titik masuk program |
| `CMakeLists.txt` | Konfigurasi build |
