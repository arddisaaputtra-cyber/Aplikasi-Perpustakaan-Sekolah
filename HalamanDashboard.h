#pragma once

#include "Database.h"
#include "Tema.h"

#include <QtSql>
#include <QtWidgets>

class HalamanDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit HalamanDashboard(QWidget *parent = nullptr) : QWidget(parent)
    {
        model = new QStandardItemModel(this);
        bangunUI();
        muatData();
    }

signals:
    void pindahKe(int indeks);   // dipakai tombol cepat di banner

public slots:
    void muatData()
    {
        // ---- Sapaan sesuai jam ----
        const int jam = QTime::currentTime().hour();
        QString sapa = "Selamat malam";
        if (jam >= 4 && jam < 11)
            sapa = "Selamat pagi";
        else if (jam >= 11 && jam < 15)
            sapa = "Selamat siang";
        else if (jam >= 15 && jam < 18)
            sapa = "Selamat sore";

        lblSapa->setText(sapa + ", Pustakawan 👋");
        lblTanggal->setText(tanggalIndonesia(QDate::currentDate()));

        // ---- Angka statistik ----
        QSqlQuery q("SELECT "
                    "(SELECT COUNT(*) FROM buku), "
                    "(SELECT ISNULL(SUM(stok), 0) FROM buku), "
                    "(SELECT COUNT(*) FROM anggota), "
                    "(SELECT COUNT(*) FROM peminjaman WHERE tgl_kembali IS NULL), "
                    "(SELECT COUNT(*) FROM peminjaman WHERE tgl_kembali IS NULL "
                    "AND CAST(GETDATE() AS DATE) > tgl_jatuh_tempo)");
        if (q.next()) {
            lblBuku->setText(QString::number(q.value(0).toInt()));
            lblStok->setText(QString::number(q.value(1).toInt()));
            lblAnggota->setText(QString::number(q.value(2).toInt()));
            lblDipinjam->setText(QString::number(q.value(3).toInt()));
            lblTerlambat->setText(QString::number(q.value(4).toInt()));
        }
        q.finish();

        // ---- Grafik donat: status peminjaman ----
        QSqlQuery qs;
        if (qs.exec("SELECT "
                    "SUM(CASE WHEN tgl_kembali IS NOT NULL THEN 1 ELSE 0 END), "
                    "SUM(CASE WHEN tgl_kembali IS NULL "
                    "AND CAST(GETDATE() AS DATE) <= tgl_jatuh_tempo THEN 1 ELSE 0 END), "
                    "SUM(CASE WHEN tgl_kembali IS NULL "
                    "AND CAST(GETDATE() AS DATE) > tgl_jatuh_tempo THEN 1 ELSE 0 END) "
                    "FROM peminjaman")
            && qs.next()) {
            donut->setData({"Dipinjam", "Terlambat", "Dikembalikan"},
                           {qs.value(1).toInt(), qs.value(2).toInt(), qs.value(0).toInt()},
                           {QColor("#C9A24D"), QColor("#B85042"), QColor("#2F5D50")});
        }
        qs.finish();

        // ---- Grafik batang: koleksi per kategori ----
        QList<QPair<QString, int>> kategori;
        QSqlQuery qk;
        if (qk.exec("SELECT TOP 5 ISNULL(NULLIF(kategori, ''), 'Lainnya') AS k, COUNT(*) AS n "
                    "FROM buku "
                    "GROUP BY ISNULL(NULLIF(kategori, ''), 'Lainnya') "
                    "ORDER BY COUNT(*) DESC, ISNULL(NULLIF(kategori, ''), 'Lainnya')")) {
            while (qk.next())
                kategori.append(qMakePair(qk.value(0).toString(), qk.value(1).toInt()));
        }
        qk.finish();
        batang->setData(kategori);

        // ---- Tabel peminjaman aktif (jatuh tempo terdekat) ----
        QSqlQuery t;
        if (!t.exec("SELECT TOP 10 a.nama, b.judul, "
                    "CONVERT(VARCHAR(10), p.tgl_jatuh_tempo, 105), "
                    "CASE WHEN CAST(GETDATE() AS DATE) > p.tgl_jatuh_tempo "
                    "THEN 'Terlambat' ELSE 'Dipinjam' END "
                    "FROM peminjaman p "
                    "JOIN anggota a ON a.id = p.id_anggota "
                    "JOIN buku b ON b.id = p.id_buku "
                    "WHERE p.tgl_kembali IS NULL "
                    "ORDER BY p.tgl_jatuh_tempo ASC, p.id ASC"))
            return;

        Database::isiModel(model, t, {"Anggota", "Buku", "Jatuh Tempo", "Status"});
        for (int r = 0; r < model->rowCount(); ++r)
            warnaiStatus(model->item(r, 3));
        rapikanKolom(tabel, 1, false);
    }

