#pragma once

#include "Database.h"
#include "Tema.h"

#include <QtSql>
#include <QtWidgets>

class HalamanPinjam : public QWidget
{
    Q_OBJECT

public:
    explicit HalamanPinjam(QWidget *parent = nullptr) : QWidget(parent)
    {
        model = new QStandardItemModel(this);
        bangunUI();
        segarkan();
    }

signals:
    void dataBerubah();

public slots:
    void segarkan()
    {
        muatPilihan();
        muatData();
    }

    // Mengisi pilihan anggota dan buku (hanya buku yang stoknya masih ada)
    void muatPilihan()
    {
        const QVariant anggotaLama = cmbAnggota->currentData();
        const QVariant bukuLama = cmbBuku->currentData();
        cmbAnggota->clear();
        cmbBuku->clear();

        QSqlQuery qa;
        if (qa.exec("SELECT id, kode, nama FROM anggota ORDER BY nama")) {
            while (qa.next())
                cmbAnggota->addItem(qa.value(1).toString() + " - " + qa.value(2).toString(),
                                    qa.value(0));
        }
        qa.finish();

        QSqlQuery qb;
        if (qb.exec("SELECT id, kode, judul, stok FROM buku WHERE stok > 0 ORDER BY judul")) {
            while (qb.next())
                cmbBuku->addItem(QString("%1 - %2 (stok %3)")
                                     .arg(qb.value(1).toString(),
                                          qb.value(2).toString(),
                                          qb.value(3).toString()),
                                 qb.value(0));
        }
        qb.finish();

        const int ia = cmbAnggota->findData(anggotaLama);
        if (ia >= 0)
            cmbAnggota->setCurrentIndex(ia);
        const int ib = cmbBuku->findData(bukuLama);
        if (ib >= 0)
            cmbBuku->setCurrentIndex(ib);
    }

    void muatData()
    {
        idTerpilih = -1;
        statusTerpilih.clear();

        const QString kw = edtCari ? edtCari->text().trimmed() : QString();
        const QString pola = "%" + kw + "%";
        const bool pakaiFilter = cmbFilter && cmbFilter->currentIndex() > 0;

        QString sql =
            "SELECT t.id, t.nama_anggota, t.judul_buku, t.pinjam, t.tempo, t.kembali, t.status, t.denda "
            "FROM ("
            "  SELECT p.id AS id, a.nama AS nama_anggota, a.kode AS kode_anggota, "
            "         b.judul AS judul_buku, b.kode AS kode_buku, "
            "         CONVERT(VARCHAR(10), p.tgl_pinjam, 105) AS pinjam, "
            "         CONVERT(VARCHAR(10), p.tgl_jatuh_tempo, 105) AS tempo, "
            "         ISNULL(CONVERT(VARCHAR(10), p.tgl_kembali, 105), '-') AS kembali, "
            "         CASE WHEN p.tgl_kembali IS NOT NULL THEN 'Dikembalikan' "
            "              WHEN CAST(GETDATE() AS DATE) > p.tgl_jatuh_tempo THEN 'Terlambat' "
            "              ELSE 'Dipinjam' END AS status, "
            "         CASE WHEN p.tgl_kembali IS NOT NULL THEN p.denda "
            "              WHEN CAST(GETDATE() AS DATE) > p.tgl_jatuh_tempo "
            "                   THEN DATEDIFF(DAY, p.tgl_jatuh_tempo, CAST(GETDATE() AS DATE)) * %1 "
            "              ELSE 0 END AS denda "
            "  FROM peminjaman p "
            "  JOIN anggota a ON a.id = p.id_anggota "
            "  JOIN buku b ON b.id = p.id_buku"
            ") t "
            "WHERE (t.nama_anggota LIKE :k1 OR t.judul_buku LIKE :k2 "
            "OR t.kode_anggota LIKE :k3 OR t.kode_buku LIKE :k4)";
        sql = sql.arg(Database::dendaPerHari);

        if (pakaiFilter)
            sql += " AND t.status = :st";
        sql += " ORDER BY t.id DESC";

        QSqlQuery q;
        q.prepare(sql);
        q.bindValue(":k1", pola);
        q.bindValue(":k2", pola);
        q.bindValue(":k3", pola);
        q.bindValue(":k4", pola);
        if (pakaiFilter)
            q.bindValue(":st", cmbFilter->currentText());

        if (!q.exec()) {
            QMessageBox::critical(this, "Gagal memuat data", q.lastError().text());
            return;
        }

        Database::isiModel(model, q, {"ID", "Anggota", "Buku", "Tgl Pinjam", "Jatuh Tempo",
                                      "Tgl Kembali", "Status", "Denda"});

        for (int r = 0; r < model->rowCount(); ++r) {
            warnaiStatus(model->item(r, 6));
            QStandardItem *d = model->item(r, 7);
            d->setText(rupiah(d->text().toInt()));
        }

        rapikanKolom(tabel, 2, true);
        lblJumlah->setText(QString::number(model->rowCount()) + " transaksi");
    }

private slots:
    void hitungTempo()
    {
        const QDate tempo = dtPinjam->date().addDays(spnLama->value());
        lblTempo->setText("Jatuh tempo: " + tempo.toString("dd-MM-yyyy"));
    }

