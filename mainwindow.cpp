#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "darktheme.h"
#include "lighttheme.h"
#include "highcontrasttheme.h"
#include <QApplication>
#include <QDate>
#include <QTime>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , darkMode(true)
{
    ui->setupUi(this);
    qApp->setStyleSheet(darkTheme);

    ui->dateEdit->setDate(QDate::currentDate());
    ui->timeEdit->setTime(QTime::currentTime());
    ui->radioButtonDark->setChecked(true);
    ui->btnChargerEnable->setChecked(true);
    ui->btnChargerEnable->setText("CHARGER ON");
    clockTimer = new QTimer(this);      //Create timer
    // Show values immediately
    ui->label_date_2->setText(
        QDate::currentDate().toString("dd/MM/yyyy"));

    ui->label_time_2->setText(
        QTime::currentTime().toString("HH:mm:ss"));
    // Update every second
    connect(clockTimer, &QTimer::timeout, this, [this]()
            {
                ui->label_date_2->setText(
                    QDate::currentDate().toString("dd/MM/yyyy"));

                ui->label_time_2->setText(
                    QTime::currentTime().toString("HH:mm:ss"));
            });

    clockTimer->start(1000);

    // Numeric keypad for the calibration page: hidden until a value is tapped
    keypad = new NumericKeypad(ui->pageCalibration);
    for (QDoubleSpinBox *spin : {ui->spin_measured_value, ui->spin_offset_value})
    {
        spin->setRange(-9999.99, 9999.99);   // allow negative values; adjust as needed
        spin->setLocale(QLocale::c());       // always use "." as the decimal point
        keypad->registerEdit(spin);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_home_clicked()
{
     ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_pushButton_menu_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_pushButtonDate_settings_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}


void MainWindow::on_pushButtonTime_settings_clicked()
{
    ui->stackedWidget->setCurrentIndex(6);
}


void MainWindow::on_pushButtonDateBack_clicked()
{
     ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_pushButtonTimeBack_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_pushButtonDateSave_clicked()
{
    QDate selectedDate = ui->dateEdit->date();

    qDebug() << "Selected Date:"
             << selectedDate.toString("yyyy-MM-dd");
}


void MainWindow::on_pushButtonTimeSave_clicked()
{
    QTime selectedTime = ui->timeEdit->time();

    qDebug() << "Selected Time:"
             << selectedTime.toString("HH:mm:ss");
}


void MainWindow::on_pushButtonMenuHome_clicked()
{
     ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_pushButtonDisplaySave_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_pushButtonDisplayBack_clicked()
{
     ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_radioButtonDark_clicked()
{
    qApp->setStyleSheet(darkTheme);
}


void MainWindow::on_radioButtonLight_clicked()
{
    qApp->setStyleSheet(lightTheme);
}


void MainWindow::on_radioButtonHighcontrast_clicked()
{
    qApp->setStyleSheet(highContrastTheme);
}


void MainWindow::on_pushButtonDisplay_settings_clicked()
{
    ui->stackedWidget->setCurrentIndex(6);
}


void MainWindow::on_btnChargerEnable_clicked(bool checked)
{
    if (checked)
    {
        ui->btnChargerEnable->setText("CHARGER ON");

        // Enable charger
    }
    else
    {
        ui->btnChargerEnable->setText("CHARGER OFF");

        // Disable charger
    }
}


void MainWindow::on_pushButton_Enable_home_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_pushButtonChargerenabledisable_clicked()
{
    ui->stackedWidget->setCurrentIndex(7);
}


void MainWindow::on_pushButtonBattery_settings_clicked()
{

}


void MainWindow::on_pushButton_5_clicked()
{

}


void MainWindow::on_pushButton_12_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_pushButton_Calibration_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