private:
    void bangunUI()
    {
        auto *utama = new QVBoxLayout(this);
        utama->setContentsMargins(0, 0, 0, 0);
        utama->setSpacing(16);

        // ================= BANNER =================
        auto *banner = new QFrame;
        banner->setObjectName("banner");
        tambahBayangan(banner, 26, 4, 45);

        auto *bh = new QHBoxLayout(banner);
        bh->setContentsMargins(30, 22, 30, 22);
        bh->setSpacing(12);

        auto *teks = new QVBoxLayout;
        teks->setSpacing(3);
        lblSapa = buatLabel("", "bannerJudul");
        lblTanggal = buatLabel("", "bannerSub");
        teks->addWidget(lblSapa);
        teks->addWidget(lblTanggal);
        teks->addSpacing(2);
        teks->addWidget(buatLabel("\"Buku adalah jendela dunia.\"", "bannerQuote"));
        bh->addLayout(teks, 1);

        auto *btnBuku = new QPushButton("📖  Tambah Buku");
        btnBuku->setObjectName("bannerBtn2");
        btnBuku->setCursor(Qt::PointingHandCursor);
        auto *btnPinjam = new QPushButton("＋  Peminjaman Baru");
        btnPinjam->setObjectName("bannerBtn");
        btnPinjam->setCursor(Qt::PointingHandCursor);
        bh->addWidget(btnBuku);
        bh->addWidget(btnPinjam);

        connect(btnBuku, &QPushButton::clicked, this, [this]() { emit pindahKe(1); });
        connect(btnPinjam, &QPushButton::clicked, this, [this]() { emit pindahKe(3); });

        utama->addWidget(banner);

        // ================= KARTU STATISTIK =================
        auto buatStat = [&](const QString &ikon, const QString &judul, QLabel *&nilai,
                            const QString &warnaIkon, bool merah) {
            auto *kartu = buatKartu();
            auto *h = new QHBoxLayout(kartu);
            h->setContentsMargins(16, 14, 16, 14);
            h->setSpacing(12);

            auto *ik = new QLabel(ikon);
            ik->setFixedSize(46, 46);
            ik->setAlignment(Qt::AlignCenter);
            ik->setStyleSheet(QString("background: %1; border-radius: 12px; font-size: 18pt;")
                                  .arg(warnaIkon));
            h->addWidget(ik);

            auto *v = new QVBoxLayout;
            v->setSpacing(0);
            v->addWidget(buatLabel(judul, "statJudul"));
            nilai = buatLabel("0", merah ? "statNilaiMerah" : "statNilai");
            v->addWidget(nilai);
            h->addLayout(v);
            h->addStretch();
            return kartu;
        };

        auto *barisStat = new QHBoxLayout;
        barisStat->setSpacing(16);
        barisStat->addWidget(buatStat("📚", "Judul Buku", lblBuku, "#DCE9E2", false));
        barisStat->addWidget(buatStat("📦", "Stok Tersedia", lblStok, "#F3E5BE", false));
        barisStat->addWidget(buatStat("👥", "Anggota", lblAnggota, "#E6DCF0", false));
        barisStat->addWidget(buatStat("📖", "Sedang Dipinjam", lblDipinjam, "#D6E6EA", false));
        barisStat->addWidget(buatStat("⏰", "Terlambat", lblTerlambat, "#F5DDD8", true));
        utama->addLayout(barisStat);

        // ================= BAGIAN BAWAH =================
        auto *bawah = new QHBoxLayout;
        bawah->setSpacing(16);

        // ---- Kiri: tabel peminjaman aktif ----
        auto *kartuTabel = buatKartu();
        auto *tl = new QVBoxLayout(kartuTabel);
        tl->setContentsMargins(20, 16, 20, 16);
        tl->setSpacing(10);

        auto *atas = new QHBoxLayout;
        atas->addWidget(buatLabel("Peminjaman Aktif", "judulKartu"));
        atas->addWidget(buatLabel("jatuh tempo terdekat", "info"));
        atas->addStretch();
        tl->addLayout(atas);

        tabel = new QTableView;
        aturTabel(tabel, model);
        tl->addWidget(tabel, 1);
        bawah->addWidget(kartuTabel, 3);

        // ---- Kanan: dua grafik ----
        auto *kanan = new QVBoxLayout;
        kanan->setSpacing(16);

        auto *kartuDonut = buatKartu();
        auto *kd = new QVBoxLayout(kartuDonut);
        kd->setContentsMargins(20, 16, 20, 14);
        kd->setSpacing(6);
        kd->addWidget(buatLabel("Status Peminjaman", "judulKartu"));
        donut = new GrafikDonut;
        kd->addWidget(donut, 1);
        kanan->addWidget(kartuDonut, 1);

        auto *kartuBatang = buatKartu();
        auto *kb = new QVBoxLayout(kartuBatang);
        kb->setContentsMargins(20, 16, 20, 14);
        kb->setSpacing(6);
        kb->addWidget(buatLabel("Koleksi per Kategori", "judulKartu"));
        batang = new GrafikBatang;
        kb->addWidget(batang, 1);
        kanan->addWidget(kartuBatang, 1);

        bawah->addLayout(kanan, 2);
        utama->addLayout(bawah, 1);
    }

    QLabel *lblSapa = nullptr;
    QLabel *lblTanggal = nullptr;
    QLabel *lblBuku = nullptr;
    QLabel *lblStok = nullptr;
    QLabel *lblAnggota = nullptr;
    QLabel *lblDipinjam = nullptr;
    QLabel *lblTerlambat = nullptr;
    QTableView *tabel = nullptr;
    GrafikDonut *donut = nullptr;
    GrafikBatang *batang = nullptr;
    QStandardItemModel *model = nullptr;
};