    // ---------- Tombol "Pinjamkan Buku" ----------
    void pinjamkan()
    {
        if (cmbAnggota->currentIndex() < 0) {
            QMessageBox::information(this, "Info", "Belum ada data anggota. Tambahkan anggota dulu.");
            return;
        }
        if (cmbBuku->currentIndex() < 0) {
            QMessageBox::information(this, "Info", "Tidak ada buku yang tersedia (stok habis).");
            return;
        }

        const int idAnggota = cmbAnggota->currentData().toInt();
        const int idBuku = cmbBuku->currentData().toInt();

        // Batas jumlah pinjaman aktif per anggota
        int aktif = 0;
        QSqlQuery cek;
        cek.prepare("SELECT COUNT(*) FROM peminjaman WHERE id_anggota = :a AND tgl_kembali IS NULL");
        cek.bindValue(":a", idAnggota);
        if (cek.exec() && cek.next())
            aktif = cek.value(0).toInt();
        cek.finish();

        if (aktif >= Database::maksPinjam) {
            QMessageBox::warning(this, "Batas pinjam",
                                 QString("Anggota ini sudah meminjam %1 buku yang belum dikembalikan.")
                                     .arg(aktif));
            return;
        }

        // Buku yang sama belum boleh dipinjam dua kali oleh anggota yang sama
        int sama = 0;
        QSqlQuery cek2;
        cek2.prepare("SELECT COUNT(*) FROM peminjaman "
                     "WHERE id_anggota = :a AND id_buku = :b AND tgl_kembali IS NULL");
        cek2.bindValue(":a", idAnggota);
        cek2.bindValue(":b", idBuku);
        if (cek2.exec() && cek2.next())
            sama = cek2.value(0).toInt();
        cek2.finish();

        if (sama > 0) {
            QMessageBox::warning(this, "Sudah dipinjam",
                                 "Anggota ini masih meminjam buku yang sama dan belum mengembalikannya.");
            return;
        }

        // Kurangi stok dan catat peminjaman dalam satu transaksi
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.transaction()) {
            QMessageBox::critical(this, "Gagal", db.lastError().text());
            return;
        }

        QSqlQuery kurang;
        kurang.prepare("UPDATE buku SET stok = stok - 1 WHERE id = :id AND stok > 0");
        kurang.bindValue(":id", idBuku);
        if (!kurang.exec() || kurang.numRowsAffected() != 1) {
            db.rollback();
            QMessageBox::warning(this, "Gagal", "Stok buku sudah habis.");
            segarkan();
            return;
        }
        kurang.finish();

