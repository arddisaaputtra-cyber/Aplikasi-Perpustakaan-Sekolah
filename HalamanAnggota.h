#pragma once

#include "Database.h"
#include "Tema.h"

#include <QtSql>
#include <QtWidgets>

class HalamanAnggota : public QWidget
{
    Q_OBJECT

public:
    explicit HalamanAnggota(QWidget *parent = nullptr) : QWidget(parent)
    {
        model = new QStandardItemModel(this);
        bangunUI();
        muatData();
    }

signals:
    void dataBerubah();

public slots:
    void muatData()
    {
        const QString kw = edtCari ? edtCari->text().trimmed() : QString();
        const QString pola = "%" + kw + "%";

        QSqlQuery q;
        q.prepare("SELECT id, kode, nama, jenis_kelamin, kelas, no_hp, alamat "
                  "FROM anggota "
                  "WHERE kode LIKE :k1 OR nama LIKE :k2 OR kelas LIKE :k3 "
                  "ORDER BY nama");
        q.bindValue(":k1", pola);
        q.bindValue(":k2", pola);
        q.bindValue(":k3", pola);

        if (!q.exec()) {
            QMessageBox::critical(this, "Gagal memuat data", q.lastError().text());
            return;
        }

        Database::isiModel(model, q, {"ID", "Kode", "Nama", "Jenis Kelamin",
                                      "Kelas / Status", "No. HP", "Alamat"});
        rapikanKolom(tabel, 2, true);
        lblJumlah->setText(QString::number(model->rowCount()) + " anggota");
    }

private slots:
    void tambahData()
    {
        if (!validasi())
            return;

        QSqlQuery q;
        q.prepare("INSERT INTO anggota (kode, nama, jenis_kelamin, kelas, no_hp, alamat) "
                  "VALUES (:kode, :nama, :jk, :kelas, :hp, :alamat)");
        isiParameter(q);

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (galatDuplikat(err))
                QMessageBox::warning(this, "Gagal", "Kode anggota sudah terdaftar. Gunakan kode lain.");
            else
                QMessageBox::critical(this, "Gagal", err);
            return;
        }
        q.finish();

