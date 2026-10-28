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

    ui->dateEdit->setDisplayFormat("dd/MM/yyyy");
    ui->timeEdit->setDisplayFormat("HH:mm:ss");
    ui->radioButtonDark->setChecked(true);
    ui->btnChargerEnable->setChecked(true);
    ui->btnChargerEnable->setText("CHARGER ON");
    clockTimer = new QTimer(this);      //Create timer
    // Show values immediately, then update every second
    updateClockLabels();
    connect(clockTimer, &QTimer::timeout, this, &MainWindow::updateClockLabels);
    clockTimer->start(1000);

    // Numeric keypad for the calibration page: hidden until a value is tapped
    keypad = new NumericKeypad(ui->pageCalibration);
    for (QDoubleSpinBox *spin : {ui->spin_measured_value, ui->spin_offset_value})
    {
        spin->setRange(-9999.99, 9999.99);   // allow negative values; adjust as needed
        spin->setLocale(QLocale::c());       // always use "." as the decimal point
        keypad->registerEdit(spin);
    }

    // Numeric keypad for the date and time page. Tap a part of the date or time
    // (day, month, year, hour...) and type; it moves on to the next part by itself.
    dateTimeKeypad = new NumericKeypad(ui->pageDate_settings);
    dateTimeKeypad->registerEdit(ui->dateEdit);
    dateTimeKeypad->registerEdit(ui->timeEdit);

    // Live data from the rs485d daemon (it handles the RS485 link and CRC).
    // Ask for the analogue inputs twice a second and show them on the dashboard.
    rs485 = new Rs485Client(this);

    connect(rs485, &Rs485Client::analogueInputsReceived, this, [this](const Measurements &m)
            {
                if (!daemonOk)
                {
                    daemonOk = true;
                    qDebug() << "rs485d: connected";
                }

                ui->label_batt_volt_value_2->setText(QString::number(m.vBatt, 'f', 1) + " V");

                // The daemon may send only vBatt for now; show "--" for whatever it did not send
                if (m.complete)
                {
                    ui->label_batt_current_value_2->setText(QString::number(m.iBatt, 'f', 1) + " A");
                    ui->label_batt_mode_value_4->setText(QString::number(m.tBatt, 'f', 1) + " C");
                    ui->label_load_volt_value_2->setText(QString::number(m.vLoad, 'f', 1) + " V");
                    ui->label_load_current_value_2->setText(QString::number(m.iLoad, 'f', 1) + " A");
                }
                else
                {
                    for (QLabel *label : {ui->label_batt_current_value_2,
                                          ui->label_batt_mode_value_4,
                                          ui->label_load_volt_value_2,
                                          ui->label_load_current_value_2})
                    {
                        label->setText("--");
                    }
                }
            });

    connect(rs485, &Rs485Client::failed, this, [this](const QString &reason)
            {
                if (daemonOk)
                {
                    daemonOk = false;
                    qDebug() << "rs485d: lost connection:" << reason;
                }

                // Don't leave old numbers on screen as if they were live
                for (QLabel *label : {ui->label_batt_volt_value_2,
                                      ui->label_batt_current_value_2,
                                      ui->label_batt_mode_value_4,
                                      ui->label_load_volt_value_2,
                                      ui->label_load_current_value_2})
                {
                    label->setText("--");
                }
            });

    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, rs485, &Rs485Client::requestAnalogueInputs);
    pollTimer->start(500);
}

// System time plus whatever offset was saved on the date/time page
QDateTime MainWindow::appNow() const
{
    return QDateTime::currentDateTime().addSecs(clockOffsetSecs);
}

void MainWindow::updateClockLabels()
{
    const QDateTime now = appNow();
    ui->label_date_2->setText(now.date().toString("dd/MM/yyyy"));
    ui->label_time_2->setText(now.time().toString("HH:mm:ss"));
}

// Fill the editors with the current date and time, then open the page
void MainWindow::showDateTimePage()
{
    const QDateTime now = appNow();
    ui->dateEdit->setDate(now.date());
    ui->timeEdit->setTime(QTime(now.time().hour(), now.time().minute(), now.time().second()));
    ui->stackedWidget->setCurrentWidget(ui->pageDate_settings);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_home_clicked()
{
     ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}


void MainWindow::on_pushButton_menu_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}


void MainWindow::on_pushButtonDate_settings_clicked()
{
    showDateTimePage();
}


void MainWindow::on_pushButtonDateBack_clicked()
{
     ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}


void MainWindow::on_pushButtonDateSave_clicked()
{
    // Combine the chosen date and time, and work out how far it is from the system clock
    const QTime t = ui->timeEdit->time();
    const QDateTime selected(ui->dateEdit->date(), QTime(t.hour(), t.minute(), t.second()));
    clockOffsetSecs = selected.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch();   // whole seconds, so the saved time shows exactly

    qDebug() << "Saved date and time:" << selected.toString("yyyy-MM-dd HH:mm:ss");

    updateClockLabels();    // dashboard shows the new date and time straight away
    clockTimer->start(1000);    // restart the one-second tick so it lines up with the new time
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}


void MainWindow::on_pushButtonMenuHome_clicked()
{
     ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}


void MainWindow::on_pushButtonDisplaySave_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}


void MainWindow::on_pushButtonDisplayBack_clicked()
{
     ui->stackedWidget->setCurrentWidget(ui->pageMenu);
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
    ui->stackedWidget->setCurrentWidget(ui->pageDisplay_settings);
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
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}


void MainWindow::on_pushButtonChargerenabledisable_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageEnable_Disable);
}


void MainWindow::on_pushButtonBattery_settings_clicked()
{

}


void MainWindow::on_pushButton_5_clicked()
{

}


void MainWindow::on_pushButton_12_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}


void MainWindow::on_pushButton_Calibration_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageCalibration);
}