        const QDate tglPinjam = dtPinjam->date();
        const QDate tempo = tglPinjam.addDays(spnLama->value());

        QSqlQuery tambah;
        tambah.prepare("INSERT INTO peminjaman (id_buku, id_anggota, tgl_pinjam, tgl_jatuh_tempo, denda) "
                       "VALUES (:b, :a, :p, :t, 0)");
        tambah.bindValue(":b", idBuku);
        tambah.bindValue(":a", idAnggota);
        tambah.bindValue(":p", tglPinjam.toString("yyyy-MM-dd"));
        tambah.bindValue(":t", tempo.toString("yyyy-MM-dd"));
        if (!tambah.exec()) {
            const QString err = tambah.lastError().text();
            db.rollback();
            QMessageBox::critical(this, "Gagal", err);
            return;
        }
        tambah.finish();

        if (!db.commit()) {
            const QString err = db.lastError().text();
            db.rollback();
            QMessageBox::critical(this, "Gagal", err);
            return;
        }

        emit dataBerubah();
        QMessageBox::information(this, "Berhasil",
                                 "Peminjaman berhasil dicatat.\nJatuh tempo: " +
                                     tempo.toString("dd-MM-yyyy"));
    }

    // ---------- Tombol "Kembalikan Buku" ----------
    void kembalikan()
    {
        if (idTerpilih < 0) {
            QMessageBox::information(this, "Info", "Pilih data peminjaman pada tabel terlebih dahulu.");
            return;
        }
        if (statusTerpilih == "Dikembalikan") {
            QMessageBox::information(this, "Info", "Buku ini sudah dikembalikan.");
            return;
        }
        if (!konfirmasi(this, "Proses pengembalian buku ini?"))
            return;

        QSqlDatabase db = QSqlDatabase::database();
        if (!db.transaction()) {
            QMessageBox::critical(this, "Gagal", db.lastError().text());
            return;
        }

        // 1) Isi tanggal kembali dan hitung denda
        QSqlQuery u1;
        u1.prepare(QString("UPDATE peminjaman SET tgl_kembali = CAST(GETDATE() AS DATE), "
                           "denda = CASE WHEN CAST(GETDATE() AS DATE) > tgl_jatuh_tempo "
                           "THEN DATEDIFF(DAY, tgl_jatuh_tempo, CAST(GETDATE() AS DATE)) * %1 "
                           "ELSE 0 END "
                           "WHERE id = :id AND tgl_kembali IS NULL")
                       .arg(Database::dendaPerHari));
        u1.bindValue(":id", idTerpilih);
        if (!u1.exec() || u1.numRowsAffected() != 1) {
            db.rollback();
            QMessageBox::warning(this, "Gagal", "Data peminjaman tidak dapat diproses.");
            segarkan();
            return;
        }
        u1.finish();

        // 2) Kembalikan stok buku
        QSqlQuery u2;
        u2.prepare("UPDATE buku SET stok = stok + 1 "
                   "WHERE id = (SELECT id_buku FROM peminjaman WHERE id = :id)");
        u2.bindValue(":id", idTerpilih);
        if (!u2.exec()) {
            const QString err = u2.lastError().text();
            db.rollback();
            QMessageBox::critical(this, "Gagal", err);
            return;
        }
        u2.finish();

        // 3) Ambil denda untuk ditampilkan
        int denda = 0;
        QSqlQuery q3;
        q3.prepare("SELECT denda FROM peminjaman WHERE id = :id");
        q3.bindValue(":id", idTerpilih);
        if (q3.exec() && q3.next())
            denda = q3.value(0).toInt();
        q3.finish();

        if (!db.commit()) {
            const QString err = db.lastError().text();
            db.rollback();
            QMessageBox::critical(this, "Gagal", err);
            return;
        }

        emit dataBerubah();

        if (denda > 0)
            QMessageBox::warning(this, "Buku dikembalikan",
                                 "Buku berhasil dikembalikan.\nDenda keterlambatan: " + rupiah(denda));
        else
            QMessageBox::information(this, "Berhasil", "Buku berhasil dikembalikan tepat waktu.");
    }

    // ---------- Tombol "Hapus Riwayat" (hanya yang sudah dikembalikan) ----------
    void hapusRiwayat()
    {
        if (idTerpilih < 0) {
            QMessageBox::information(this, "Info", "Pilih data peminjaman pada tabel terlebih dahulu.");
            return;
        }
        if (statusTerpilih != "Dikembalikan") {
            QMessageBox::information(this, "Info",
                                     "Hanya riwayat yang sudah dikembalikan yang bisa dihapus.");
            return;
        }
        if (!konfirmasi(this, "Hapus riwayat peminjaman ini?"))
            return;

        QSqlQuery q;
        q.prepare("DELETE FROM peminjaman WHERE id = :id");
        q.bindValue(":id", idTerpilih);
        if (!q.exec()) {
            QMessageBox::critical(this, "Gagal", q.lastError().text());
            return;
        }
        q.finish();

        emit dataBerubah();
    }

    void pilihBaris(const QModelIndex &index)
    {
        const int r = index.row();
        idTerpilih = model->item(r, 0)->text().toInt();
        statusTerpilih = model->item(r, 6)->text();
    }

    void eksporData()
    {
        eksporModelCsv(this, model, "data_peminjaman.csv");
    }

