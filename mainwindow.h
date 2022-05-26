#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void readSerialData();
    void sendSerialData();

    void on_pushButton_search_clicked();

    void on_pushButton_open_clicked();

    void on_radioButton_recv_hex_clicked();

    void on_radioButton_recv_ascii_clicked();

    void on_pushButton_clear_recv_clicked();

    void on_checkBox_stop_display_clicked(bool checked);

    void on_checkBox_recv_to_file_clicked(bool checked);

    void on_radioButton_send_ascii_clicked();

    void on_radioButton_send_hex_clicked();

    void on_pushButton_clear_send_clicked();

    void on_pushButton_send_clicked();

    void on_checkBox_send_cyclic_clicked(bool checked);

    void on_pushButton_read_clicked();

private:
    Ui::MainWindow *ui;

    QSerialPort serial;

    int TXCounts;
    QLabel TXLabel;
    int RXCounts;
    QLabel RXLabel;
    QPushButton pushButton_countClear;

    bool recvToFile;
    bool display;
    bool recvASCII;

    bool sendASCII;

    QFile recvFile;

    bool sendCyclic;
    int period; //ms
    QTimer sendTimer;
};

#endif // MAINWINDOW_H
