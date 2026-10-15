#pragma once

#include "Database.h"
#include "Tema.h"

#include <QPropertyAnimation>
#include <QtWidgets>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Masuk - Aplikasi Perpustakaan");
        setFixedSize(880, 540);
        bangunUI();
    }

private slots:
    void masuk()
    {
        const QString user = edtUser->text().trimmed();
        if (user.isEmpty() || edtSandi->text().isEmpty()) {
            lblError->setText("Username dan kata sandi wajib diisi.");
            return;
        }

        if (Database::periksaLogin(user, edtSandi->text())) {
            accept();
            return;
        }

        lblError->setText("Username atau kata sandi salah.");
        edtSandi->clear();
        edtSandi->setFocus();
        getar();
    }

private:
    // Efek jendela bergetar saat login salah
    void getar()
    {
        const QPoint asal = pos();
        auto *a = new QPropertyAnimation(this, "pos", this);
        a->setDuration(320);
        a->setKeyValueAt(0.0, asal);
        a->setKeyValueAt(0.2, asal + QPoint(-12, 0));
        a->setKeyValueAt(0.4, asal + QPoint(12, 0));
        a->setKeyValueAt(0.6, asal + QPoint(-8, 0));
        a->setKeyValueAt(0.8, asal + QPoint(8, 0));
        a->setKeyValueAt(1.0, asal);
        a->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void bangunUI()
    {
        auto *utama = new QHBoxLayout(this);
        utama->setContentsMargins(0, 0, 0, 0);
        utama->setSpacing(0);

        // ================= PANEL KIRI (hijau) =================
        auto *kiri = new QFrame;
        kiri->setObjectName("loginKiri");
        kiri->setFixedWidth(380);

        auto *kl = new QVBoxLayout(kiri);
        kl->setContentsMargins(38, 44, 38, 38);
        kl->setSpacing(6);

        auto *lencana = new QLabel("📚");
        lencana->setObjectName("lencana");
        lencana->setFixedSize(68, 68);
        lencana->setAlignment(Qt::AlignCenter);
        lencana->setStyleSheet("font-size: 28pt;");
        kl->addWidget(lencana);
        kl->addSpacing(18);

        kl->addWidget(buatLabel("Perpustakaan\nSekolah", "loginJudul"));
        kl->addSpacing(6);

        auto *sub = buatLabel("Sistem informasi pengelolaan buku, anggota, dan peminjaman.", "loginSub");
        sub->setWordWrap(true);
        kl->addWidget(sub);
        kl->addSpacing(20);

        kl->addWidget(buatLabel("✓  Data buku dan anggota\n"
                                "✓  Peminjaman dan denda otomatis\n"
                                "✓  Laporan PDF dan kartu anggota",
                                "loginFitur"));
        kl->addStretch();
        kl->addWidget(buatLabel("\"Membaca adalah jendela dunia.\"", "loginQuote"));

        // ================= PANEL KANAN (formulir) =================
        auto *kanan = new QFrame;
        kanan->setObjectName("loginKanan");

        auto *fl = new QVBoxLayout(kanan);
        fl->setContentsMargins(56, 40, 56, 30);
        fl->setSpacing(8);

        fl->addStretch();
        fl->addWidget(buatLabel("Selamat Datang", "judulHalaman"));
        fl->addWidget(buatLabel("Masuk untuk melanjutkan ke dashboard.", "subHalaman"));
        fl->addSpacing(16);

        edtUser = new QLineEdit;
        edtUser->setPlaceholderText("Masukkan username");

        edtSandi = new QLineEdit;
        edtSandi->setPlaceholderText("Masukkan kata sandi");
        edtSandi->setEchoMode(QLineEdit::Password);

        auto *cekLihat = new QCheckBox("Tampilkan kata sandi");
        connect(cekLihat, &QCheckBox::toggled, this, [this](bool lihat) {
            edtSandi->setEchoMode(lihat ? QLineEdit::Normal : QLineEdit::Password);
        });

        lblError = buatLabel("", "loginError");

        auto *btnMasuk = new QPushButton("Masuk");
        btnMasuk->setObjectName("primer");
        btnMasuk->setCursor(Qt::PointingHandCursor);
        btnMasuk->setMinimumHeight(44);
        btnMasuk->setDefault(true);

        fl->addWidget(kolomForm("Username", edtUser));
        fl->addWidget(kolomForm("Kata Sandi", edtSandi));
        fl->addWidget(cekLihat);
        fl->addWidget(lblError);
        fl->addSpacing(4);
        fl->addWidget(btnMasuk);
        fl->addSpacing(6);
        fl->addWidget(buatLabel("Akun awal:  admin  /  admin123", "info"));
        fl->addStretch();
        fl->addWidget(buatLabel("© Perpustakaan Sekolah", "info"));

        utama->addWidget(kiri);
        utama->addWidget(kanan, 1);

        connect(btnMasuk, &QPushButton::clicked, this, &LoginDialog::masuk);
        connect(edtSandi, &QLineEdit::returnPressed, this, &LoginDialog::masuk);
        connect(edtUser, &QLineEdit::returnPressed, this, [this]() { edtSandi->setFocus(); });

        edtUser->setFocus();
    }

    QLineEdit *edtUser = nullptr;
    QLineEdit *edtSandi = nullptr;
    QLabel *lblError = nullptr;
};