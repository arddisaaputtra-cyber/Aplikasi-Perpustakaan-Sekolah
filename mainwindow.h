#pragma once

#include <QList>
#include <QMainWindow>

class QPushButton;
class QStackedWidget;
class HalamanDashboard;
class HalamanBuku;
class HalamanAnggota;
class HalamanPinjam;
class HalamanLaporan;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:
    void logout();

private slots:
    void pindahHalaman(int indeks);
    void segarkanSemua();

private:
    void buatUI();

    QStackedWidget *tumpukan = nullptr;
    QList<QPushButton *> tombolNav;

    HalamanDashboard *halDashboard = nullptr;
    HalamanBuku *halBuku = nullptr;
    HalamanAnggota *halAnggota = nullptr;
    HalamanPinjam *halPinjam = nullptr;
    HalamanLaporan *halLaporan = nullptr;
};