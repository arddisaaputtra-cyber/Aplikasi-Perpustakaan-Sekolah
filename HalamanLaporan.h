#pragma once

#include "Database.h"
#include "Tema.h"

#include <QDesktopServices>
#include <QPdfWriter>
#include <QTextDocument>
#include <QUrl>
#include <QtSql>
#include <QtWidgets>

// ===================== BARCODE CODE 39 =====================
inline void gambarBarcode39(QPainter &p, const QRectF &area, const QString &kode)
{
    static const QString karakter = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *";
    static const QStringList pola = {
        "nnnwwnwnn", "wnnwnnnnw", "nnwwnnnnw", "wnwwnnnnn", "nnnwwnnnw",
        "wnnwwnnnn", "nnwwwnnnn", "nnnwnnwnw", "wnnwnnwnn", "nnwwnnwnn",
        "wnnnnwnnw", "nnwnnwnnw", "wnwnnwnnn", "nnnnwwnnw", "wnnnwwnnn",
        "nnwnwwnnn", "nnnnnwwnw", "wnnnnwwnn", "nnwnnwwnn", "nnnnwwwnn",
        "wnnnnnnww", "nnwnnnnww", "wnwnnnnwn", "nnnnwnnww", "wnnnwnnwn",
        "nnwnwnnwn", "nnnnnnwww", "wnnnnnwwn", "nnwnnnwwn", "nnnnwnwwn",
        "wwnnnnnnw", "nwwnnnnnw", "wwwnnnnnn", "nwnnwnnnw", "wwnnwnnnn",
        "nwwnwnnnn", "nwnnnnwnw", "wwnnnnwnn", "nwwnnnwnn", "nwnnwnwnn"
    };

    // Awal dan akhir kode selalu tanda bintang (*)
    QString isi = "*";
    const QString besar = kode.toUpper();
    for (int i = 0; i < besar.size(); ++i) {
        const QChar c = besar.at(i);
        if (c != QLatin1Char('*') && karakter.contains(c))
            isi += c;
    }
    isi += "*";

    // Hitung total lebar (batang tebal = 3 satuan, tipis = 1 satuan)
    int total = 0;
    for (int i = 0; i < isi.size(); ++i) {
        const QString pl = pola[int(karakter.indexOf(isi.at(i)))];
        for (int e = 0; e < pl.size(); ++e)
            total += (pl.at(e) == QLatin1Char('w')) ? 3 : 1;
        total += 1;   // jarak antar karakter
    }
    total -= 1;

    const double satuan = area.width() / total;
    double x = area.left();

    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#2B1D14"));

    for (int i = 0; i < isi.size(); ++i) {
        const QString pl = pola[int(karakter.indexOf(isi.at(i)))];
        for (int e = 0; e < pl.size(); ++e) {
            const double lebar = ((pl.at(e) == QLatin1Char('w')) ? 3 : 1) * satuan;
            if (e % 2 == 0)   // posisi genap = batang, ganjil = spasi
                p.drawRect(QRectF(x, area.top(), lebar, area.height()));
            x += lebar;
        }
        x += satuan;
    }
    p.restore();
}