private:
    void bangunUI()
    {
        auto *utama = new QVBoxLayout(this);
        utama->setContentsMargins(0, 0, 0, 0);
        utama->setSpacing(14);

        utama->addWidget(headerHalaman("Peminjaman",
                                       "Catat peminjaman dan pengembalian buku. Denda dihitung otomatis."));

        // ---------- Kartu form (kiri) ----------
        auto *kartuForm = buatKartu();
        kartuForm->setFixedWidth(380);
        auto *fl = new QVBoxLayout(kartuForm);
        fl->setContentsMargins(20, 16, 20, 16);
        fl->setSpacing(8);
        fl->addWidget(buatLabel("Peminjaman Baru", "judulKartu"));

        cmbAnggota = new QComboBox;
        cmbBuku = new QComboBox;

        dtPinjam = new QDateEdit(QDate::currentDate());
        dtPinjam->setCalendarPopup(true);
        dtPinjam->setDisplayFormat("dd-MM-yyyy");
        dtPinjam->setMinimumDate(QDate(2000, 1, 1));

        spnLama = new QSpinBox;
        spnLama->setRange(1, 30);
        spnLama->setValue(7);
        spnLama->setSuffix(" hari");

        lblTempo = buatLabel("", "info");

        fl->addWidget(kolomForm("Anggota", cmbAnggota));
        fl->addWidget(kolomForm("Buku", cmbBuku));
        fl->addWidget(barisForm(kolomForm("Tanggal Pinjam", dtPinjam),
                                kolomForm("Lama Pinjam", spnLama)));
        fl->addWidget(lblTempo);

        auto *btnPinjam = new QPushButton("Pinjamkan Buku");
        btnPinjam->setObjectName("primer");
        btnPinjam->setCursor(Qt::PointingHandCursor);
        fl->addWidget(btnPinjam);

        auto *garis = new QFrame;
        garis->setObjectName("garis");
        garis->setFixedHeight(1);
        fl->addSpacing(4);
        fl->addWidget(garis);
        fl->addSpacing(4);

        fl->addWidget(buatLabel("Pengembalian", "judulKartu"));
        auto *petunjuk = buatLabel("Pilih satu baris pada tabel, lalu klik tombol Kembalikan Buku.",
                                   "info");
        petunjuk->setWordWrap(true);
        fl->addWidget(petunjuk);

        auto *btnKembali = new QPushButton("Kembalikan Buku");
        btnKembali->setObjectName("emas");
        auto *btnHapus = new QPushButton("Hapus Riwayat");
        btnHapus->setObjectName("bahaya");
        btnKembali->setCursor(Qt::PointingHandCursor);
        btnHapus->setCursor(Qt::PointingHandCursor);

        auto *barisBtn = new QHBoxLayout;
        barisBtn->setSpacing(8);
        barisBtn->addWidget(btnKembali);
        barisBtn->addWidget(btnHapus);
        fl->addLayout(barisBtn);
        fl->addStretch();

        auto *aturan = buatLabel(QString("Denda %1 per hari keterlambatan. "
                                         "Maksimal %2 buku dipinjam per anggota.")
                                     .arg(rupiah(Database::dendaPerHari))
                                     .arg(Database::maksPinjam),
                                 "info");
        aturan->setWordWrap(true);
        fl->addWidget(aturan);

        // ---------- Kartu tabel (kanan) ----------
        auto *kartuTabel = buatKartu();
        auto *tl = new QVBoxLayout(kartuTabel);
        tl->setContentsMargins(20, 16, 20, 16);
        tl->setSpacing(10);

        auto *atas = new QHBoxLayout;
        atas->setSpacing(10);
        atas->addWidget(buatLabel("Daftar Peminjaman", "judulKartu"));
        lblJumlah = buatLabel("0 transaksi", "info");
        atas->addWidget(lblJumlah);
        atas->addStretch();

        cmbFilter = new QComboBox;
        cmbFilter->addItems({"Semua Status", "Dipinjam", "Terlambat", "Dikembalikan"});
        cmbFilter->setFixedWidth(150);

        edtCari = new QLineEdit;
        edtCari->setPlaceholderText("Cari anggota atau buku...");
        edtCari->setClearButtonEnabled(true);
        edtCari->setFixedWidth(220);

        auto *btnEkspor = new QPushButton("Ekspor CSV");
        btnEkspor->setCursor(Qt::PointingHandCursor);

        atas->addWidget(cmbFilter);
        atas->addWidget(edtCari);
        atas->addWidget(btnEkspor);
        tl->addLayout(atas);

        tabel = new QTableView;
        aturTabel(tabel, model);
        tl->addWidget(tabel, 1);

        // ---------- Gabungkan ----------
        auto *konten = new QHBoxLayout;
        konten->setSpacing(16);
        konten->addWidget(kartuForm);
        konten->addWidget(kartuTabel, 1);
        utama->addLayout(konten, 1);

        // ---------- Hubungkan tombol ----------
        connect(btnPinjam, &QPushButton::clicked, this, &HalamanPinjam::pinjamkan);
        connect(btnKembali, &QPushButton::clicked, this, &HalamanPinjam::kembalikan);
        connect(btnHapus, &QPushButton::clicked, this, &HalamanPinjam::hapusRiwayat);
        connect(btnEkspor, &QPushButton::clicked, this, &HalamanPinjam::eksporData);
        connect(dtPinjam, &QDateEdit::dateChanged, this, &HalamanPinjam::hitungTempo);
        connect(spnLama, QOverload<int>::of(&QSpinBox::valueChanged), this,
                &HalamanPinjam::hitungTempo);
        connect(cmbFilter, &QComboBox::currentIndexChanged, this, &HalamanPinjam::muatData);
        connect(edtCari, &QLineEdit::textChanged, this, &HalamanPinjam::muatData);
        connect(tabel, &QTableView::clicked, this, &HalamanPinjam::pilihBaris);

        hitungTempo();
    }

    QComboBox *cmbAnggota = nullptr;
    QComboBox *cmbBuku = nullptr;
    QComboBox *cmbFilter = nullptr;
    QDateEdit *dtPinjam = nullptr;
    QSpinBox *spnLama = nullptr;
    QLineEdit *edtCari = nullptr;
    QLabel *lblTempo = nullptr;
    QLabel *lblJumlah = nullptr;
    QTableView *tabel = nullptr;
    QStandardItemModel *model = nullptr;

    int idTerpilih = -1;
    QString statusTerpilih;
};