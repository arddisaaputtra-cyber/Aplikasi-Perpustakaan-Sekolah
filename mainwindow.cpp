#include "mainwindow.h"

#include "Database.h"
#include "Tema.h"
#include "HalamanDashboard.h"
#include "HalamanBuku.h"
#include "HalamanAnggota.h"
#include "HalamanPinjam.h"
#include "HalamanLaporan.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Aplikasi Perpustakaan");
    resize(1280, 720);
    setMinimumSize(1100, 640);
    buatUI();
}

void MainWindow::buatUI()
{
    auto *root = new QWidget;
    setCentralWidget(root);

    auto *utama = new QHBoxLayout(root);
    utama->setContentsMargins(0, 0, 0, 0);
    utama->setSpacing(0);

    // ================= MENU SAMPING (rak kayu) =================
    auto *sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setAttribute(Qt::WA_StyledBackground, true);
    sidebar->setFixedWidth(250);

    auto *sl = new QVBoxLayout(sidebar);
    sl->setContentsMargins(18, 24, 18, 18);
    sl->setSpacing(6);

    // ---- Logo ----
    auto *lencana = new QLabel("📚");
    lencana->setObjectName("lencana");
    lencana->setFixedSize(46, 46);
    lencana->setAlignment(Qt::AlignCenter);

    auto *teksLogo = new QVBoxLayout;
    teksLogo->setSpacing(0);
    teksLogo->addWidget(buatLabel("Perpustakaan", "logo"));
    teksLogo->addWidget(buatLabel("Sistem Informasi", "logoSub"));

    auto *barisLogo = new QHBoxLayout;
    barisLogo->setSpacing(12);
    barisLogo->addWidget(lencana);
    barisLogo->addLayout(teksLogo);
    barisLogo->addStretch();
    sl->addLayout(barisLogo);

    sl->addSpacing(22);
    sl->addWidget(buatLabel("MENU", "menuJudul"));
    sl->addSpacing(2);

    // ---- Tombol menu ----
    const QStringList menu = {"📊   Dashboard", "📖   Data Buku", "👥   Data Anggota",
                              "🔄   Peminjaman", "🖨   Laporan & Cetak"};
    for (int i = 0; i < menu.size(); ++i) {
        auto *b = new QPushButton(menu[i]);
        b->setObjectName("nav");
        b->setCheckable(true);
        b->setAutoExclusive(true);
        b->setCursor(Qt::PointingHandCursor);
        sl->addWidget(b);
        tombolNav.append(b);
        connect(b, &QPushButton::clicked, this, [this, i]() { pindahHalaman(i); });
    }

    sl->addStretch();

    // ---- Kartu pengguna ----
    auto *kartuUser = new QFrame;
    kartuUser->setObjectName("jamKartu");
    auto *ul = new QVBoxLayout(kartuUser);
    ul->setContentsMargins(12, 10, 12, 10);
    ul->setSpacing(8);

    auto *avatar = new QLabel(Database::namaPengguna.left(1).toUpper());
    avatar->setObjectName("avatar");
    avatar->setFixedSize(38, 38);
    avatar->setAlignment(Qt::AlignCenter);

    auto *lblNama = buatLabel(Database::namaPengguna, "userNama");
    auto *lblPeran = buatLabel(Database::peranPengguna, "userPeran");
    auto *teksUser = new QVBoxLayout;
    teksUser->setSpacing(0);
    teksUser->addWidget(lblNama);
    teksUser->addWidget(lblPeran);

    auto *barisUser = new QHBoxLayout;
    barisUser->setSpacing(10);
    barisUser->addWidget(avatar);
    barisUser->addLayout(teksUser, 1);
    ul->addLayout(barisUser);

    auto *btnKeluar = new QPushButton("Keluar");
    btnKeluar->setObjectName("keluar");
    btnKeluar->setCursor(Qt::PointingHandCursor);
    ul->addWidget(btnKeluar);
    sl->addWidget(kartuUser);

    connect(btnKeluar, &QPushButton::clicked, this, [this]() {
        if (konfirmasi(this, "Keluar dari akun ini?"))
            emit logout();
    });

    // ---- Jam digital ----
    auto *kartuJam = new QFrame;
    kartuJam->setObjectName("jamKartu");
    auto *jl = new QVBoxLayout(kartuJam);
    jl->setContentsMargins(16, 10, 16, 10);
    jl->setSpacing(0);

    auto *lblJam = buatLabel("", "jam");
    auto *lblTgl = buatLabel("", "jamTanggal");
    jl->addWidget(lblJam);
    jl->addWidget(lblTgl);
    sl->addWidget(kartuJam);

    auto perbaruiJam = [lblJam, lblTgl]() {
        const QDateTime sekarang = QDateTime::currentDateTime();
        lblJam->setText(sekarang.toString("HH:mm:ss"));
        lblTgl->setText(tanggalIndonesia(sekarang.date()));
    };
    perbaruiJam();

    auto *pewaktu = new QTimer(this);
    connect(pewaktu, &QTimer::timeout, this, perbaruiJam);
    pewaktu->start(1000);

    sl->addSpacing(4);
    auto *foot = buatLabel(QString("Denda %1 / hari  •  Maks. %2 buku per anggota")
                               .arg(rupiah(Database::dendaPerHari))
                               .arg(Database::maksPinjam),
                           "footerSide");
    foot->setWordWrap(true);
    sl->addWidget(foot);

    // ================= ISI HALAMAN =================
    auto *konten = new QWidget;
    konten->setObjectName("konten");
    konten->setAttribute(Qt::WA_StyledBackground, true);

    auto *kl = new QVBoxLayout(konten);
    kl->setContentsMargins(30, 24, 30, 24);

    tumpukan = new QStackedWidget;
    kl->addWidget(tumpukan);

    halDashboard = new HalamanDashboard;
    halBuku = new HalamanBuku;
    halAnggota = new HalamanAnggota;
    halPinjam = new HalamanPinjam;
    halLaporan = new HalamanLaporan;

    tumpukan->addWidget(halDashboard);
    tumpukan->addWidget(halBuku);
    tumpukan->addWidget(halAnggota);
    tumpukan->addWidget(halPinjam);
    tumpukan->addWidget(halLaporan);

    utama->addWidget(sidebar);
    utama->addWidget(konten, 1);

    // Jika data berubah di satu halaman, semua halaman ikut diperbarui
    connect(halBuku, &HalamanBuku::dataBerubah, this, &MainWindow::segarkanSemua);
    connect(halAnggota, &HalamanAnggota::dataBerubah, this, &MainWindow::segarkanSemua);
    connect(halPinjam, &HalamanPinjam::dataBerubah, this, &MainWindow::segarkanSemua);

    // Tombol cepat di banner dashboard
    connect(halDashboard, &HalamanDashboard::pindahKe, this, &MainWindow::pindahHalaman);

    tombolNav[0]->setChecked(true);
    tumpukan->setCurrentIndex(0);
}

void MainWindow::pindahHalaman(int indeks)
{
    const bool berganti = tumpukan->currentIndex() != indeks;
    tumpukan->setCurrentIndex(indeks);
    tombolNav[indeks]->setChecked(true);
    segarkanSemua();

    if (berganti)
        animasiMuncul(tumpukan->currentWidget());
}

void MainWindow::segarkanSemua()
{
    halDashboard->muatData();
    halBuku->muatData();
    halAnggota->muatData();
    halPinjam->segarkan();
    halLaporan->segarkan();
}