        bersihkanForm();
        emit dataBerubah();
        QMessageBox::information(this, "Berhasil", "Data anggota berhasil disimpan.");
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
        q.prepare("UPDATE anggota SET kode=:kode, nama=:nama, jenis_kelamin=:jk, "
                  "kelas=:kelas, no_hp=:hp, alamat=:alamat WHERE id=:id");
        isiParameter(q);
        q.bindValue(":id", idTerpilih);

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (galatDuplikat(err))
                QMessageBox::warning(this, "Gagal", "Kode anggota sudah dipakai anggota lain.");
            else
                QMessageBox::critical(this, "Gagal", err);
            return;
        }
        q.finish();

        bersihkanForm();
        emit dataBerubah();
        QMessageBox::information(this, "Berhasil", "Data anggota berhasil diubah.");
    }

    void hapusData()
    {
        if (idTerpilih < 0) {
            QMessageBox::information(this, "Info", "Pilih data pada tabel terlebih dahulu.");
            return;
        }
        if (!konfirmasi(this, "Yakin ingin menghapus anggota ini?"))
            return;

        QSqlQuery q;
        q.prepare("DELETE FROM anggota WHERE id=:id");
        q.bindValue(":id", idTerpilih);

        if (!q.exec()) {
            const QString err = q.lastError().text();
            if (galatRelasi(err))
                QMessageBox::warning(this, "Tidak bisa dihapus",
                                     "Anggota ini punya riwayat peminjaman, sehingga tidak bisa dihapus.");
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
        edtNama->clear();
        edtHp->clear();
        edtAlamat->clear();
        cmbJK->setCurrentIndex(0);
        cmbKelas->setCurrentIndex(0);
        tabel->clearSelection();
        edtKode->setFocus();
    }

    void pilihBaris(const QModelIndex &index)
    {
        const int r = index.row();
        auto teks = [&](int kolom) { return model->item(r, kolom)->text(); };

        idTerpilih = teks(0).toInt();
        edtKode->setText(teks(1));
        edtNama->setText(teks(2));
        cmbJK->setCurrentText(teks(3));
        cmbKelas->setCurrentText(teks(4));
        edtHp->setText(teks(5));
        edtAlamat->setPlainText(teks(6));
    }

    void eksporData()
    {
        eksporModelCsv(this, model, "data_anggota.csv");
    }

private:
    void bangunUI()
    {
        auto *utama = new QVBoxLayout(this);
        utama->setContentsMargins(0, 0, 0, 0);
        utama->setSpacing(14);

        utama->addWidget(headerHalaman("Data Anggota",
                                       "Kelola anggota perpustakaan: tambah, ubah, hapus, dan cari."));

        // ---------- Kartu form (kiri) ----------
        auto *kartuForm = buatKartu();
        kartuForm->setFixedWidth(380);
        auto *fl = new QVBoxLayout(kartuForm);
        fl->setContentsMargins(20, 16, 20, 16);
        fl->setSpacing(8);
        fl->addWidget(buatLabel("Form Anggota", "judulKartu"));

        edtKode = new QLineEdit;
        edtKode->setPlaceholderText("Contoh: AG005");
        edtNama = new QLineEdit;
        edtNama->setPlaceholderText("Nama lengkap anggota");

        cmbJK = new QComboBox;
        cmbJK->addItems({"Laki-laki", "Perempuan"});

        cmbKelas = new QComboBox;
        cmbKelas->setEditable(true);
        cmbKelas->addItems({"X-A", "X-B", "X-C", "XI-A", "XI-B", "XI-C",
                            "XII-A", "XII-B", "XII-C", "Guru", "Staf", "Umum"});

        edtHp = new QLineEdit;
        edtHp->setPlaceholderText("08xxxxxxxxxx");
        edtHp->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9+]{0,15}"), this));

        edtAlamat = new QPlainTextEdit;
        edtAlamat->setPlaceholderText("Alamat lengkap");
        edtAlamat->setFixedHeight(64);

        fl->addWidget(barisForm(kolomForm("Kode Anggota", edtKode),
                                kolomForm("Jenis Kelamin", cmbJK)));
        fl->addWidget(kolomForm("Nama Lengkap", edtNama));
        fl->addWidget(barisForm(kolomForm("Kelas / Status", cmbKelas),
                                kolomForm("No. HP", edtHp)));
        fl->addWidget(kolomForm("Alamat", edtAlamat));
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
        atas->addWidget(buatLabel("Daftar Anggota", "judulKartu"));
        lblJumlah = buatLabel("0 anggota", "info");
        atas->addWidget(lblJumlah);
        atas->addStretch();

        edtCari = new QLineEdit;
        edtCari->setPlaceholderText("Cari kode, nama, atau kelas...");
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
        connect(btnSimpan, &QPushButton::clicked, this, &HalamanAnggota::tambahData);
        connect(btnUbah, &QPushButton::clicked, this, &HalamanAnggota::ubahData);
        connect(btnHapus, &QPushButton::clicked, this, &HalamanAnggota::hapusData);
        connect(btnBersih, &QPushButton::clicked, this, &HalamanAnggota::bersihkanForm);
        connect(btnEkspor, &QPushButton::clicked, this, &HalamanAnggota::eksporData);
        connect(edtCari, &QLineEdit::textChanged, this, &HalamanAnggota::muatData);
        connect(tabel, &QTableView::clicked, this, &HalamanAnggota::pilihBaris);
    }

    bool validasi()
    {
        if (edtKode->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validasi", "Kode anggota tidak boleh kosong.");
            edtKode->setFocus();
            return false;
        }
        if (edtNama->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validasi", "Nama tidak boleh kosong.");
            edtNama->setFocus();
            return false;
        }
        return true;
    }

    void isiParameter(QSqlQuery &q) const
    {
        q.bindValue(":kode", edtKode->text().trimmed());
        q.bindValue(":nama", edtNama->text().trimmed());
        q.bindValue(":jk", cmbJK->currentText());
        q.bindValue(":kelas", cmbKelas->currentText().trimmed());
        q.bindValue(":hp", edtHp->text().trimmed());
        q.bindValue(":alamat", edtAlamat->toPlainText().trimmed());
    }

    QLineEdit *edtKode = nullptr;
    QLineEdit *edtNama = nullptr;
    QLineEdit *edtHp = nullptr;
    QLineEdit *edtCari = nullptr;
    QComboBox *cmbJK = nullptr;
    QComboBox *cmbKelas = nullptr;
    QPlainTextEdit *edtAlamat = nullptr;
    QLabel *lblJumlah = nullptr;
    QTableView *tabel = nullptr;
    QStandardItemModel *model = nullptr;

    int idTerpilih = -1;
};