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
#include <QMessageBox>
#include <cmath>

// ---- Calibration -----------------------------------------------------------
// One entry per item in combo_calibration_target, in the same order. The list
// position is also the channel number sent to the SP515 (0..8).
// NOTE: confirm this order against the SP515 firmware before relying on it.
struct CalibrationChannel
{
    float Measurements::*value;     // where the live reading is found
    const char *unit;
};

static const CalibrationChannel calibrationChannels[] =
{
    { &Measurements::vBatt,   "V" },   // 0  BATTERY VOLTAGE
    { &Measurements::vRect,   "V" },   // 1  RECTIFIER VOLTAGE
    { &Measurements::vLoad,   "V" },   // 2  LOAD VOLTAGE
    { &Measurements::iBatt,   "A" },   // 3  BATTERY CURRENT
    { &Measurements::iRect,   "A" },   // 4  RECTIFIER CURRENT
    { &Measurements::iLoad,   "A" },   // 5  LOAD CURRENT
    { &Measurements::tBatt,   "C" },   // 6  BATTERY TEMPERATURE
    { &Measurements::vEarth,  "V" },   // 7  EARTH FAULT
    { &Measurements::vRipple, "V" },   // 8  RIPPLE VOLTAGE
};
static const int calibrationChannelCount = int(sizeof(calibrationChannels) / sizeof(calibrationChannels[0]));

// The SP515 is assumed to show:   value = raw * gain + offset
// Given what it shows now ("reading") and what a reference meter says
// ("measured"), work out the gain that makes the reading match, keeping the
// current offset. Returns false if the numbers cannot be trusted.
// NOTE: if the SP515 uses a different formula, this is the only place to change.
static bool calculateNewGain(double currentGain, double offset, double reading, double measured, double &newGain)
{
    const double span = reading - offset;
    if (std::fabs(span) < 1e-6)
        return false;                               // reading is zero: nothing to scale
    const double factor = (measured - offset) / span;
    if (!(factor > 0.1 && factor < 10.0))
        return false;                               // refuse corrections of more than 10x either way
    newGain = currentGain * factor;
    return true;
}

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
                lastMeasurements = m;
                haveMeasurements = true;
                updateCurrentValueLabel();

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
                haveMeasurements = false;
                updateCurrentValueLabel();

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

    // Calibration page: the "Current Value" label follows the selected item
    connect(ui->combo_calibration_target, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { updateCurrentValueLabel(); });
    connect(rs485, &Rs485Client::analogueCalibrationReceived, this, [this](const AnalogueCalibration &c)
            {
                if (calibrationChannel < 0)
                    return;                                 // not one of ours

                const int channel = calibrationChannel;
                double newGain = 0.0;
                if (!calculateNewGain(c.gain[channel], c.offset[channel],
                                      calibrationReading, calibrationMeasured, newGain))
                {
                    calibrationChannel = -1;
                    setCalibrationBusy(false);
                    showCalibrationMessage("These values cannot be calibrated.\n"
                                           "The reading must not be zero, and the measured value must be "
                                           "within a factor of 10 of the reading.");
                    return;
                }
                rs485->setAnalogueGain(quint8(channel), float(newGain));
            });
    connect(rs485, &Rs485Client::commandFinished, this, [this](quint16 command, bool ok, const QString &message)
            {
                const bool ours = command == Rs485Client::GetAnaGains
                               || command == Rs485Client::SetAnaGain
                               || command == Rs485Client::SetAnaOffset;
                if (!ours)
                    return;
                if (command == Rs485Client::GetAnaGains && ok)
                    return;                                 // the gain is calculated and sent next

                calibrationChannel = -1;
                setCalibrationBusy(false);
                if (!ok)
                    showCalibrationMessage("Calibration failed:\n" + message);
            });
    updateCurrentValueLabel();

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


// ---- Calibration page ------------------------------------------------------

// The live reading for the item selected in the combo box
bool MainWindow::selectedReading(double &value) const
{
    const int index = ui->combo_calibration_target->currentIndex();
    if (!haveMeasurements || index < 0 || index >= calibrationChannelCount)
        return false;
    value = lastMeasurements.*(calibrationChannels[index].value);
    return true;
}

void MainWindow::updateCurrentValueLabel()
{
    double value = 0.0;
    if (selectedReading(value))
    {
        const int index = ui->combo_calibration_target->currentIndex();
        ui->label_current_value->setText(QString::number(value, 'f', 2) + " " + calibrationChannels[index].unit);
    }
    else
    {
        ui->label_current_value->setText("--");
    }
}

void MainWindow::setCalibrationBusy(bool busy)
{
    ui->btn_calibrate->setEnabled(!busy);
    ui->btn_apply_offset->setEnabled(!busy);
    ui->combo_calibration_target->setEnabled(!busy);
}

void MainWindow::showCalibrationMessage(const QString &text)
{
    // Not modal-blocking, so the display keeps updating behind it
    auto *box = new QMessageBox(QMessageBox::Warning, "Calibration", text, QMessageBox::Ok, this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->open();
}

// Calibrate: the user types what a reference meter shows. The gain of the
// selected channel is adjusted so the SP515 reading matches that value.
void MainWindow::on_btn_calibrate_clicked()
{
    ui->spin_measured_value->interpretText();       // pick up a value typed but not yet confirmed

    double reading = 0.0;
    if (!selectedReading(reading))
    {
        showCalibrationMessage("No reading available from the SP515.");
        return;
    }

    calibrationChannel  = ui->combo_calibration_target->currentIndex();
    calibrationMeasured = ui->spin_measured_value->value();
    calibrationReading  = reading;

    setCalibrationBusy(true);
    rs485->requestAnalogueCalibration();            // the new gain is worked out when this arrives
}

// Apply Offset: the offset of the selected channel is set to the typed value.
void MainWindow::on_btn_apply_offset_clicked()
{
    ui->spin_offset_value->interpretText();

    const int channel = ui->combo_calibration_target->currentIndex();
    if (channel < 0 || channel >= calibrationChannelCount)
        return;

    calibrationChannel = channel;
    setCalibrationBusy(true);
    rs485->setAnalogueOffset(quint8(channel), float(ui->spin_offset_value->value()));
}
