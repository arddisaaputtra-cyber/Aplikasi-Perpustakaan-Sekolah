#pragma once

#include "Database.h"
#include "Tema.h"

#include <QtSql>
#include <QtWidgets>

class HalamanBuku : public QWidget
{
    Q_OBJECT

public:
    explicit HalamanBuku(QWidget *parent = nullptr) : QWidget(parent)
    {
        model = new QStandardItemModel(this);
        bangunUI();
        muatData();
    }

signals:
    void dataBerubah();

public slots:
    // Memuat tabel (dipakai juga oleh kolom pencarian)
    void muatData()
    {
        const QString kw = edtCari ? edtCari->text().trimmed() : QString();
        const QString pola = "%" + kw + "%";

        QSqlQuery q;
        q.prepare("SELECT id, kode, judul, pengarang, penerbit, tahun, kategori, stok "
                  "FROM buku "
                  "WHERE kode LIKE :k1 OR judul LIKE :k2 OR pengarang LIKE :k3 OR kategori LIKE :k4 "
                  "ORDER BY judul");
        q.bindValue(":k1", pola);
        q.bindValue(":k2", pola);
        q.bindValue(":k3", pola);
        q.bindValue(":k4", pola);

        if (!q.exec()) {
            QMessageBox::critical(this, "Gagal memuat data", q.lastError().text());
            return;
        }

        Database::isiModel(model, q, {"ID", "Kode", "Judul", "Pengarang", "Penerbit",
                                      "Tahun", "Kategori", "Stok"});
        rapikanKolom(tabel, 2, true);
        lblJumlah->setText(QString::number(model->rowCount()) + " judul");
    }

private slots:
    void tambahData()
    {
        if (!validasi())
            return;

        QSqlQuery q;
        q.prepare("INSERT INTO buku (kode, judul, pengarang, penerbit, tahun, kategori, stok) "
                  "VALUES (:kode, :judul, :pengarang, :penerbit, :tahun, :kategori, :stok)");
        isiParameter(q);

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (galatDuplikat(err))
                QMessageBox::warning(this, "Gagal", "Kode buku sudah terdaftar. Gunakan kode lain.");
            else
                QMessageBox::critical(this, "Gagal", err);
            return;
        }
        q.finish();

        bersihkanForm();
        emit dataBerubah();
        QMessageBox::information(this, "Berhasil", "Data buku berhasil disimpan.");
    }

    void ubahData()
    {
        if (idTerpilih < 0) {
            QMessageBox::information(this, "Info", "Pilih data pada tabel terlebih dahulu.");
            return;
        }
        if (!validasi())
            return;

        QSqlQuery q;
        q.prepare("UPDATE buku SET kode=:kode, judul=:judul, pengarang=:pengarang, "
                  "penerbit=:penerbit, tahun=:tahun, kategori=:kategori, stok=:stok "
                  "WHERE id=:id");
        isiParameter(q);
        q.bindValue(":id", idTerpilih);

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (galatDuplikat(err))
                QMessageBox::warning(this, "Gagal", "Kode buku sudah dipakai buku lain.");
            else
                QMessageBox::critical(this, "Gagal", err);
            return;
        }
        q.finish();

        bersihkanForm();
        emit dataBerubah();
        QMessageBox::information(this, "Berhasil", "Data buku berhasil diubah.");
    }

    void hapusData()
    {
        if (idTerpilih < 0) {
            QMessageBox::information(this, "Info", "Pilih data pada tabel terlebih dahulu.");
            return;
        }
        if (!konfirmasi(this, "Yakin ingin menghapus buku ini?"))
            return;

        QSqlQuery q;
        q.prepare("DELETE FROM buku WHERE id=:id");
        q.bindValue(":id", idTerpilih);

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (galatRelasi(err))
                QMessageBox::warning(this, "Tidak bisa dihapus",
                                     "Buku ini sudah pernah dipinjam, sehingga tidak bisa dihapus.");
            else
                QMessageBox::critical(this, "Gagal", err);
            return;
        }
        q.finish();

        bersihkanForm();
        emit dataBerubah();
    }

    void bersihkanForm()
    {
        idTerpilih = -1;
        edtKode->clear();
        edtJudul->clear();
        edtPengarang->clear();
        edtPenerbit->clear();
        spnTahun->setValue(2020);
        spnStok->setValue(1);
        cmbKategori->setCurrentIndex(0);
        tabel->clearSelection();
        edtKode->setFocus();
    }

    // Klik baris tabel -> isi form
    void pilihBaris(const QModelIndex &index)
    {
        const int r = index.row();
        auto teks = [&](int kolom) { return model->item(r, kolom)->text(); };

        idTerpilih = teks(0).toInt();
        edtKode->setText(teks(1));
        edtJudul->setText(teks(2));
        edtPengarang->setText(teks(3));
        edtPenerbit->setText(teks(4));
        const int th = teks(5).toInt();
        spnTahun->setValue(th > 0 ? th : 2020);
        cmbKategori->setCurrentText(teks(6));
        spnStok->setValue(teks(7).toInt());
    }

    void eksporData()
    {
        eksporModelCsv(this, model, "data_buku.csv");
    }

