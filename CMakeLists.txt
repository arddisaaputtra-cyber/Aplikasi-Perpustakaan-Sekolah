cmake_minimum_required(VERSION 3.16)
project(Aplikasi_Perpustakaan_Ardi VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

qt_add_executable(Aplikasi_Perpustakaan_Ardi
    main.cpp
    mainwindow.h
    mainwindow.cpp
    Database.h
    Tema.h
    LoginDialog.h
    HalamanDashboard.h
    HalamanBuku.h
    HalamanAnggota.h
    HalamanPinjam.h
    HalamanLaporan.h



    LoginDialog.h
    HalamanLaporan.h
)

target_link_libraries(Aplikasi_Perpustakaan_Ardi PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

set_target_properties(Aplikasi_Perpustakaan_Ardi PROPERTIES
    WIN32_EXECUTABLE ON
)