// ===================== GAMBAR KARTU ANGGOTA =====================
inline QPixmap gambarKartuAnggota(const QString &kode, const QString &nama, const QString &kelas)
{
    const int W = 1011;   // 85,6 mm pada 300 dpi
    const int H = 638;    // 54 mm pada 300 dpi

    QPixmap pm(W, H);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    QPainterPath bentuk;
    bentuk.addRoundedRect(QRectF(0, 0, W, H), 38, 38);
    p.setClipPath(bentuk);

    // latar hijau
    QLinearGradient latar(0, 0, W, H);
    latar.setColorAt(0.0, QColor("#24493F"));
    latar.setColorAt(0.65, QColor("#2F5D50"));
    latar.setColorAt(1.0, QColor("#4C7A66"));
    p.fillRect(QRectF(0, 0, W, H), QBrush(latar));

    // lingkaran dekoratif
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 18));
    p.drawEllipse(QPointF(W - 70, 30), 230, 230);
    p.setBrush(QColor(201, 162, 77, 45));
    p.drawEllipse(QPointF(W - 150, H + 20), 190, 190);

    // strip emas di atas
    p.fillRect(QRectF(0, 0, W, 16), QColor("#C9A24D"));

    // lencana dan judul
    p.setBrush(QColor("#C9A24D"));
    p.drawRoundedRect(QRectF(50, 52, 80, 80), 20, 20);

    QFont fEmoji("Segoe UI Emoji");
    fEmoji.setPixelSize(44);
    p.setFont(fEmoji);
    p.setPen(QColor("#2B1D14"));
    p.drawText(QRectF(50, 52, 80, 80), Qt::AlignCenter, "📚");

    QFont fJudul("Georgia");
    fJudul.setPixelSize(40);
    fJudul.setBold(true);
    p.setFont(fJudul);
    p.setPen(Qt::white);
    p.drawText(QPointF(152, 92), "PERPUSTAKAAN SEKOLAH");

    QFont fSub("Segoe UI");
    fSub.setPixelSize(22);
    fSub.setLetterSpacing(QFont::AbsoluteSpacing, 4);
    p.setFont(fSub);
    p.setPen(QColor("#E9D8A6"));
    p.drawText(QPointF(154, 126), "KARTU ANGGOTA");

    // lingkaran inisial
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 40));
    p.drawEllipse(QRectF(56, 190, 180, 180));

    QFont fInisial("Georgia");
    fInisial.setPixelSize(96);
    fInisial.setBold(true);
    p.setFont(fInisial);
    p.setPen(Qt::white);
    p.drawText(QRectF(56, 190, 180, 180), Qt::AlignCenter, nama.left(1).toUpper());

    // data anggota
    const int tx = 272;
    const int lebarTeks = W - tx - 50;

    QFont fLabel("Segoe UI");
    fLabel.setPixelSize(19);
    fLabel.setLetterSpacing(QFont::AbsoluteSpacing, 2);

    p.setFont(fLabel);
    p.setPen(QColor("#E9D8A6"));
    p.drawText(QPointF(tx, 214), "NAMA");

    QFont fNama("Georgia");
    fNama.setPixelSize(44);
    fNama.setBold(true);
    p.setFont(fNama);
    p.setPen(Qt::white);
    p.drawText(QPointF(tx, 262), p.fontMetrics().elidedText(nama, Qt::ElideRight, lebarTeks));

    p.setFont(fLabel);
    p.setPen(QColor("#E9D8A6"));
    p.drawText(QPointF(tx, 314), "KODE ANGGOTA");
    p.drawText(QPointF(tx + 330, 314), "KELAS / STATUS");

    QFont fNilai("Segoe UI");
    fNilai.setPixelSize(36);
    fNilai.setBold(true);
    p.setFont(fNilai);
    p.setPen(Qt::white);
    p.drawText(QPointF(tx, 358), kode);
    p.drawText(QPointF(tx + 330, 358),
               p.fontMetrics().elidedText(kelas.isEmpty() ? QString("-") : kelas,
                                          Qt::ElideRight, lebarTeks - 330));

    // panel putih untuk barcode
    const QRectF panel(50, 408, W - 100, 192);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#FFFDF8"));
    p.drawRoundedRect(panel, 22, 22);

    gambarBarcode39(p, QRectF(panel.left() + 70, panel.top() + 22, panel.width() - 140, 108), kode);

    QFont fKode("Segoe UI");
    fKode.setPixelSize(26);
    fKode.setLetterSpacing(QFont::AbsoluteSpacing, 6);
    p.setFont(fKode);
    p.setPen(QColor("#3A2E26"));
    p.drawText(QRectF(panel.left(), panel.top() + 138, panel.width(), 40), Qt::AlignCenter, kode);

    p.end();
    return pm;
}