private:
    void bangunUI()
    {
        auto *utama = new QVBoxLayout(this);
        utama->setContentsMargins(0, 0, 0, 0);
        utama->setSpacing(14);

        utama->addWidget(headerHalaman("Data Buku",
                                       "Kelola koleksi perpustakaan: tambah, ubah, hapus, dan cari buku."));

        // ---------- Kartu form (kiri) ----------
        auto *kartuForm = buatKartu();
        kartuForm->setFixedWidth(380);
        auto *fl = new QVBoxLayout(kartuForm);
        fl->setContentsMargins(20, 16, 20, 16);
        fl->setSpacing(8);
        fl->addWidget(buatLabel("Form Buku", "judulKartu"));

        edtKode = new QLineEdit;
        edtKode->setPlaceholderText("Contoh: BK009");
        edtJudul = new QLineEdit;
        edtJudul->setPlaceholderText("Judul buku");
        edtPengarang = new QLineEdit;
        edtPengarang->setPlaceholderText("Nama pengarang");
        edtPenerbit = new QLineEdit;
        edtPenerbit->setPlaceholderText("Nama penerbit");

        spnTahun = new QSpinBox;
        spnTahun->setRange(1500, 2100);
        spnTahun->setValue(2020);

        spnStok = new QSpinBox;
        spnStok->setRange(0, 9999);
        spnStok->setValue(1);

        cmbKategori = new QComboBox;
        cmbKategori->setEditable(true);
        cmbKategori->addItems({"Fiksi", "Non-Fiksi", "Sains", "Teknologi", "Sejarah",
                               "Agama", "Pelajaran", "Referensi", "Biografi", "Komik"});

        fl->addWidget(barisForm(kolomForm("Kode Buku", edtKode),
                                kolomForm("Tahun Terbit", spnTahun)));
        fl->addWidget(kolomForm("Judul", edtJudul));
        fl->addWidget(kolomForm("Pengarang", edtPengarang));
        fl->addWidget(kolomForm("Penerbit", edtPenerbit));
        fl->addWidget(barisForm(kolomForm("Kategori", cmbKategori),
                                kolomForm("Stok Tersedia", spnStok)));
        fl->addSpacing(4);

        auto *btnSimpan = new QPushButton("Simpan");
        btnSimpan->setObjectName("primer");
        auto *btnUbah = new QPushButton("Ubah");
        btnUbah->setObjectName("emas");
        auto *btnHapus = new QPushButton("Hapus");
        btnHapus->setObjectName("bahaya");
        auto *btnBersih = new QPushButton("Bersihkan");

        for (auto *b : {btnSimpan, btnUbah, btnHapus, btnBersih})
            b->setCursor(Qt::PointingHandCursor);

        auto *grid = new QGridLayout;
        grid->setSpacing(8);
        grid->addWidget(btnSimpan, 0, 0);
        grid->addWidget(btnUbah, 0, 1);
        grid->addWidget(btnHapus, 1, 0);
        grid->addWidget(btnBersih, 1, 1);
        fl->addLayout(grid);
        fl->addStretch();

        // ---------- Kartu tabel (kanan) ----------
        auto *kartuTabel = buatKartu();
        auto *tl = new QVBoxLayout(kartuTabel);
        tl->setContentsMargins(20, 16, 20, 16);
        tl->setSpacing(10);

        auto *atas = new QHBoxLayout;
        atas->setSpacing(10);
        atas->addWidget(buatLabel("Koleksi Buku", "judulKartu"));
        lblJumlah = buatLabel("0 judul", "info");
        atas->addWidget(lblJumlah);
        atas->addStretch();

        edtCari = new QLineEdit;
        edtCari->setPlaceholderText("Cari kode, judul, pengarang...");
        edtCari->setClearButtonEnabled(true);
        edtCari->setFixedWidth(260);
        auto *btnEkspor = new QPushButton("Ekspor CSV");
        btnEkspor->setCursor(Qt::PointingHandCursor);
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
        connect(btnSimpan, &QPushButton::clicked, this, &HalamanBuku::tambahData);
        connect(btnUbah, &QPushButton::clicked, this, &HalamanBuku::ubahData);
        connect(btnHapus, &QPushButton::clicked, this, &HalamanBuku::hapusData);
        connect(btnBersih, &QPushButton::clicked, this, &HalamanBuku::bersihkanForm);
        connect(btnEkspor, &QPushButton::clicked, this, &HalamanBuku::eksporData);
        connect(edtCari, &QLineEdit::textChanged, this, &HalamanBuku::muatData);
        connect(tabel, &QTableView::clicked, this, &HalamanBuku::pilihBaris);
    }

    bool validasi()
    {
        if (edtKode->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validasi", "Kode buku tidak boleh kosong.");
            edtKode->setFocus();
            return false;
        }
        if (edtJudul->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validasi", "Judul tidak boleh kosong.");
            edtJudul->setFocus();
            return false;
        }
        if (edtPengarang->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validasi", "Pengarang tidak boleh kosong.");
            edtPengarang->setFocus();
            return false;
        }
        return true;
    }

    void isiParameter(QSqlQuery &q) const
    {
        q.bindValue(":kode", edtKode->text().trimmed());
        q.bindValue(":judul", edtJudul->text().trimmed());
        q.bindValue(":pengarang", edtPengarang->text().trimmed());
        q.bindValue(":penerbit", edtPenerbit->text().trimmed());
        q.bindValue(":tahun", spnTahun->value());
        q.bindValue(":kategori", cmbKategori->currentText().trimmed());
        q.bindValue(":stok", spnStok->value());
    }

    QLineEdit *edtKode = nullptr;
    QLineEdit *edtJudul = nullptr;
    QLineEdit *edtPengarang = nullptr;
    QLineEdit *edtPenerbit = nullptr;
    QLineEdit *edtCari = nullptr;
    QSpinBox *spnTahun = nullptr;
    QSpinBox *spnStok = nullptr;
    QComboBox *cmbKategori = nullptr;
    QLabel *lblJumlah = nullptr;
    QTableView *tabel = nullptr;
    QStandardItemModel *model = nullptr;

    int idTerpilih = -1;
};