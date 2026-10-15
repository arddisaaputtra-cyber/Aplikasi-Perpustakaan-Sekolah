#pragma once

#include "Database.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QtWidgets>

// ===================== IKON PANAH (digambar sekali saat program mulai) =====================
inline QString buatPanah(const QString &nama, bool atas)
{
    const QString path = QDir::temp().filePath(nama);

    QPixmap pm(12, 12);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#6B5A4A"));

    QPolygonF segitiga;
    if (atas)
        segitiga << QPointF(2, 8) << QPointF(10, 8) << QPointF(6, 3.5);
    else
        segitiga << QPointF(2, 4) << QPointF(10, 4) << QPointF(6, 8.5);
    p.drawPolygon(segitiga);
    p.end();

    pm.save(path, "PNG");
    return QDir::fromNativeSeparators(path);
}

// ===================== DESAIN PERPUSTAKAAN (QSS) =====================
// Warna: krem (kertas), cokelat kayu (rak), emas, dan hijau tua.
inline QString temaPerpustakaan()
{
    const QString panahBawah = buatPanah("perpus_panah_bawah.png", false);
    const QString panahAtas = buatPanah("perpus_panah_atas.png", true);

    QString qss = QString(R"QSS(
        QWidget { font-family: "Segoe UI"; font-size: 10pt; color: #3A2E26; }
        QMainWindow { background: #F6EFE3; }
        QWidget#konten { background: #F6EFE3; }
        QDialog { background: #FFFDF8; }
        QLabel { background: transparent; }
        QScrollArea { background: transparent; border: none; }

        /* ---------- MENU SAMPING ---------- */
        QWidget#sidebar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #46322A, stop:1 #2A1D15);
        }
        QLabel#lencana {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #E0B95E, stop:1 #B8913F);
            border-radius: 12px; font-size: 20pt;
        }
        QLabel#logo { font-family: "Georgia"; font-size: 15pt; font-weight: 700; color: #F6EBD6; }
        QLabel#logoSub { color: #C9B79C; font-size: 8pt; }
        QLabel#menuJudul { color: #8F7D66; font-size: 8pt; font-weight: 700; }
        QLabel#footerSide { color: #A8977F; font-size: 8pt; }

        QFrame#jamKartu {
            background: rgba(255, 255, 255, 14);
            border: 1px solid rgba(255, 255, 255, 22);
            border-radius: 12px;
        }
        QLabel#jam { font-size: 17pt; font-weight: 700; color: #E9D8A6; }
        QLabel#jamTanggal { color: #C9B79C; font-size: 9pt; }

        QLabel#avatar {
            background: #C9A24D; color: #2B1D14; border-radius: 19px;
            font-size: 13pt; font-weight: 700;
        }
        QLabel#userNama { color: #F6EBD6; font-weight: 700; }
        QLabel#userPeran { color: #C9B79C; font-size: 8pt; }
        QPushButton#keluar {
            background: rgba(255, 255, 255, 18); color: #F1D9A6;
            border-radius: 8px; padding: 7px; font-weight: 600;
        }
        QPushButton#keluar:hover { background: rgba(184, 80, 66, 190); color: #FFFFFF; }

        QPushButton#nav {
            background: transparent; color: #E6D9C3; text-align: left;
            padding: 12px 16px; border: none; border-radius: 10px;
            font-size: 11pt; font-weight: 600;
        }
        QPushButton#nav:hover { background: rgba(255, 255, 255, 22); }
        QPushButton#nav:checked { background: #C9A24D; color: #2B1D14; }

        /* ---------- JUDUL & KARTU ---------- */
        QLabel#judulHalaman { font-family: "Georgia"; font-size: 24pt; font-weight: 700; color: #3B2A20; }
        QLabel#subHalaman { color: #8A7968; }
        QFrame#aksen { background: #C9A24D; border: none; border-radius: 2px; }
        QLabel#judulKartu { font-family: "Georgia"; font-size: 14pt; font-weight: 700; color: #3B2A20; }
        QLabel#label { font-size: 9pt; font-weight: 600; color: #6B5A4A; }
        QLabel#info { color: #8A7968; font-size: 9pt; }
        QLabel#kosong { color: #A8977F; font-size: 12pt; }

        QFrame#card { background: #FFFDF8; border: 1px solid #EADFCB; border-radius: 16px; }
        QFrame#garis { background: #E4D8C3; border: none; }

        /* ---------- BANNER DASHBOARD ---------- */
        QFrame#banner {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #24493F, stop:0.6 #2F5D50, stop:1 #4C7A66);
            border: none; border-radius: 16px;
        }
        QLabel#bannerJudul { font-family: "Georgia"; font-size: 20pt; font-weight: 700; color: #FFFFFF; }
        QLabel#bannerSub { color: #DCE9E2; }
        QLabel#bannerQuote { color: #E9D8A6; font-style: italic; }
        QLabel#pengingat {
            background: rgba(255, 255, 255, 34); color: #FFFFFF;
            border-radius: 10px; padding: 6px 14px; font-weight: 600;
        }
        QPushButton#bannerBtn {
            background: #C9A24D; color: #2B1D14; border-radius: 10px;
            padding: 11px 20px; font-weight: 700;
        }
        QPushButton#bannerBtn:hover { background: #E0B95E; }
        QPushButton#bannerBtn2 {
            background: rgba(255, 255, 255, 28); color: #FFFFFF;
            border: 1px solid rgba(255, 255, 255, 90); border-radius: 10px;
            padding: 11px 20px; font-weight: 700;
        }
        QPushButton#bannerBtn2:hover { background: rgba(255, 255, 255, 55); }

        /* ---------- KARTU STATISTIK & PERINGKAT ---------- */
        QLabel#statJudul { color: #8A7968; font-size: 9pt; font-weight: 600; }
        QLabel#statNilai { font-size: 22pt; font-weight: 700; color: #2F5D50; }
        QLabel#statNilaiMerah { font-size: 22pt; font-weight: 700; color: #B85042; }
        QLabel#peringkatNama { color: #3A2E26; font-weight: 600; }
        QLabel#peringkatNilai { color: #8A7968; font-size: 9pt; }

        /* ---------- LOGIN ---------- */
        QFrame#loginKiri {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #24493F, stop:0.6 #2F5D50, stop:1 #4C7A66);
            border: none;
        }
        QFrame#loginKanan { background: #FFFDF8; border: none; }
        QLabel#loginJudul { font-family: "Georgia"; font-size: 26pt; font-weight: 700; color: #FFFFFF; }
        QLabel#loginSub { color: #DCE9E2; }
        QLabel#loginFitur { color: #F3E5BE; }
        QLabel#loginQuote { color: #E9D8A6; font-style: italic; }
        QLabel#loginError { color: #B85042; font-weight: 600; }
        QCheckBox { color: #6B5A4A; spacing: 8px; }

        /* ---------- KARTU PILIHAN LAPORAN ---------- */
        QPushButton#tile {
            background: #FBF7EF; color: #3A2E26; border: 1px solid #E4D8C3;
            border-radius: 12px; padding: 14px 16px; text-align: left;
            font-weight: 600; min-height: 54px;
        }
        QPushButton#tile:hover { background: #F6EEDC; border: 1px solid #C9A24D; }

        /* ---------- INPUT ---------- */
        QLineEdit, QComboBox, QDateEdit, QSpinBox, QPlainTextEdit {
            background: #FBF7EF; border: 1px solid #D9CBB2; border-radius: 10px;
            padding: 7px 12px; min-height: 20px;
            selection-background-color: #2F5D50; selection-color: #FFFFFF;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QPlainTextEdit:focus {
            border: 1px solid #2F5D50; background: #FFFFFF;
        }
        QComboBox, QDateEdit { padding-right: 30px; }
        QSpinBox { padding-right: 28px; }

        QComboBox::drop-down, QDateEdit::drop-down {
            subcontrol-origin: padding; subcontrol-position: center right;
            width: 28px; border: none;
        }
        QComboBox::down-arrow, QDateEdit::down-arrow {
            image: url("@BAWAH@"); width: 12px; height: 12px;
        }
        QSpinBox::up-button {
            subcontrol-origin: border; subcontrol-position: top right;
            width: 22px; border: none; background: transparent; margin: 3px 4px 0px 0px;
        }
        QSpinBox::down-button {
            subcontrol-origin: border; subcontrol-position: bottom right;
            width: 22px; border: none; background: transparent; margin: 0px 4px 3px 0px;
        }
        QSpinBox::up-button:hover, QSpinBox::down-button:hover { background: #EADFCB; border-radius: 5px; }
        QSpinBox::up-arrow { image: url("@ATAS@"); width: 12px; height: 12px; }
        QSpinBox::down-arrow { image: url("@BAWAH@"); width: 12px; height: 12px; }

        QComboBox QAbstractItemView {
            background: #FFFDF8; border: 1px solid #D9CBB2; outline: 0;
            selection-background-color: #DCE9E2; selection-color: #3A2E26;
        }
        QComboBox QLineEdit { border: none; background: transparent; padding: 0px; min-height: 0px; }
        QComboBox QLineEdit:focus { border: none; background: transparent; }

        /* ---------- TOMBOL ---------- */
        QPushButton {
            background: #EADFCB; color: #4A3B2F; border: none;
            border-radius: 10px; padding: 10px 16px; font-weight: 600;
        }
        QPushButton:hover { background: #DFD0B6; }
        QPushButton#primer { background: #2F5D50; color: #FFFFFF; }
        QPushButton#primer:hover { background: #24493F; }
        QPushButton#emas { background: #C9A24D; color: #2B1D14; }
        QPushButton#emas:hover { background: #B8913F; }
        QPushButton#bahaya { background: #F5DDD8; color: #9B2C2C; }
        QPushButton#bahaya:hover { background: #EFC9C1; }

        /* ---------- TABEL ---------- */
        QTableView {
            background: #FFFDF8; alternate-background-color: #FAF4E8;
            border: none; selection-background-color: #DCE9E2; selection-color: #2B1D14;
        }
        QTableView::item { padding: 6px 10px; }
        QHeaderView::section {
            background: #F1E7D3; color: #6B5A4A; border: none;
            border-bottom: 2px solid #C9A24D; padding: 10px; font-weight: 700;
        }
        QTableCornerButton::section { background: #F1E7D3; border: none; }

        /* ---------- SCROLLBAR ---------- */
        QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
        QScrollBar::handle:vertical { background: #D9CBB2; border-radius: 5px; min-height: 30px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }
        QScrollBar::handle:horizontal { background: #D9CBB2; border-radius: 5px; min-width: 30px; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

        /* ---------- DIALOG ---------- */
        QMessageBox { background: #FFFDF8; }
        QMessageBox QLabel { color: #3A2E26; }
        QMessageBox QPushButton { min-width: 84px; }
        QToolTip { background: #FFFDF8; color: #3A2E26; border: 1px solid #D9CBB2; padding: 4px; }
    )QSS");

    qss.replace("@BAWAH@", panahBawah);
    qss.replace("@ATAS@", panahAtas);
    return qss;
}

// ===================== GRAFIK BATANG HORIZONTAL (digambar sendiri) =====================
class GrafikBatang : public QWidget
{
public:
    explicit GrafikBatang(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(90);
    }

    void setData(const QList<QPair<QString, int>> &d)
    {
        data = d;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QFont f = font();
        f.setPointSize(9);
        p.setFont(f);

        if (data.isEmpty()) {
            p.setPen(QColor("#A8977F"));
            p.drawText(rect(), Qt::AlignCenter, "Belum ada data");
            return;
        }

        int maks = 1;
        for (const auto &d : data)
            maks = qMax(maks, d.second);

        const QList<QColor> warna = {QColor("#2F5D50"), QColor("#C9A24D"), QColor("#4C7A66"),
                                     QColor("#B8913F"), QColor("#7A9E8E")};
        const int labelW = 96;
        const int nilaiW = 34;
        const int n = int(data.size());
        const double barisH = double(height()) / n;
        const double tebal = qMin(14.0, barisH * 0.55);
        const double lebarMaks = qMax(10.0, double(width() - labelW - nilaiW));

        for (int i = 0; i < n; ++i) {
            const double top = i * barisH;
            const double y = top + (barisH - tebal) / 2.0;

            p.setPen(QColor("#6B5A4A"));
            p.drawText(QRectF(0, top, labelW - 8, barisH), Qt::AlignVCenter | Qt::AlignLeft,
                       p.fontMetrics().elidedText(data[i].first, Qt::ElideRight, labelW - 8));

            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#F1E7D3"));
            p.drawRoundedRect(QRectF(labelW, y, lebarMaks, tebal), tebal / 2, tebal / 2);

            const double w = qMax(tebal, lebarMaks * data[i].second / maks);
            p.setBrush(warna[i % warna.size()]);
            p.drawRoundedRect(QRectF(labelW, y, w, tebal), tebal / 2, tebal / 2);

            p.setPen(QColor("#3A2E26"));
            p.drawText(QRectF(width() - nilaiW + 8, top, nilaiW - 8, barisH),
                       Qt::AlignVCenter | Qt::AlignLeft, QString::number(data[i].second));
        }
    }

private:
    QList<QPair<QString, int>> data;
};

// ===================== GRAFIK DONAT (digambar sendiri) =====================
class GrafikDonut : public QWidget
{
public:
    explicit GrafikDonut(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(100);
    }

    void setData(const QStringList &l, const QList<int> &n, const QList<QColor> &w)
    {
        label = l;
        nilai = n;
        warna = w;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        int total = 0;
        for (int v : nilai)
            total += v;

        const int sisi = qMax(40, qMin(height() - 6, width() / 2));
        const double tebal = sisi * 0.2;
        const double atas = (height() - sisi) / 2.0;
        const QRectF cincin(4 + tebal / 2, atas + tebal / 2, sisi - tebal, sisi - tebal);

        QPen pena(QColor("#F1E7D3"), tebal);
        pena.setCapStyle(Qt::FlatCap);
        p.setBrush(Qt::NoBrush);

        if (total == 0) {
            p.setPen(pena);
            p.drawEllipse(cincin);
        } else {
            int mulai = 90 * 16;
            for (int i = 0; i < nilai.size(); ++i) {
                if (nilai[i] <= 0)
                    continue;
                const int sapuan = -qRound(5760.0 * nilai[i] / total);
                pena.setColor(warna[i]);
                p.setPen(pena);
                p.drawArc(cincin, mulai, sapuan);
                mulai += sapuan;
            }
        }

        p.setPen(QColor("#3B2A20"));
        p.setFont(QFont("Segoe UI", 15, QFont::Bold));
        p.drawText(QRectF(4, atas, sisi, sisi), Qt::AlignCenter, QString::number(total));

        QFont f = font();
        f.setPointSize(9);
        p.setFont(f);

        const int n = int(label.size());
        const int tinggiBaris = 24;
        const double x = sisi + 28;
        double y = (height() - n * tinggiBaris) / 2.0;

        for (int i = 0; i < n; ++i) {
            p.setPen(Qt::NoPen);
            p.setBrush(warna[i]);
            p.drawEllipse(QRectF(x, y + 7, 10, 10));

            p.setPen(QColor("#6B5A4A"));
            p.drawText(QRectF(x + 18, y, width() - x - 18, tinggiBaris),
                       Qt::AlignVCenter | Qt::AlignLeft,
                       QString("%1   %2").arg(label[i]).arg(nilai[i]));
            y += tinggiBaris;
        }
    }

private:
    QStringList label;
    QList<int> nilai;
    QList<QColor> warna;
};

// ===================== GRAFIK AKTIVITAS (batang berpasangan) =====================
class GrafikTren : public QWidget
{
public:
    explicit GrafikTren(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(170);
    }

    void setData(const QStringList &l, const QList<int> &pinjam, const QList<int> &kembali)
    {
        label = l;
        dataPinjam = pinjam;
        dataKembali = kembali;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QFont f = font();
        f.setPointSize(8);
        p.setFont(f);

        const int n = int(label.size());
        if (n == 0 || dataPinjam.size() != n || dataKembali.size() != n) {
            p.setPen(QColor("#A8977F"));
            p.drawText(rect(), Qt::AlignCenter, "Belum ada data");
            return;
        }

        int maks = 1;
        for (int i = 0; i < n; ++i)
            maks = qMax(maks, qMax(dataPinjam[i], dataKembali[i]));

        const int garis = 4;
        int puncak = qMax(maks, garis);
        if (puncak % garis)
            puncak += garis - puncak % garis;

        const int kiri = 30;
        const int bawah = 22;
        const int atas = 8;
        const int kanan = 4;
        const QRectF area(kiri, atas, width() - kiri - kanan, height() - atas - bawah);

        // garis bantu dan angka sumbu
        for (int i = 0; i <= garis; ++i) {
            const double y = area.bottom() - area.height() * i / garis;
            p.setPen(QPen(QColor("#EADFCB"), 1));
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
            p.setPen(QColor("#A8977F"));
            p.drawText(QRectF(0, y - 8, kiri - 6, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(puncak * i / garis));
        }

        const double lebarGrup = area.width() / n;
        const double tebal = qMin(18.0, lebarGrup * 0.28);

        auto batang = [&](double x, int nilai, const QColor &warna) {
            if (nilai <= 0)
                return;
            const double h = area.height() * nilai / puncak;
            p.setPen(Qt::NoPen);
            p.setBrush(warna);
            p.drawRoundedRect(QRectF(x, area.bottom() - h, tebal, h), 4, 4);
        };

        for (int i = 0; i < n; ++i) {
            const double pusat = area.left() + lebarGrup * (i + 0.5);
            batang(pusat - tebal - 1, dataPinjam[i], QColor("#2F5D50"));
            batang(pusat + 1, dataKembali[i], QColor("#C9A24D"));

            p.setPen(QColor("#6B5A4A"));
            p.drawText(QRectF(pusat - lebarGrup / 2, area.bottom() + 4, lebarGrup, 16),
                       Qt::AlignCenter, label[i]);
        }
    }

private:
    QStringList label;
    QList<int> dataPinjam;
    QList<int> dataKembali;
};

// ===================== DAFTAR PERINGKAT (Top 5) =====================
class DaftarPeringkat : public QWidget
{
public:
    explicit DaftarPeringkat(const QString &satuanTeks, QWidget *parent = nullptr)
        : QWidget(parent), satuan(satuanTeks)
    {
        tata = new QVBoxLayout(this);
        tata->setContentsMargins(0, 0, 0, 0);
        tata->setSpacing(8);
    }

    void setData(const QList<QPair<QString, int>> &data)
    {
        while (QLayoutItem *item = tata->takeAt(0)) {
            delete item->widget();
            delete item;
        }

        if (data.isEmpty()) {
            auto *k = new QLabel("Belum ada data");
            k->setObjectName("info");
            k->setAlignment(Qt::AlignCenter);
            tata->addStretch();
            tata->addWidget(k);
            tata->addStretch();
            return;
        }

        const QStringList warnaRank = {"#C9A24D", "#9AA5A0", "#B9825A"};
        int no = 1;

        for (const auto &d : data) {
            auto *baris = new QWidget;
            auto *h = new QHBoxLayout(baris);
            h->setContentsMargins(0, 0, 0, 0);
            h->setSpacing(10);

            auto *rank = new QLabel(QString::number(no));
            rank->setFixedSize(26, 26);
            rank->setAlignment(Qt::AlignCenter);
            const QString latar = no <= 3 ? warnaRank[no - 1] : QString("#E4D8C3");
            const QString teks = no <= 3 ? QString("#FFFFFF") : QString("#6B5A4A");
            rank->setStyleSheet(QString("background: %1; color: %2; border-radius: 13px; font-weight: 700;")
                                    .arg(latar, teks));

            auto *nama = new QLabel(d.first);
            nama->setObjectName("peringkatNama");
            nama->setToolTip(d.first);
            nama->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

            auto *nilai = new QLabel(QString("%1 %2").arg(d.second).arg(satuan));
            nilai->setObjectName("peringkatNilai");

            h->addWidget(rank);
            h->addWidget(nama, 1);
            h->addWidget(nilai);
            tata->addWidget(baris);
            ++no;
        }
        tata->addStretch();
    }

private:
    QString satuan;
    QVBoxLayout *tata = nullptr;
};

// ===================== FUNGSI BANTU TAMPILAN =====================
inline QLabel *buatLabel(const QString &teks, const char *nama)
{
    auto *l = new QLabel(teks);
    l->setObjectName(nama);
    return l;
}

// Bayangan halus di bawah kartu
inline void tambahBayangan(QWidget *w, int blur = 22, int y = 3, int alpha = 32)
{
    auto *e = new QGraphicsDropShadowEffect(w);
    e->setBlurRadius(blur);
    e->setOffset(0, y);
    e->setColor(QColor(59, 42, 32, alpha));
    w->setGraphicsEffect(e);
}

inline QFrame *buatKartu()
{
    auto *k = new QFrame;
    k->setObjectName("card");
    tambahBayangan(k);
    return k;
}

// Label kecil di atas kolom input
inline QWidget *kolomForm(const QString &label, QWidget *w)
{
    auto *c = new QWidget;
    c->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *v = new QVBoxLayout(c);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(4);
    v->addWidget(buatLabel(label, "label"));
    v->addWidget(w);
    return c;
}

// Dua kolom berdampingan
inline QWidget *barisForm(QWidget *kiri, QWidget *kanan)
{
    auto *c = new QWidget;
    c->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *h = new QHBoxLayout(c);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    h->addWidget(kiri);
    h->addWidget(kanan);
    return c;
}

// Judul, garis emas kecil, dan subjudul di bagian atas halaman
inline QWidget *headerHalaman(const QString &judul, const QString &sub)
{
    auto *c = new QWidget;
    auto *v = new QVBoxLayout(c);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(3);
    v->addWidget(buatLabel(judul, "judulHalaman"));

    auto *aksen = new QFrame;
    aksen->setObjectName("aksen");
    aksen->setFixedSize(54, 4);
    v->addWidget(aksen);

    v->addWidget(buatLabel(sub, "subHalaman"));
    return c;
}

inline void aturTabel(QTableView *t, QAbstractItemModel *m)
{
    t->setModel(m);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setShowGrid(false);
    t->setWordWrap(false);
    t->setFrameShape(QFrame::NoFrame);
    t->verticalHeader()->setVisible(false);
    t->verticalHeader()->setDefaultSectionSize(42);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    t->horizontalHeader()->setHighlightSections(false);
}

// Dipanggil setiap selesai mengisi model: lebar kolom, tooltip, dan pesan "Belum ada data"
inline void rapikanKolom(QTableView *t, int kolomRegang, bool sembunyikanId)
{
    if (sembunyikanId)
        t->hideColumn(0);

    auto *h = t->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::ResizeToContents);
    h->setSectionResizeMode(kolomRegang, QHeaderView::Stretch);
    h->setMinimumSectionSize(60);

    auto *m = qobject_cast<QStandardItemModel *>(t->model());
    if (m) {
        for (int r = 0; r < m->rowCount(); ++r)
            for (int c = 0; c < m->columnCount(); ++c)
                if (auto *it = m->item(r, c))
                    it->setToolTip(it->text());
    }

    QLabel *kosong = t->viewport()->findChild<QLabel *>("kosong");
    if (!kosong) {
        kosong = new QLabel("📭  Belum ada data", t->viewport());
        kosong->setObjectName("kosong");
        auto *l = new QVBoxLayout(t->viewport());
        l->addWidget(kosong, 0, Qt::AlignCenter);
    }
    kosong->setVisible(!m || m->rowCount() == 0);
}

// Warna teks status: Dipinjam (emas tua), Terlambat (merah), Dikembalikan (hijau)
inline void warnaiStatus(QStandardItem *item)
{
    if (!item)
        return;
    QColor warna("#8A6D1F");
    const QString s = item->text();
    if (s == "Terlambat")
        warna = QColor("#B85042");
    else if (s == "Dikembalikan")
        warna = QColor("#2F5D50");
    item->setForeground(QBrush(warna));
    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);
}

// Pesan error SQL Server: data sudah ada / masih dipakai tabel lain
inline bool galatDuplikat(const QString &pesan)
{
    return pesan.contains("UNIQUE", Qt::CaseInsensitive)
    || pesan.contains("duplicate", Qt::CaseInsensitive);
}

inline bool galatRelasi(const QString &pesan)
{
    return pesan.contains("REFERENCE", Qt::CaseInsensitive)
    || pesan.contains("FOREIGN KEY", Qt::CaseInsensitive);
}

inline bool konfirmasi(QWidget *induk, const QString &pesan)
{
    return QMessageBox::question(induk, "Konfirmasi", pesan,
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No) == QMessageBox::Yes;
}

// 15000 -> "Rp 15.000"
inline QString rupiah(int n)
{
    QString s = QString::number(qAbs(n));
    for (int i = int(s.size()) - 3; i > 0; i -= 3)
        s.insert(i, QChar('.'));
    return QString("Rp ") + (n < 0 ? "-" : "") + s;
}

// Contoh: "Kamis, 15 Oktober 2026"
inline QString tanggalIndonesia(const QDate &d)
{
    const QStringList hari = {"Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu"};
    const QStringList bulan = {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli",
                               "Agustus", "September", "Oktober", "November", "Desember"};
    return QString("%1, %2 %3 %4")
        .arg(hari[d.dayOfWeek() - 1])
        .arg(d.day())
        .arg(bulan[d.month() - 1])
        .arg(d.year());
}

// Angka naik perlahan dari nilai lama ke nilai baru
inline void animasiAngka(QLabel *lbl, int target)
{
    const int awal = lbl->property("nilaiAnim").toInt();
    lbl->setProperty("nilaiAnim", target);

    if (awal == target) {
        lbl->setText(QString::number(target));
        return;
    }

    auto *a = new QVariantAnimation(lbl);
    a->setDuration(750);
    a->setStartValue(awal);
    a->setEndValue(target);
    a->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(a, &QVariantAnimation::valueChanged, lbl, [lbl](const QVariant &v) {
        lbl->setText(QString::number(v.toInt()));
    });
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

// Halaman muncul perlahan (fade in)
inline void animasiMuncul(QWidget *w)
{
    auto *efek = new QGraphicsOpacityEffect(w);
    efek->setOpacity(0.0);
    w->setGraphicsEffect(efek);

    auto *a = new QPropertyAnimation(efek, "opacity", w);
    a->setDuration(280);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(a, &QPropertyAnimation::finished, w, [w]() { w->setGraphicsEffect(nullptr); });
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

// Ekspor isi tabel ke CSV (kolom pertama = ID, tidak ikut diekspor)
inline void eksporModelCsv(QWidget *induk, QStandardItemModel *m, const QString &namaFile)
{
    if (m->rowCount() == 0) {
        QMessageBox::information(induk, "Info", "Tidak ada data untuk diekspor.");
        return;
    }

    const QString path = QFileDialog::getSaveFileName(induk, "Ekspor ke CSV", namaFile,
                                                      "File CSV (*.csv)");
    if (path.isEmpty())
        return;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(induk, "Gagal", "File tidak dapat dibuat.");
        return;
    }

    auto esc = [](QString s) {
        s.replace("\"", "\"\"");
        return QString("\"") + s + QString("\"");
    };

    QStringList kepala;
    for (int c = 1; c < m->columnCount(); ++c)
        kepala << esc(m->headerData(c, Qt::Horizontal).toString());
    QString out = kepala.join(",") + "\r\n";

    for (int r = 0; r < m->rowCount(); ++r) {
        QStringList baris;
        for (int c = 1; c < m->columnCount(); ++c)
            baris << esc(m->item(r, c)->text());
        out += baris.join(",") + "\r\n";
    }

    f.write("\xEF\xBB\xBF");   // BOM agar Excel membaca UTF-8 dengan benar
    f.write(out.toUtf8());
    f.close();

    QMessageBox::information(induk, "Berhasil", "Data berhasil diekspor.");
}