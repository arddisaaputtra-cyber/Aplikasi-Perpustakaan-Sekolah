#include "mainwindow.h"
#include "Database.h"
#include "LoginDialog.h"
#include "Tema.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Sekolah");
    QCoreApplication::setApplicationName("Aplikasi_Perpustakaan_Ardi");
    a.setStyle("Fusion");
    a.setStyleSheet(temaPerpustakaan());

    if (!Database::connect()) {
        QMessageBox::critical(nullptr, "Koneksi SQL Server Gagal",
                              "Tidak bisa terhubung ke " + Database::server +
                                  "\n\nDetail:\n" + Database::pesanError);
        return 1;
    }

    // Login dulu. Setelah "Keluar", layar login muncul kembali.
    int hasil = 0;
    bool ulang = true;
    while (ulang) {
        ulang = false;

        LoginDialog login;
        if (login.exec() != QDialog::Accepted)
            break;

        MainWindow w;
        QObject::connect(&w, &MainWindow::logout, &w, [&]() {
            ulang = true;
            w.close();
        });
        w.showMaximized();
        hasil = a.exec();
    }
    return hasil;
}