// ===================== DIALOG PRATINJAU KARTU =====================
class DialogKartu : public QDialog
{
    Q_OBJECT

public:
    DialogKartu(const QPixmap &kartu, const QString &namaDasar, QWidget *parent = nullptr)
        : QDialog(parent), gambar(kartu), dasar(namaDasar)
    {
        setWindowTitle("Kartu Anggota");

        auto *v = new QVBoxLayout(this);
        v->setContentsMargins(24, 20, 24, 20);
        v->setSpacing(14);

        v->addWidget(buatLabel("Pratinjau Kartu Anggota", "judulKartu"));

        auto *pratinjau = new QLabel;
        pratinjau->setAlignment(Qt::AlignCenter);
        pratinjau->setPixmap(gambar.scaledToWidth(620, Qt::SmoothTransformation));
        v->addWidget(pratinjau);

        auto *btnPng = new QPushButton("💾  Simpan Gambar (PNG)");
        btnPng->setObjectName("primer");
        auto *btnPdf = new QPushButton("📄  Simpan PDF (siap cetak)");
        btnPdf->setObjectName("emas");
        auto *btnTutup = new QPushButton("Tutup");
        for (auto *b : {btnPng, btnPdf, btnTutup})
            b->setCursor(Qt::PointingHandCursor);

        auto *baris = new QHBoxLayout;
        baris->setSpacing(10);
        baris->addWidget(btnPng);
        baris->addWidget(btnPdf);
        baris->addWidget(btnTutup);
        v->addLayout(baris);

        connect(btnPng, &QPushButton::clicked, this, &DialogKartu::simpanPng);
        connect(btnPdf, &QPushButton::clicked, this, &DialogKartu::simpanPdf);
        connect(btnTutup, &QPushButton::clicked, this, &QDialog::accept);
    }

private slots:
    void simpanPng()
    {
        const QString path = QFileDialog::getSaveFileName(this, "Simpan Kartu", dasar + ".png",
                                                          "Gambar PNG (*.png)");
        if (path.isEmpty())
            return;

        if (gambar.save(path, "PNG"))
            QMessageBox::information(this, "Berhasil", "Kartu berhasil disimpan.");
        else
            QMessageBox::critical(this, "Gagal", "Kartu tidak dapat disimpan.");
    }

