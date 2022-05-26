#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTime>
#include <QDate>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    RXCounts = 0;
    TXCounts = 0;
    TXLabel.setText("0");
    RXLabel.setText("0");
    this -> ui -> statusBar -> addPermanentWidget(new QLabel("TX"));
    this -> ui -> statusBar -> addPermanentWidget(&TXLabel);
    this -> ui -> statusBar -> addPermanentWidget(new QLabel("RX"));
    this -> ui -> statusBar -> addPermanentWidget(&RXLabel);
    pushButton_countClear.setText("Reset");
    connect(&pushButton_countClear,&QPushButton::clicked,this,[=](){
        RXCounts = 0;
        TXCounts = 0;
        TXLabel.setText("0");
        RXLabel.setText("0");
    });
    this -> ui -> statusBar -> addPermanentWidget(&pushButton_countClear);

    connect(&serial,&QSerialPort::readyRead,this,&MainWindow::readSerialData);
    connect(&serial,&QSerialPort::errorOccurred,this,[=](QSerialPort::SerialPortError portErr){
        this -> ui -> statusBar -> showMessage(QString("Serial error %1").arg(portErr),1000);
    });

    recvToFile = false;
    display = true;
    recvASCII = true;

    sendCyclic = false;
    connect(&sendTimer,&QTimer::timeout,this,&MainWindow::sendSerialData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_search_clicked()
{
    this -> ui  -> comboBox_name -> clear();
    foreach (QSerialPortInfo avaiablePort, QSerialPortInfo::availablePorts()) {
        this -> ui -> comboBox_name -> addItem(avaiablePort.portName());
    }
}


void MainWindow::on_pushButton_open_clicked()
{
    if(this->ui->pushButton_open->text() == "Open")
    {
        serial.setPortName(this->ui->comboBox_name->currentText());
        serial.setBaudRate(this->ui->comboBox_baud->currentText().toInt());
        switch (this -> ui -> comboBox_paity -> currentIndex()){
        case 0: serial.setParity(QSerialPort::NoParity); break;
        case 1: serial.setParity(QSerialPort::EvenParity); break;
        case 2: serial.setParity(QSerialPort::OddParity); break;
        case 3: serial.setParity(QSerialPort::SpaceParity); break;
        case 4: serial.setParity(QSerialPort::MarkParity); break;
        default: serial.setParity(QSerialPort::UnknownParity); break;
        }
        switch (this->ui->comboBox_dataBits->currentText().toInt()){
        case 5: serial.setDataBits(QSerialPort::Data5); break;
        case 6: serial.setDataBits(QSerialPort::Data6); break;
        case 7: serial.setDataBits(QSerialPort::Data7); break;
        case 8: serial.setDataBits(QSerialPort::Data8); break;
        default: serial.setDataBits(QSerialPort::UnknownDataBits); break;
        }
        switch (this->ui->comboBox_stopBits->currentIndex()){
        case 0: serial.setStopBits(QSerialPort::OneStop); break;
        case 1: serial.setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2: serial.setStopBits(QSerialPort::TwoStop); break;
        default: serial.setStopBits(QSerialPort::UnknownStopBits); break;
        }
        serial.setFlowControl(QSerialPort::NoFlowControl);

        if(serial.open(QIODevice::ReadWrite))
        {
            this -> ui -> pushButton_search -> setEnabled(false);
            this -> ui -> comboBox_name -> setEnabled(false);
            this -> ui -> comboBox_baud -> setEnabled(false);
            this -> ui -> comboBox_paity -> setEnabled(false);
            this -> ui -> comboBox_dataBits -> setEnabled(false);
            this -> ui -> comboBox_stopBits -> setEnabled(false);
            this -> ui -> pushButton_open -> setText("Close");
            QSerialPortInfo serialInfo(serial);
        }else {
            QMessageBox::warning(this,"Open Error","Serialport Open Error!");
        }
    }else{
        serial.close();
        this -> ui -> pushButton_search -> setEnabled(true);
        this -> ui -> comboBox_name -> setEnabled(true);
        this -> ui -> comboBox_baud -> setEnabled(true);
        this -> ui -> comboBox_paity -> setEnabled(true);
        this -> ui -> comboBox_dataBits -> setEnabled(true);
        this -> ui -> comboBox_stopBits -> setEnabled(true);
        this -> ui -> pushButton_open -> setText("Open");
    }
}

void MainWindow::readSerialData()
{
    QByteArray recvData = serial.readAll();
    RXCounts += recvData.length();
    RXLabel . setText(QString::number(RXCounts));

    QString newData;
    if(recvASCII)
        newData = QString(recvData);
    else
        newData = QString(recvData.toHex(' '));

    if(display)
    {
        this -> ui -> textBrowser -> append(newData);
    }

    if(recvToFile)
    {
        QTextStream out(&recvFile);
        out << newData;
    }
}

void MainWindow::on_radioButton_recv_hex_clicked()
{
    recvASCII = false;
}

void MainWindow::on_radioButton_recv_ascii_clicked()
{
    recvASCII = true;
}

void MainWindow::on_pushButton_clear_recv_clicked()
{
    this -> ui -> textBrowser -> clear();
}

void MainWindow::on_checkBox_stop_display_clicked(bool checked)
{
    display = !checked;
}

void MainWindow::on_checkBox_recv_to_file_clicked(bool checked)
{
    if(checked)
    {
        recvFile.setFileName(QString("%1_%2.txt").arg(QDate::currentDate().toString("yy_MM_dd")).arg(QTime::currentTime().toString("hh_mm_ss")));
        if(!recvFile.open(QIODevice::WriteOnly))
            this -> ui -> statusBar -> showMessage("File open failed, try again!",1000);
        else{
            recvToFile = true;
        }
    }else{
        recvToFile = false;
        recvFile.close();
    }
}

void MainWindow::on_radioButton_send_ascii_clicked()
{
    if(!sendASCII)
    {
        QString hexStr = this -> ui -> textEdit -> toPlainText();
        QString str = QByteArray::fromHex(hexStr.toLatin1());
        this -> ui -> textEdit -> setText(str);
        sendASCII = true;
    }
}

void MainWindow::on_radioButton_send_hex_clicked()
{
    if(sendASCII)
    {
        QString str = this -> ui -> textEdit -> toPlainText();
        QString hexStr = str.toLatin1().toHex(' ').toUpper();
        this -> ui -> textEdit -> setText(hexStr);
        sendASCII = false;
    }
}

void MainWindow::on_pushButton_clear_send_clicked()
{
    this -> ui -> textEdit ->clear();
}

void MainWindow::on_pushButton_send_clicked()
{
    if(this -> ui -> pushButton_send -> text() == "Send")
    {
        if(serial.isOpen())
        {
            if(sendCyclic)
            {
                sendTimer.start(period);
                this -> ui -> pushButton_send -> setText("Stop");
                this -> ui -> checkBox_send_cyclic -> setEnabled(false);
                this -> ui -> lineEdit_send_period -> setEnabled(false);
            }else{
                sendSerialData();
            }
        }
        else {
            this -> ui -> statusBar -> showMessage("Serial closed, please open first",1000);
        }
    }
    else if(this -> ui -> pushButton_send -> text() == "Stop") {
        sendTimer.stop();
        this -> ui -> pushButton_send -> setText("Send");
        this -> ui -> checkBox_send_cyclic -> setEnabled(true);
        this -> ui -> lineEdit_send_period -> setEnabled(true);
    }
}

void MainWindow::on_checkBox_send_cyclic_clicked(bool checked)
{
    if(checked)
    {
        if(this->ui->lineEdit_send_period -> text().isEmpty())
        {
            QMessageBox::information(this,"Waring","Please edit period first");
            this -> ui -> checkBox_send_cyclic -> setChecked(false);
        }else {
            period = this->ui->lineEdit_send_period -> text().toInt();
            sendCyclic = true;
        }
    }else{
        sendCyclic = false;
    }
}

void MainWindow::sendSerialData()
{
    QByteArray sendData;
    if(sendASCII)
    {
        sendData = this -> ui -> textEdit -> toPlainText().toLatin1();
    }else{
        sendData = QByteArray::fromHex(this -> ui -> textEdit -> toPlainText().toLatin1());
    }

    TXCounts += serial.write(sendData);
    this -> TXLabel . setText(QString::number(TXCounts));
}

void MainWindow::on_pushButton_read_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Please select file","./","TXT(*.txt)");
    QFile sendFile(fileName);
    if(!sendFile.open(QIODevice::ReadOnly))
        QMessageBox::warning(this,"Error","File open failed, please try again");
    QString sendStr = sendFile.readAll();
    this -> ui -> textEdit -> setText(sendStr);
}
