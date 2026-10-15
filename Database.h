#pragma once

#include <QCryptographicHash>
#include <QList>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>

class Database
{
public:
    // ---------- PENGATURAN ----------
    static inline const QString server = "localhost\\SQLEXPRESS07";
    static inline const QString namaDb = "PerpustakaanDB";
    static inline QString pesanError;
    static constexpr int dendaPerHari = 1000;   // denda per hari terlambat (Rp)
    static constexpr int maksPinjam = 3;        // maksimal buku dipinjam per anggota

    // Pengguna yang sedang login
    static inline QString namaPengguna = "Pustakawan";
    static inline QString peranPengguna = "Admin";

    // ---------- Hash kata sandi (SHA-256) ----------
    static QString hashSandi(const QString &sandi)
    {
        const QByteArray h = QCryptographicHash::hash(sandi.toUtf8(), QCryptographicHash::Sha256);
        return QString::fromLatin1(h.toHex());
    }

    // ---------- Mencari driver ODBC SQL Server di Windows ----------
    static QString cariDriver()
    {
        QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers",
                      QSettings::NativeFormat);
        const QStringList tersedia = reg.childKeys();

        const QStringList prioritas = {
            "ODBC Driver 18 for SQL Server",
            "ODBC Driver 17 for SQL Server",
            "ODBC Driver 13 for SQL Server",
            "ODBC Driver 11 for SQL Server",
            "SQL Server Native Client 11.0",
            "SQL Server"
        };
        for (const QString &d : prioritas)
            if (tersedia.contains(d, Qt::CaseInsensitive))
                return d;
        return QString();
    }

    // ---------- Menyusun teks koneksi (Windows Authentication) ----------
    static QString buatKoneksi(const QString &driver, const QString &db)
    {
        QString s = QString("DRIVER={%1};SERVER=%2;DATABASE=%3;Trusted_Connection=Yes;")
        .arg(driver, server, db);
        if (driver.startsWith("ODBC Driver"))
            s += "TrustServerCertificate=Yes;";
        return s;
    }

    // ---------- Membuat database jika belum ada ----------
    static void buatDatabaseJikaBelum(const QString &driver)
    {
        {
            QSqlDatabase setup = QSqlDatabase::addDatabase("QODBC", "setup_conn");
            setup.setDatabaseName(buatKoneksi(driver, "master"));
            if (setup.open()) {
                QSqlQuery q(setup);
                q.exec(QString("IF DB_ID('%1') IS NULL CREATE DATABASE [%1]").arg(namaDb));
            } else {
                pesanError = setup.lastError().text();
            }
            setup.close();
        }
        QSqlDatabase::removeDatabase("setup_conn");
    }

    // ---------- Membuat tabel ----------
    static bool buatTabel(QSqlDatabase &db)
    {
        const QStringList perintah = {
            "IF OBJECT_ID('dbo.buku', 'U') IS NULL "
            "CREATE TABLE dbo.buku ("
            "id INT IDENTITY(1,1) PRIMARY KEY,"
            "kode NVARCHAR(20) NOT NULL UNIQUE,"
            "judul NVARCHAR(150) NOT NULL,"
            "pengarang NVARCHAR(100) NOT NULL,"
            "penerbit NVARCHAR(100) NULL,"
            "tahun INT NULL,"
            "kategori NVARCHAR(50) NULL,"
            "stok INT NOT NULL DEFAULT 1)",

            "IF OBJECT_ID('dbo.anggota', 'U') IS NULL "
            "CREATE TABLE dbo.anggota ("
            "id INT IDENTITY(1,1) PRIMARY KEY,"
            "kode NVARCHAR(20) NOT NULL UNIQUE,"
            "nama NVARCHAR(100) NOT NULL,"
            "jenis_kelamin NVARCHAR(10) NOT NULL,"
            "kelas NVARCHAR(30) NULL,"
            "no_hp NVARCHAR(20) NULL,"
            "alamat NVARCHAR(255) NULL)",

            "IF OBJECT_ID('dbo.peminjaman', 'U') IS NULL "
            "CREATE TABLE dbo.peminjaman ("
            "id INT IDENTITY(1,1) PRIMARY KEY,"
            "id_buku INT NOT NULL REFERENCES dbo.buku(id),"
            "id_anggota INT NOT NULL REFERENCES dbo.anggota(id),"
            "tgl_pinjam DATE NOT NULL,"
            "tgl_jatuh_tempo DATE NOT NULL,"
            "tgl_kembali DATE NULL,"
            "denda INT NOT NULL DEFAULT 0)",

            "IF OBJECT_ID('dbo.pengguna', 'U') IS NULL "
            "CREATE TABLE dbo.pengguna ("
            "id INT IDENTITY(1,1) PRIMARY KEY,"
            "username NVARCHAR(50) NOT NULL UNIQUE,"
            "sandi NVARCHAR(100) NOT NULL,"
            "nama_lengkap NVARCHAR(100) NOT NULL,"
            "peran NVARCHAR(30) NOT NULL)"
        };

        QSqlQuery q(db);
        for (const QString &sql : perintah) {
            if (!q.exec(sql)) {
                pesanError = q.lastError().text();
                return false;
            }
        }
        return true;
    }

    // ---------- Akun awal: admin / admin123 (hanya jika tabel masih kosong) ----------
    static void isiPenggunaAwal(QSqlDatabase &db)
    {
        QSqlQuery cek(db);
        if (cek.exec("SELECT COUNT(*) FROM dbo.pengguna") && cek.next() && cek.value(0).toInt() > 0)
            return;
        cek.finish();

        QSqlQuery q(db);
        q.prepare("INSERT INTO dbo.pengguna (username, sandi, nama_lengkap, peran) "
                  "VALUES (:u, :s, :n, :p)");
        q.bindValue(":u", "admin");
        q.bindValue(":s", hashSandi("admin123"));
        q.bindValue(":n", "Pustakawan");
        q.bindValue(":p", "Administrator");
        q.exec();
    }

    // ---------- Fungsi utama, dipanggil dari main.cpp ----------
    static bool connect()
    {
        if (!QSqlDatabase::isDriverAvailable("QODBC")) {
            pesanError = "Driver QODBC tidak ditemukan di instalasi Qt.";
            return false;
        }

        const QString driver = cariDriver();
        if (driver.isEmpty()) {
            pesanError = "Driver ODBC untuk SQL Server tidak terpasang di Windows.";
            return false;
        }

        buatDatabaseJikaBelum(driver);

        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
        db.setDatabaseName(buatKoneksi(driver, namaDb));

        if (!db.open()) {
            pesanError = db.lastError().text();
            return false;
        }

        if (!buatTabel(db))
            return false;

        isiPenggunaAwal(db);
        return true;
    }

    // ---------- Pemeriksaan login: username bebas, kata sandi harus benar ----------
    static bool periksaLogin(const QString &user, const QString &sandi)
    {
        const QString nama = user.trimmed();
        if (nama.isEmpty())
            return false;

        QSqlQuery q;
        q.prepare("SELECT TOP 1 peran FROM pengguna WHERE sandi = :s");
        q.bindValue(":s", hashSandi(sandi));

        if (q.exec() && q.next()) {
            namaPengguna = nama;
            peranPengguna = q.value(0).toString();
            q.finish();
            return true;
        }
        return false;
    }

    // ---------- Mengisi model tabel dari hasil query ----------
    static void isiModel(QStandardItemModel *m, QSqlQuery &q, const QStringList &judul)
    {
        m->clear();
        m->setHorizontalHeaderLabels(judul);
        const int jumlahKolom = judul.size();

        while (q.next()) {
            QList<QStandardItem *> baris;
            for (int i = 0; i < jumlahKolom; ++i) {
                auto *item = new QStandardItem(q.value(i).toString());
                item->setEditable(false);
                baris << item;
            }
            m->appendRow(baris);
        }
        q.finish();
    }
};