    void simpanPdf()
    {
        const QString path = QFileDialog::getSaveFileName(this, "Simpan Kartu PDF", dasar + ".pdf",
                                                          "File PDF (*.pdf)");
        if (path.isEmpty())
            return;

        {
            QPdfWriter w(path);
            w.setPageSize(QPageSize(QPageSize::A4));
            w.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);
            w.setResolution(300);

            QPainter p(&w);
            const int x = (w.width() - gambar.width()) / 2;
            const int y = 300;

            p.setPen(QPen(QColor("#999999"), 2, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawRect(x - 12, y - 12, gambar.width() + 24, gambar.height() + 24);
            p.drawPixmap(x, y, gambar);
            p.end();
        }

        if (QMessageBox::question(this, "Berhasil", "Kartu berhasil disimpan.\nBuka file sekarang?")
            == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }

private:
    QPixmap gambar;
    QString dasar;
};

// ===================== HALAMAN LAPORAN & CETAK =====================
class HalamanLaporan : public QWidget
{
    Q_OBJECT

public:
    explicit HalamanLaporan(QWidget *parent = nullptr) : QWidget(parent)
    {
        bangunUI();
        segarkan();
    }

public slots:
    // Mengisi ulang daftar anggota untuk kartu
    void segarkan()
    {
        const QVariant lama = cmbAnggota->currentData();
        cmbAnggota->clear();

        QSqlQuery q;
        if (q.exec("SELECT id, kode, nama FROM anggota ORDER BY nama")) {
            while (q.next())
                cmbAnggota->addItem(q.value(1).toString() + " - " + q.value(2).toString(),
                                    q.value(0));
        }
        q.finish();

        const int i = cmbAnggota->findData(lama);
        if (i >= 0)
            cmbAnggota->setCurrentIndex(i);
    }

private slots:
    // ---------- Kartu anggota ----------
    void cetakKartu()
    {
        if (cmbAnggota->currentIndex() < 0) {
            QMessageBox::information(this, "Info", "Belum ada data anggota. Tambahkan anggota dulu.");
            return;
        }

        QSqlQuery q;
        q.prepare("SELECT kode, nama, kelas FROM anggota WHERE id = :id");
        q.bindValue(":id", cmbAnggota->currentData().toInt());
        if (!q.exec() || !q.next()) {
            QMessageBox::warning(this, "Gagal", "Data anggota tidak ditemukan.");
            return;
        }

        const QString kode = q.value(0).toString();
        const QString nama = q.value(1).toString();
        const QString kelas = q.value(2).toString();
        q.finish();

        DialogKartu dlg(gambarKartuAnggota(kode, nama, kelas), "kartu_" + kode, this);
        dlg.exec();
    }

    // ---------- Laporan peminjaman (per periode) ----------
    void laporanPeminjaman()
    {
        const QDate d1 = dtDari->date();
        const QDate d2 = dtSampai->date();
        if (d1 > d2) {
            QMessageBox::warning(this, "Periode salah", "Tanggal awal tidak boleh setelah tanggal akhir.");
            return;
        }

        QSqlQuery q;
        q.prepare(QString(
                      "SELECT a.nama, b.judul, CONVERT(VARCHAR(10), p.tgl_pinjam, 105), "
                      "CONVERT(VARCHAR(10), p.tgl_jatuh_tempo, 105), "
                      "ISNULL(CONVERT(VARCHAR(10), p.tgl_kembali, 105), '-'), "
                      "CASE WHEN p.tgl_kembali IS NOT NULL THEN 'Dikembalikan' "
                      "WHEN CAST(GETDATE() AS DATE) > p.tgl_jatuh_tempo THEN 'Terlambat' ELSE 'Dipinjam' END, "
                      "CASE WHEN p.tgl_kembali IS NOT NULL THEN p.denda "
                      "WHEN CAST(GETDATE() AS DATE) > p.tgl_jatuh_tempo "
                      "THEN DATEDIFF(DAY, p.tgl_jatuh_tempo, CAST(GETDATE() AS DATE)) * %1 ELSE 0 END "
                      "FROM peminjaman p "
                      "JOIN anggota a ON a.id = p.id_anggota "
                      "JOIN buku b ON b.id = p.id_buku "
                      "WHERE p.tgl_pinjam BETWEEN :d1 AND :d2 "
                      "ORDER BY p.tgl_pinjam, p.id").arg(Database::dendaPerHari));
        q.bindValue(":d1", d1.toString("yyyy-MM-dd"));
        q.bindValue(":d2", d2.toString("yyyy-MM-dd"));

        QList<QStringList> baris;
        int totalDenda = 0;
        if (q.exec()) {
            while (q.next()) {
                const int denda = q.value(6).toInt();
                totalDenda += denda;

                QStringList b;
                b << q.value(0).toString() << q.value(1).toString() << q.value(2).toString()
                  << q.value(3).toString() << q.value(4).toString() << q.value(5).toString()
                  << rupiah(denda);
                baris << b;
            }
        }
        q.finish();

        buatPdf("Laporan Peminjaman Buku",
                "Periode: " + d1.toString("dd-MM-yyyy") + " s/d " + d2.toString("dd-MM-yyyy"),
                {"Anggota", "Buku", "Tgl Pinjam", "Jatuh Tempo", "Tgl Kembali", "Status", "Denda"},
                {17, 25, 11, 11, 11, 12, 8}, baris,
                {QString("Total transaksi: %1").arg(baris.size()),
                 "Total denda: " + rupiah(totalDenda)},
                true, "laporan_peminjaman.pdf");
    }

    // ---------- Laporan keterlambatan (kondisi saat ini) ----------
    void laporanTerlambat()
    {
        QSqlQuery q;
        const bool ok = q.exec(
            "SELECT a.nama, ISNULL(a.kelas, ''), b.judul, "
            "CONVERT(VARCHAR(10), p.tgl_jatuh_tempo, 105), "
            "DATEDIFF(DAY, p.tgl_jatuh_tempo, CAST(GETDATE() AS DATE)) "
            "FROM peminjaman p "
            "JOIN anggota a ON a.id = p.id_anggota "
            "JOIN buku b ON b.id = p.id_buku "
            "WHERE p.tgl_kembali IS NULL AND CAST(GETDATE() AS DATE) > p.tgl_jatuh_tempo "
            "ORDER BY p.tgl_jatuh_tempo, p.id");

        QList<QStringList> baris;
        int totalDenda = 0;
        if (ok) {
            while (q.next()) {
                const int hari = q.value(4).toInt();
                const int denda = hari * Database::dendaPerHari;
                totalDenda += denda;

                QStringList b;
                b << q.value(0).toString() << q.value(1).toString() << q.value(2).toString()
                  << q.value(3).toString() << QString::number(hari) + " hari" << rupiah(denda);
                baris << b;
            }
        }
        q.finish();

        buatPdf("Laporan Keterlambatan Pengembalian",
                "Kondisi per " + tanggalIndonesia(QDate::currentDate()),
                {"Anggota", "Kelas", "Buku", "Jatuh Tempo", "Terlambat", "Estimasi Denda"},
                {22, 10, 30, 13, 10, 10}, baris,
                {QString("Total buku terlambat: %1").arg(baris.size()),
                 "Total estimasi denda: " + rupiah(totalDenda)},
                false, "laporan_keterlambatan.pdf");
    }

    // ---------- Daftar buku ----------
    void laporanBuku()
    {
        QSqlQuery q;
        const bool ok = q.exec("SELECT kode, judul, pengarang, penerbit, tahun, kategori, stok "
                               "FROM buku ORDER BY judul");

        QList<QStringList> baris;
        int totalStok = 0;
        if (ok) {
            while (q.next()) {
                totalStok += q.value(6).toInt();

                QStringList b;
                b << q.value(0).toString() << q.value(1).toString() << q.value(2).toString()
                  << q.value(3).toString() << q.value(4).toString() << q.value(5).toString()
                  << q.value(6).toString();
                baris << b;
            }
        }
        q.finish();

        buatPdf("Daftar Koleksi Buku",
                "Per " + tanggalIndonesia(QDate::currentDate()),
                {"Kode", "Judul", "Pengarang", "Penerbit", "Tahun", "Kategori", "Stok"},
                {8, 29, 20, 16, 7, 10, 5}, baris,
                {QString("Total judul: %1").arg(baris.size()),
                 QString("Total stok tersedia: %1").arg(totalStok)},
                true, "daftar_buku.pdf");
    }

    // ---------- Daftar anggota ----------
    void laporanAnggota()
    {
        QSqlQuery q;
        const bool ok = q.exec("SELECT kode, nama, jenis_kelamin, kelas, no_hp, alamat "
                               "FROM anggota ORDER BY nama");

        QList<QStringList> baris;
        if (ok) {
            while (q.next()) {
                QStringList b;
                b << q.value(0).toString() << q.value(1).toString() << q.value(2).toString()
                  << q.value(3).toString() << q.value(4).toString() << q.value(5).toString();
                baris << b;
            }
        }
        q.finish();

        buatPdf("Daftar Anggota Perpustakaan",
                "Per " + tanggalIndonesia(QDate::currentDate()),
                {"Kode", "Nama", "Jenis Kelamin", "Kelas / Status", "No. HP", "Alamat"},
                {9, 22, 12, 11, 16, 25}, baris,
                {QString("Total anggota: %1").arg(baris.size())},
                false, "daftar_anggota.pdf");
    }

private:
    // ---------- Membuat PDF berkop surat dari data tabel ----------
    // Jumlah lebar kolom (persen) sebaiknya 95, sisanya untuk kolom "No".
    void buatPdf(const QString &judul, const QString &subjudul, const QStringList &kepala,
                 const QList<int> &lebar, const QList<QStringList> &baris,
                 const QStringList &ringkasan, bool landscape, const QString &namaFile)
    {
        if (baris.isEmpty()) {
            QMessageBox::information(this, "Info", "Tidak ada data untuk laporan ini.");
            return;
        }

        const QString path = QFileDialog::getSaveFileName(this, "Simpan Laporan PDF", namaFile,
                                                          "File PDF (*.pdf)");
        if (path.isEmpty())
            return;

        const QString waktu = tanggalIndonesia(QDate::currentDate()) + ", "
                              + QTime::currentTime().toString("HH:mm");

        QString html;
        html += "<html><head><style>"
                "body { font-family: 'Segoe UI'; font-size: 10pt; color: #3A2E26; }"
                "</style></head><body>";

        // kop surat
        html += "<table width='100%' cellspacing='0' cellpadding='0'><tr>"
                "<td><span style='font-size:19pt; font-weight:bold; color:#2F5D50;'>"
                "PERPUSTAKAAN SEKOLAH</span><br>"
                "<span style='color:#8A7968;'>Sistem Informasi Perpustakaan</span></td>"
                "<td align='right'><span style='color:#8A7968;'>Dicetak: "
                + waktu.toHtmlEscaped() + "<br>Oleh: "
                + Database::namaPengguna.toHtmlEscaped() + "</span></td>"
                                                           "</tr></table><hr>";

        html += "<p style='font-size:14pt; font-weight:bold;'>" + judul.toHtmlEscaped() + "</p>";
        html += "<p style='color:#6B5A4A;'>" + subjudul.toHtmlEscaped() + "</p>";

        // tabel data
        html += "<table width='100%' border='1' cellspacing='0' cellpadding='5' "
                "style='border-collapse:collapse;'>";

        html += "<tr><th width='5%' align='left' bgcolor='#2F5D50'>"
                "<font color='#FFFFFF'>No</font></th>";
        for (int i = 0; i < kepala.size(); ++i) {
            html += QString("<th width='%1%' align='left' bgcolor='#2F5D50'>"
                            "<font color='#FFFFFF'>%2</font></th>")
                        .arg(lebar[i])
                        .arg(kepala[i].toHtmlEscaped());
        }
        html += "</tr>";

        int no = 0;
        for (const QStringList &b : baris) {
            const QString warna = (no % 2 == 0) ? QString("#FFFFFF") : QString("#FAF4E8");
            ++no;

            html += "<tr>";
            html += QString("<td bgcolor='%1'>%2</td>").arg(warna).arg(no);
            for (const QString &sel : b)
                html += QString("<td bgcolor='%1'>%2</td>").arg(warna, sel.toHtmlEscaped());
            html += "</tr>";
        }
        html += "</table>";

        // ringkasan
        html += "<p>";
        for (const QString &r : ringkasan)
            html += "<b>" + r.toHtmlEscaped() + "</b><br>";
        html += "</p><br>";

        // kolom tanda tangan
        html += "<table width='100%' cellspacing='0' cellpadding='0'><tr>"
                "<td width='50%' align='center'>Mengetahui,<br>Kepala Sekolah"
                "<br><br><br><br>( ............................ )</td>"
                "<td width='50%' align='center'>Petugas Perpustakaan"
                "<br><br><br><br><br><u>" + Database::namaPengguna.toHtmlEscaped() + "</u></td>"
                                                           "</tr></table>";

        html += "</body></html>";

        QTextDocument doc;
        doc.setHtml(html);

        {
            QPdfWriter writer(path);
            writer.setPageSize(QPageSize(QPageSize::A4));
            writer.setPageOrientation(landscape ? QPageLayout::Landscape : QPageLayout::Portrait);
            writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
            writer.setTitle(judul);
            doc.print(&writer);
        }

        if (QMessageBox::question(this, "Laporan tersimpan", "Laporan berhasil dibuat.\nBuka file sekarang?")
            == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }

    void bangunUI()
    {
        auto *utama = new QVBoxLayout(this);
        utama->setContentsMargins(0, 0, 0, 0);
        utama->setSpacing(14);

        utama->addWidget(headerHalaman("Laporan & Cetak",
                                       "Buat laporan PDF berkop surat dan kartu anggota siap cetak."));

        // ---------- Kartu kiri: kartu anggota ----------
        auto *kartuKiri = buatKartu();
        kartuKiri->setFixedWidth(380);
        auto *kl = new QVBoxLayout(kartuKiri);
        kl->setContentsMargins(20, 18, 20, 20);
        kl->setSpacing(10);

        kl->addWidget(buatLabel("Kartu Anggota", "judulKartu"));
        auto *infoKartu = buatLabel("Buat kartu anggota ber-barcode. Bisa disimpan sebagai "
                                    "gambar atau PDF siap cetak.", "info");
        infoKartu->setWordWrap(true);
        kl->addWidget(infoKartu);

        cmbAnggota = new QComboBox;
        kl->addWidget(kolomForm("Pilih Anggota", cmbAnggota));

        auto *btnKartu = new QPushButton("💳  Buat Kartu Anggota");
        btnKartu->setObjectName("primer");
        btnKartu->setCursor(Qt::PointingHandCursor);
        btnKartu->setMinimumHeight(42);
        kl->addSpacing(4);
        kl->addWidget(btnKartu);

        // ---------- Kartu kanan: laporan PDF ----------
        auto *kartuKanan = buatKartu();
        auto *kr = new QVBoxLayout(kartuKanan);
        kr->setContentsMargins(20, 18, 20, 20);
        kr->setSpacing(10);

        kr->addWidget(buatLabel("Laporan PDF", "judulKartu"));
        auto *infoLaporan = buatLabel("Pilih periode (khusus Laporan Peminjaman), lalu klik jenis laporan "
                                      "yang diinginkan.", "info");
        infoLaporan->setWordWrap(true);
        kr->addWidget(infoLaporan);

        const QDate hariIni = QDate::currentDate();
        dtDari = new QDateEdit(QDate(hariIni.year(), hariIni.month(), 1));
        dtSampai = new QDateEdit(hariIni);
        for (auto *d : {dtDari, dtSampai}) {
            d->setCalendarPopup(true);
            d->setDisplayFormat("dd-MM-yyyy");
        }
        kr->addWidget(barisForm(kolomForm("Dari Tanggal", dtDari),
                                kolomForm("Sampai Tanggal", dtSampai)));
        kr->addSpacing(6);

        auto buatTile = [](const QString &teks) {
            auto *b = new QPushButton(teks);
            b->setObjectName("tile");
            b->setCursor(Qt::PointingHandCursor);
            return b;
        };

        auto *tilePinjam = buatTile("📄  Laporan Peminjaman\nRiwayat pinjam per periode");
        auto *tileTerlambat = buatTile("⏰  Laporan Keterlambatan\nBuku yang belum kembali");
        auto *tileBuku = buatTile("📚  Daftar Koleksi Buku\nSeluruh buku dan stok");
        auto *tileAnggota = buatTile("👥  Daftar Anggota\nSeluruh anggota terdaftar");

        auto *grid = new QGridLayout;
        grid->setSpacing(12);
        grid->addWidget(tilePinjam, 0, 0);
        grid->addWidget(tileTerlambat, 0, 1);
        grid->addWidget(tileBuku, 1, 0);
        grid->addWidget(tileAnggota, 1, 1);
        kr->addLayout(grid);

        auto *konten = new QHBoxLayout;
        konten->setSpacing(16);
        konten->addWidget(kartuKiri, 0, Qt::AlignTop);
        konten->addWidget(kartuKanan, 1, Qt::AlignTop);
        utama->addLayout(konten);
        utama->addStretch();

        connect(btnKartu, &QPushButton::clicked, this, &HalamanLaporan::cetakKartu);
        connect(tilePinjam, &QPushButton::clicked, this, &HalamanLaporan::laporanPeminjaman);
        connect(tileTerlambat, &QPushButton::clicked, this, &HalamanLaporan::laporanTerlambat);
        connect(tileBuku, &QPushButton::clicked, this, &HalamanLaporan::laporanBuku);
        connect(tileAnggota, &QPushButton::clicked, this, &HalamanLaporan::laporanAnggota);
    }

    QComboBox *cmbAnggota = nullptr;
    QDateEdit *dtDari = nullptr;
    QDateEdit *dtSampai = nullptr;
};