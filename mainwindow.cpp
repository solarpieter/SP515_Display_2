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
#include <QProgressBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QtEndian>
#include <cmath>
#include <cstring>

// Bigger text and buttons for pop-up windows, so they are easy to read and to tap on the touch screen
static void applyBigPopupStyle(QMessageBox *box)
{
    QFont font = box->font();
    font.setPixelSize(20);
    box->setFont(font);
    box->setStyleSheet("QMessageBox { min-width: 520px; }"
                       "QPushButton { min-width: 140px; min-height: 50px; }");
}

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

namespace {

// Name of a charge mode, from the mode number the SP515 reports in its status.
// The numbers are eChargeMode_t in the SP515 firmware (service_charge.h); keep this list in the same order.
// A number without a name is shown as "MODE n".
QString chargeModeName(quint8 mode)
{
    static const QStringList names = {
        "NONE",         // CHARGE_MODE_NONE = 0
        "FLOAT",        // CHARGE_MODE_FLOAT
        "BOOST",        // CHARGE_MODE_BOOST
        "EQUALIZE",     // CHARGE_MODE_EQUALIZE
        "INITIALIZE",   // CHARGE_MODE_INITIALIZE
        "BATT TEST",    // CHARGE_MODE_BTEST
    };
    if (mode < names.size() && !names.at(mode).isEmpty())
        return names.at(mode);
    return QString("MODE %1").arg(mode);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , darkMode(true)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);     // Qt Designer saves whichever page was open last
    qApp->setStyleSheet(darkTheme);

    ui->dateEdit->setDisplayFormat("dd/MM/yyyy");
    ui->timeEdit->setDisplayFormat("HH:mm:ss");
    ui->radioButtonDark->setChecked(true);
    setChargerUnknown();                                // the switch follows the SP515's own state, once it has been read
    clockTimer = new QTimer(this);      //Create timer
    // Show values immediately, then update every second
    updateClockLabels();
    connect(clockTimer, &QTimer::timeout, this, &MainWindow::updateClockLabels);
    clockTimer->start(1000);

    // Numeric keypad for the calibration page: hidden until a value is tapped
    keypad = new NumericKeypad(ui->pageCalibration);
    keypad->setKeySize(84, 72);
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

    // Enable / Disable page starts with the same text as the dashboard
    ui->label_batt_volt_value_3->setText(ui->label_batt_volt_value_2->text());
    ui->label_batt_current_value_3->setText(ui->label_batt_current_value_2->text());
    ui->label_alarms_value_3->setText(ui->label_alarms_value_2->text());
    for (QLabel *label : {ui->label_4_rect_volt_value, ui->label_rect_curr_value,
                          ui->label_rect_volt_value_4, ui->label_rect_current_value_4})
        label->setText("--");                       // until the first reading arrives
    setSystemInfo("RECTIFIER --", nullptr, "Waiting for the SP515...");

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

                // Dashboard and Enable / Disable page: rectifier voltage and current
                const QString rectV = m.complete ? QString::number(m.vRect, 'f', 1) + " V" : QString("--");
                const QString rectI = m.complete ? QString::number(m.iRect, 'f', 1) + " A" : QString("--");
                ui->label_4_rect_volt_value->setText(rectV);
                ui->label_rect_volt_value_4->setText(rectV);
                ui->label_rect_curr_value->setText(rectI);
                ui->label_rect_current_value_4->setText(rectI);

                // PID page: rectifier voltage and current, three decimals
                ui->label_rectifier_voltage_value->setText(m.complete ? QString::number(m.vRect, 'f', 3) + " V" : QString("--"));
                ui->label_rectifier_current_value->setText(m.complete ? QString::number(m.iRect, 'f', 3) + " A" : QString("--"));

                ui->label_batt_volt_value_2->setText(QString::number(m.vBatt, 'f', 1) + " V");
                ui->label_batt_volt_value_3->setText(ui->label_batt_volt_value_2->text());     // Enable / Disable page

                // The daemon may send only vBatt for now; show "--" for whatever it did not send
                if (m.complete)
                {
                    ui->label_batt_current_value_2->setText(QString::number(m.iBatt, 'f', 1) + " A");
                    ui->label_batt_current_value_3->setText(ui->label_batt_current_value_2->text());   // Enable / Disable page
                    ui->label_batt_mode_value_4->setText(QString::number(m.tBatt, 'f', 1) + " C");
                    ui->label_load_volt_value_2->setText(QString::number(m.vLoad, 'f', 1) + " V");
                    ui->label_load_current_value_2->setText(QString::number(m.iLoad, 'f', 1) + " A");
                }
                else
                {
                    for (QLabel *label : {ui->label_batt_current_value_2,
                                          ui->label_batt_current_value_3,
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
                                      ui->label_batt_volt_value_3,
                                      ui->label_batt_current_value_2,
                                      ui->label_batt_current_value_3,
                                      ui->label_batt_mode_value_4,
                                      ui->label_batt_mode_value_2,
                                      ui->label_load_volt_value_2,
                                      ui->label_load_current_value_2,
                                      ui->label_rectifier_voltage_value,
                                      ui->label_rectifier_current_value,
                                      ui->label_4_rect_volt_value,
                                      ui->label_rect_curr_value,
                                      ui->label_rect_volt_value_4,
                                      ui->label_rect_current_value_4})
                {
                    label->setText("--");
                }
                setSystemInfo("RECTIFIER --", nullptr, "No link to the daemon");
                setChargerUnknown();
                showAlarmState(false, false);
            });

    connect(rs485, &Rs485Client::alarmsReceived, this, [this](const Alarms &a)
            {
                showAlarmState(true, a.any());
            });

    // Dashboard, next to "System Status": is the rectifier really enabled (as reported by the SP515)
    connect(rs485, &Rs485Client::statusReceived, this, [this](const Status &st)
            {
                statusFailures = 0;
                showChargerFromSp515(st.enabled != 0);              // the SP515 decides, not the display
                ui->label_batt_mode_value_2->setText(chargeModeName(st.mode));

                const bool bad = st.configDefaulted != 0;
                setSystemInfo(st.enabled ? "RECTIFIER ENABLED" : "RECTIFIER DISABLED",
                              st.enabled ? "#43a047" : "#fb8c00",
                              bad ? "CONFIG DEFAULTED" : "Config OK",
                              bad ? "#e53935" : nullptr);
            });

    // Calibration page: progress bar and the OK / FAIL labels (found by name, hidden until used)
    calibrationProgress  = ui->pageCalibration->findChild<QProgressBar *>("progress_calibration");
    calibrateResultLabel = ui->pageCalibration->findChild<QLabel *>("label_calibrate_result");
    offsetResultLabel    = ui->pageCalibration->findChild<QLabel *>("label_offset_result");
    if (!calibrationProgress || !calibrateResultLabel || !offsetResultLabel)
        qWarning() << "Calibration page: progress_calibration, label_calibrate_result or label_offset_result not found";
    if (calibrationProgress)
    {
        calibrationProgress->setRange(0, 100);
        calibrationProgress->setTextVisible(false);
        calibrationProgress->setValue(0);               // always visible, empty until a run starts
        calibrationAnimation = new QPropertyAnimation(calibrationProgress, "value", this);
        calibrationAnimation->setDuration(3000);        // how long a calibration run lasts
        calibrationAnimation->setStartValue(0);
        calibrationAnimation->setEndValue(100);
        connect(calibrationAnimation, &QPropertyAnimation::finished, this, &MainWindow::calibrationBarFinished);
    }
    if (calibrateResultLabel)
        calibrateResultLabel->hide();
    if (offsetResultLabel)
        offsetResultLabel->hide();

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
                    reportCalibrationResult(false, "These values cannot be calibrated.\n"
                                                   "The reading must not be zero, and the measured value must be "
                                                   "within a factor of 10 of the reading.");
                    return;
                }
                rs485->setAnalogueGain(quint8(channel), float(newGain));
            });
    connect(rs485, &Rs485Client::commandFinished, this, [this](quint16 command, bool ok, const QString &message)
            {
                if (handleModeCommandFinished(command, ok, message))
                    return;                                 // a boost, equalize or initialize page read or save
                if (handleBatteryCommandFinished(command, ok, message))
                    return;                                 // a battery settings read or save
                if (handlePidCommandFinished(command, ok, message))
                    return;                                 // a PID settings read or save
                if (command == Rs485Client::GetStatus)      // only failures arrive here
                {
                    const bool noAnswer = rs485->lastErrorStatus() == 3;
                    setSystemInfo("RECTIFIER --", nullptr, noAnswer ? "No answer from the SP515" : "No link to the daemon");
                    ui->label_batt_mode_value_2->setText("--");
                    if (++statusFailures >= 2)                      // one missed poll is not enough to grey out the switch
                        setChargerUnknown();
                    return;
                }

                if (command == Rs485Client::SetChargeSetpoint)
                {
                    // The SP515's answer to the ON / OFF request
                    chargerCommandPending = false;
                    ui->btnChargerEnable->setEnabled(chargerStateKnown);
                    if (ok)
                        setChargerState(chargerPendingOn);          // confirmed: the switch moves
                    else
                        showMessageBox("Charger", QString("The charger was NOT switched %1:\n%2")
                                                      .arg(chargerPendingOn ? "ON" : "OFF", message));
                    return;
                }

                const bool ours = command == Rs485Client::GetAnaGains
                               || command == Rs485Client::SetAnaGain
                               || command == Rs485Client::SetAnaOffset;
                if (!ours)
                    return;
                if (command == Rs485Client::GetAnaGains && ok)
                    return;                                 // the gain is calculated and sent next

                calibrationChannel = -1;
                reportCalibrationResult(ok, "Calibration failed:\n" + message);
            });
    updateCurrentValueLabel();

    setupDisplayPage();
    setupBatteryPage();
    setupPidPage();
    setupModePages();

    // Numbers are typed on the numeric keypad; the small up / down arrows on the spin boxes are too small to use.
    // (Date and time boxes keep theirs: they step through day, month, hour and so on.)
    for (QSpinBox *spin : findChildren<QSpinBox *>())
        spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    for (QDoubleSpinBox *spin : findChildren<QDoubleSpinBox *>())
        spin->setButtonSymbols(QAbstractSpinBox::NoButtons);

    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, [this]()
            {
                if (!pidPollTimer->isActive())          // the PID page polls faster by itself
                    rs485->requestAnalogueInputs();
                if (++pollCount % 2 == 0)
                    rs485->requestAlarms();         // alarms every second
                else
                    rs485->requestStatus();         // and the SP515 status every second
            });
    pollTimer->start(500);
}

// Two lines of text next to "System Status" on the dashboard. Colours are optional (nullptr = the theme's text colour).
void MainWindow::setSystemInfo(const QString &line1, const char *color1, const QString &line2, const char *color2)
{
    auto line = [](const QString &text, const char *color, int pt, bool bold)
    {
        return QString("<div style=\"font-size:%1pt;%2%3\">%4</div>")
            .arg(pt)
            .arg(bold ? "font-weight:bold;" : "")
            .arg(color ? QString("color:%1;").arg(color) : QString())
            .arg(text.toHtmlEscaped());
    };
    ui->label_system_info->setText(line(line1, color1, 18, true) + line(line2, color2, 14, false));
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
    // Stop talking to the daemon before the user interface is deleted
    pollTimer->stop();
    rs485->disconnect(this);

    qApp->removeEventFilter(this);
    backlight->setPercent(backlightPercent);        // never leave the screen dim or off behind

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


// System alarms on the dashboard and on the Enable / Disable page (always the same text)
void MainWindow::showAlarmState(bool haveData, bool active)
{
    alarmActive = haveData && active;
    if (screenSaver && powerState == PowerState::ScreenSaver)
        applyPowerState();                          // an alarm appeared or cleared while the screen saver is on

    const QString text  = !haveData ? "--" : (active ? "ALARM" : "NO ALARMS");
    const QString style = (haveData && active) ? "color: #FF3B30; font-weight: bold;" : QString();

    for (QLabel *label : {ui->label_alarms_value_2, ui->label_alarms_value_3})
    {
        label->setText(text);
        label->setStyleSheet(style);
    }
}


void MainWindow::on_btnChargerEnable_clicked(bool checked)
{
    // De-bounce: a second press within 400 ms is ignored and the button goes back to how it was
    if (chargerClickTimer.isValid() && chargerClickTimer.elapsed() < 400)
    {
        ui->btnChargerEnable->setChecked(!checked);
        return;
    }
    chargerClickTimer.restart();

    // The button has just flipped itself. Put it back and ask first: it only changes if the answer is Yes.
    ui->btnChargerEnable->setChecked(!checked);

    auto *box = new QMessageBox(QMessageBox::Question, "Charger",
                                QString("Are you sure you want to switch the charger %1?").arg(checked ? "ON" : "OFF"),
                                QMessageBox::Yes | QMessageBox::No, this);
    box->setDefaultButton(QMessageBox::No);         // Enter, or closing the window, means No
    box->setEscapeButton(QMessageBox::No);
    box->setAttribute(Qt::WA_DeleteOnClose);
    applyBigPopupStyle(box);
    connect(box, &QMessageBox::finished, this, [this, checked](int result)
            {
                if (result == QMessageBox::Yes)
                    requestChargerState(checked);
            });
    box->open();                                    // does not block the display
}

// Asks the SP515 to switch the charger. The switch only moves when the SP515 has confirmed it.
void MainWindow::requestChargerState(bool on)
{
    if (chargerCommandPending)
        return;

    if (on && (chargerOnVoltage <= 0.0f || chargerOnCurrentLimit <= 0.0f))
    {
        showMessageBox("Charger", "The charger voltage and current limit have not been set yet.");
        return;
    }

    chargerCommandPending = true;
    chargerPendingOn = on;
    ui->btnChargerEnable->setEnabled(false);        // no second press while we wait for the answer

    rs485->setChargerSetpoint(on, on ? chargerOnVoltage : 0.0f, on ? chargerOnCurrentLimit : 0.0f);
}

// The SP515 reported its charger state (every second): the switch follows it, so it is always right
// even after the display was restarted or changed. While an ON / OFF request is waiting for its answer
// the switch is left alone; the answer decides.
void MainWindow::showChargerFromSp515(bool enabled)
{
    chargerStateKnown = true;
    if (chargerCommandPending)
        return;
    ui->btnChargerEnable->setEnabled(true);
    setChargerState(enabled);
}

// No state from the SP515 (no link): grey the switch out instead of showing a guess
void MainWindow::setChargerUnknown()
{
    chargerStateKnown = false;
    if (chargerCommandPending)
        return;
    ui->btnChargerEnable->setEnabled(false);
    ui->btnChargerEnable->setChecked(false);
    ui->btnChargerEnable->setText("CHARGER --");
}

void MainWindow::setChargerState(bool on)
{
    ui->btnChargerEnable->setChecked(on);

    if (on)
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
ui->stackedWidget->setCurrentWidget(ui->pageBattery_settings);
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

// A run starts when Calibrate or Apply Offset is pressed. The buttons stay off until
// the bar is full and the result is shown, so they cannot be pressed again too quickly.
// The result is only displayed once the bar has run to 100 %, so every run takes 3 seconds.
void MainWindow::startCalibrationRun(CalibrationOp op)
{
    calibrationOp = op;
    calibrationBarFull = false;
    calibrationResultKnown = false;
    setCalibrationBusy(true);

    if (calibrateResultLabel)
        calibrateResultLabel->hide();
    if (offsetResultLabel)
        offsetResultLabel->hide();

    if (calibrationProgress)
    {
        calibrationProgress->setStyleSheet(QString());      // back to the theme colour
        calibrationProgress->setValue(0);
    }
    if (calibrationAnimation)
        calibrationAnimation->start();
    else
        QTimer::singleShot(3000, this, &MainWindow::calibrationBarFinished);   // no progress bar on the page
}

// The SP515 has answered (or the request was refused). Remember it; it is shown once the bar is full.
void MainWindow::reportCalibrationResult(bool ok, const QString &failMessage)
{
    calibrationResultOk = ok;
    calibrationFailMessage = failMessage;
    calibrationResultKnown = true;

    if (calibrationBarFull)
        showCalibrationResult();            // the answer was slower than the bar
}

void MainWindow::calibrationBarFinished()
{
    calibrationBarFull = true;
    if (calibrationResultKnown)
        showCalibrationResult();            // otherwise it is shown when the answer arrives
}

// Green OK or red FAIL next to the button that was pressed. It stays until the next press.
void MainWindow::showCalibrationResult()
{
    QLabel *label = (calibrationOp == CalibrationOp::ApplyOffset) ? offsetResultLabel : calibrateResultLabel;
    if (label)
    {
        label->setText(calibrationResultOk ? "OK" : "FAIL");
        label->setStyleSheet(calibrationResultOk ? "color: #22B14C; font-weight: bold;"
                                                 : "color: #FF3B30; font-weight: bold;");
        label->show();
    }
    if (!calibrationResultOk)
    {
        if (calibrationProgress)
            calibrationProgress->setStyleSheet("QProgressBar::chunk { background-color: #FF3B30; }");
        showCalibrationMessage(calibrationFailMessage);
    }

    calibrationOp = CalibrationOp::None;
    calibrationResultKnown = false;
    setCalibrationBusy(false);
}

void MainWindow::showCalibrationMessage(const QString &text)
{
    showMessageBox("Calibration", text);
}

void MainWindow::showMessageBox(const QString &title, const QString &text)
{
    // Not modal-blocking, so the display keeps updating behind it
    auto *box = new QMessageBox(QMessageBox::Warning, title, text, QMessageBox::Ok, this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    applyBigPopupStyle(box);
    box->open();
}

// Calibrate: the user types what a reference meter shows. The gain of the
// selected channel is adjusted so the SP515 reading matches that value.
void MainWindow::on_btn_calibrate_clicked()
{
    ui->spin_measured_value->interpretText();       // pick up a value typed but not yet confirmed

    startCalibrationRun(CalibrationOp::Calibrate);

    double reading = 0.0;
    if (!selectedReading(reading))
    {
        reportCalibrationResult(false, "No reading available from the SP515.");
        return;
    }

    calibrationChannel  = ui->combo_calibration_target->currentIndex();
    calibrationMeasured = ui->spin_measured_value->value();
    calibrationReading  = reading;

    rs485->requestAnalogueCalibration();            // the new gain is worked out when this arrives
}

// Apply Offset: the offset of the selected channel is set to the typed value.
void MainWindow::on_btn_apply_offset_clicked()
{
    ui->spin_offset_value->interpretText();

    const int channel = ui->combo_calibration_target->currentIndex();
    if (channel < 0 || channel >= calibrationChannelCount)
        return;

    startCalibrationRun(CalibrationOp::ApplyOffset);
    calibrationChannel = channel;
    rs485->setAnalogueOffset(quint8(channel), float(ui->spin_offset_value->value()));
}

void MainWindow::on_pushButton_6_clicked()
{
ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}


void MainWindow::on_pushButton_menu_2_clicked()
{
 ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}


// ---- Display page: back light, dimming and screen saver ---------------------

void MainWindow::setupDisplayPage()
{
    // The radio buttons are no longer inside one layout, so the groups are made here.
    // A QButtonGroup keeps them exclusive wherever they are placed on the page.
    themeGroup = new QButtonGroup(this);
    themeGroup->addButton(ui->radioButtonDark);
    themeGroup->addButton(ui->radioButtonLight);
    themeGroup->addButton(ui->radioButtonHighcontrast);
    ui->radioButtonDark->setChecked(true);

    keyboardGroup = new QButtonGroup(this);
    keyboardGroup->addButton(ui->radioButton_Keyboard_On);
    keyboardGroup->addButton(ui->radioButton_Keyboard_Off);
    keyboardGroup->addButton(ui->radioButton_Keyboard_Auto);
    if (!keyboardGroup->checkedButton())
        ui->radioButton_Keyboard_Auto->setChecked(true);

    // Saved settings
    QSettings settings("SP515", "SP515_Display_2");
    backlightPercent  = settings.value("display/backlightPercent", 100).toInt();
    dimAfterS         = settings.value("display/dimAfterS", 60).toInt();
    screensaverAfterS = settings.value("display/screensaverAfterS", 300).toInt();

    backlight = new Backlight(this);
    screenSaver = new ScreenSaver(this);

    ui->horizontalSlider_BackLight->setRange(10, 100);      // never fully dark from the slider
    backlightPercent = qBound(ui->horizontalSlider_BackLight->minimum(), backlightPercent, ui->horizontalSlider_BackLight->maximum());
    ui->horizontalSlider_BackLight->setValue(backlightPercent);
    ui->spin_dim_after->setValue(dimAfterS);
    ui->spin_screensaver_after->setValue(screensaverAfterS);
    backlight->setPercent(backlightPercent);

    // Numeric keypad for the two timers
    displayKeypad = new NumericKeypad(ui->pageDisplay_settings);
    displayKeypad->setKeySize(84, 72);
    displayKeypad->registerEdit(ui->spin_dim_after);
    displayKeypad->registerEdit(ui->spin_screensaver_after);

    // Changes take effect at once and are saved at once
    connect(ui->horizontalSlider_BackLight, &QSlider::valueChanged, this, [this](int value)
            {
                backlightPercent = value;
                if (powerState == PowerState::Normal)
                    backlight->setPercent(backlightPercent);
                saveDisplaySettings();
            });
    connect(ui->spin_dim_after, qOverload<int>(&QSpinBox::valueChanged), this, [this](int seconds)
            {
                dimAfterS = seconds;
                saveDisplaySettings();
            });
    connect(ui->spin_screensaver_after, qOverload<int>(&QSpinBox::valueChanged), this, [this](int seconds)
            {
                screensaverAfterS = seconds;
                saveDisplaySettings();
            });

    // Idle timers: every touch or key press restarts them
    idleClock.start();
    idleTimer = new QTimer(this);
    connect(idleTimer, &QTimer::timeout, this, &MainWindow::checkIdle);
    idleTimer->start(500);
    qApp->installEventFilter(this);
}

void MainWindow::saveDisplaySettings()
{
    QSettings settings("SP515", "SP515_Display_2");
    settings.setValue("display/backlightPercent", backlightPercent);
    settings.setValue("display/dimAfterS", dimAfterS);
    settings.setValue("display/screensaverAfterS", screensaverAfterS);
}

// Called twice a second: has the display been left alone long enough to dim or to start the screen saver?
void MainWindow::checkIdle()
{
    if (QApplication::activeModalWidget())
    {
        idleClock.restart();                        // a pop-up is waiting for an answer: not idle
        return;
    }

    const qint64 idleS = idleClock.elapsed() / 1000;

    PowerState target = PowerState::Normal;
    if (screensaverAfterS > 0 && idleS >= screensaverAfterS)
        target = PowerState::ScreenSaver;
    else if (dimAfterS > 0 && idleS >= dimAfterS)
        target = PowerState::Dimmed;

    if (target != powerState)
        setPowerState(target);
}

void MainWindow::userActivity()
{
    idleClock.restart();
    if (powerState != PowerState::Normal)
        setPowerState(PowerState::Normal);
}

void MainWindow::setPowerState(PowerState state)
{
    powerState = state;
    applyPowerState();
}

// A quarter of the chosen brightness, but not below 5 %
int MainWindow::dimPercent() const
{
    return qMin(backlightPercent, qMax(5, backlightPercent / 4));
}

void MainWindow::applyPowerState()
{
    switch (powerState)
    {
    case PowerState::Normal:
        screenSaver->stop();
        backlight->setPercent(backlightPercent);
        break;

    case PowerState::Dimmed:
        backlight->setPercent(dimPercent());
        break;

    case PowerState::ScreenSaver:
        // Dark screen. Only an alarm lights it up: dim, with a red button bouncing around.
        screenSaver->start(alarmActive);
        backlight->setPercent(alarmActive ? dimPercent() : 0);
        break;
    }
}

// Sees every touch and key press in the application.
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    const QEvent::Type type = event->type();

    const bool begins = (type == QEvent::MouseButtonPress || type == QEvent::TouchBegin || type == QEvent::KeyPress);
    const bool otherInput = (type == QEvent::MouseButtonRelease || type == QEvent::MouseMove
                             || type == QEvent::TouchUpdate || type == QEvent::TouchEnd
                             || type == QEvent::KeyRelease || type == QEvent::Wheel);

    if (begins || otherInput)
    {
        if (powerState == PowerState::ScreenSaver)
        {
            // Only a touch or key press wakes the screen. It is not passed on, so it cannot press a button.
            if (begins)
            {
                swallowUntilRelease = (type != QEvent::KeyPress);
                userActivity();
            }
            return true;
        }

        if (swallowUntilRelease)
        {
            // The rest of the touch that woke the screen
            if (type == QEvent::MouseButtonRelease || type == QEvent::TouchEnd)
                swallowUntilRelease = false;
            return true;
        }

        userActivity();
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_pushButton_battset_home_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}


void MainWindow::on_pushButton_battset_back_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}


// ---- Charger settings and PID pages ----------------------------------------
// Menu -> Charger Settings -> PID Settings. The SAVE buttons on both pages are not connected yet.

void MainWindow::on_pushButtonCharger_settings_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageCharger_settings);
}

void MainWindow::on_pushButton_chgset_home_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}

void MainWindow::on_pushButton_chgset_back_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageMenu);
}

void MainWindow::on_pushButton_chgset_pid_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pagePID_settings);
}

void MainWindow::on_pushButton_pid_home_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageDashboard);
}

void MainWindow::on_pushButton_pid_back_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageCharger_settings);
}


// ---- Battery settings page -------------------------------------------------
//
// Five configs are read when the page opens: battery (14), float (6), boost (8),
// equalize (10) and initialize (12). Save sends back only the blocks that were edited.
// The SP515 holds the limits and checks every value; if it does not accept one, the
// daemon answers with an error and a red message appears next to that block.

namespace {

// Byte layout of each reply, as in the daemon's packed sUI_..Config_t structs.
struct BatteryBlockLayout
{
    quint16 getCommand;
    quint16 setCommand;
    int size;               // payload size in bytes
    int cellVOffset;        // where the V/Cell float sits (-1: the battery block has no V/Cell)
};

const BatteryBlockLayout batteryLayout[] =
{
    { Rs485Client::GetBatteryConfig,    Rs485Client::SetBatteryConfig,     7, -1 },  // U8 type, U8 cells, U8 strings, float capacity
    { Rs485Client::GetFloatConfig,      Rs485Client::SetFloatConfig,       8,  0 },  // float cellV, float currentLimit
    { Rs485Client::GetBoostConfig,      Rs485Client::SetBoostConfig,      26, 14 },  // 2 flags, 3 x U32, float cellV, 2 floats
    { Rs485Client::GetEqualizeConfig,   Rs485Client::SetEqualizeConfig,   14,  6 },  // 2 flags, U32, float cellV, float
    { Rs485Client::GetInitializeConfig, Rs485Client::SetInitializeConfig, 13,  5 },  // flag, U32, float cellV, float
};

// A config SET can take the daemon up to 5 s, so wait a little longer than that.
const int batterySetTimeoutMs = 7000;

const char *const batteryUnsentNote = "Recharge rate, efficiency and compensation are not sent to the SP515 yet.";

// The battery type list on the page has six entries; the SP515 stores only the chemistry.
// Index in comboBox_battery_type -> eBattType_t (1 lead acid, 2 NiCad, 3 lithium).
const quint8 batteryTypeOfComboIndex[] = { 1, 1, 2, 2, 2, 3 };
const int batteryComboCount = int(sizeof(batteryTypeOfComboIndex) / sizeof(batteryTypeOfComboIndex[0]));

float floatAt(const QByteArray &bytes, int offset)
{
    quint32 raw;
    std::memcpy(&raw, bytes.constData() + offset, sizeof(raw));
    raw = qFromLittleEndian(raw);
    float value;
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}

void putFloat(QByteArray &bytes, int offset, float value)
{
    quint32 raw;
    std::memcpy(&raw, &value, sizeof(raw));
    raw = qToLittleEndian(raw);
    std::memcpy(bytes.data() + offset, &raw, sizeof(raw));
}

} // namespace

QLabel *MainWindow::batteryErrorLabel(int block) const
{
    switch (block)
    {
    case BlockBattery:    return ui->label_battery_error;
    case BlockFloat:      return ui->label_float_error;
    case BlockBoost:      return ui->label_boost_error;
    case BlockEqualize:   return ui->label_equalize_error;
    case BlockInitialize: return ui->label_initialize_error;
    }
    return nullptr;
}

QDoubleSpinBox *MainWindow::batteryCellVSpin(int block) const
{
    switch (block)
    {
    case BlockFloat:      return ui->doubleSpinBox_float_v_cell;
    case BlockBoost:      return ui->doubleSpinBox_boost_v_cell;
    case BlockEqualize:   return ui->doubleSpinBox_equalize_v_cell;
    case BlockInitialize: return ui->doubleSpinBox_initialize_v_cell;
    }
    return nullptr;
}

void MainWindow::setupBatteryPage()
{
    for (int block = 0; block < BlockCount; ++block)
    {
        batteryErrorLabel(block)->setStyleSheet("color: #e53935;");
        batteryErrorLabel(block)->clear();
    }
    ui->label_battset_status->clear();

    // The SP515 stores these as single bytes
    ui->spinBox_cells->setRange(0, 255);
    ui->spinBox_strings->setRange(0, 255);

    // Temperature compensation is normally negative (mV per degree C per cell)
    ui->doubleSpinBox_compensation->setRange(-999.99, 999.99);

    // Cell voltages need three decimals, or a value such as 2.275 V would be shown (and saved) as 2.28 V
    for (int block : {BlockFloat, BlockBoost, BlockEqualize, BlockInitialize})
    {
        QDoubleSpinBox *spin = batteryCellVSpin(block);
        spin->setDecimals(3);
        spin->setSingleStep(0.005);
        spin->setRange(0.0, 99.999);
    }

    // Numeric keypad: hidden until a value is tapped
    batteryKeypad = new NumericKeypad(ui->pageBattery_settings);
    batteryKeypad->setKeySize(84, 72);
    for (QSpinBox *spin : {ui->spinBox_capacity, ui->spinBox_cells, ui->spinBox_strings})
        batteryKeypad->registerEdit(spin);
    for (QDoubleSpinBox *spin : {ui->doubleSpinBox_recharge_rate, ui->doubleSpinBox_recharge_eff,
                                 ui->doubleSpinBox_compensation, ui->doubleSpinBox_float_v_cell,
                                 ui->doubleSpinBox_boost_v_cell, ui->doubleSpinBox_equalize_v_cell,
                                 ui->doubleSpinBox_initialize_v_cell})
    {
        spin->setLocale(QLocale::c());      // always use "." as the decimal point
        batteryKeypad->registerEdit(spin);
    }

    // Recharge rate, recharge efficiency and temperature compensation can be edited, but are not sent
    // to the SP515 yet: no command carries the profile values, and the daemon does not support the
    // compensation commands (16 and 17) yet. Say so when one is edited.
    for (QDoubleSpinBox *spin : {ui->doubleSpinBox_recharge_rate, ui->doubleSpinBox_recharge_eff,
                                 ui->doubleSpinBox_compensation})
        connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double)
                {
                    batteryUnsentEdited = true;
                    setBatteryStatus(batteryUnsentNote, 3);
                });

    // Remember which block the user edited, so Save sends only those
    connect(ui->comboBox_battery_type, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { batteryEdited(BlockBattery); });
    connect(ui->spinBox_capacity, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) { batteryEdited(BlockBattery); });
    connect(ui->spinBox_cells,    qOverload<int>(&QSpinBox::valueChanged), this, [this](int) { batteryEdited(BlockBattery); });
    connect(ui->spinBox_strings,  qOverload<int>(&QSpinBox::valueChanged), this, [this](int) { batteryEdited(BlockBattery); });
    for (int block : {BlockFloat, BlockBoost, BlockEqualize, BlockInitialize})
        connect(batteryCellVSpin(block), qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                [this, block](double) { batteryEdited(block); });

    // Read the values every time the page is opened
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int)
            {
                if (ui->stackedWidget->currentWidget() == ui->pageBattery_settings)
                    loadBatterySettings();
            });
    connect(rs485, &Rs485Client::replyReceived, this, &MainWindow::handleBatteryReply);

    for (int block = 0; block < BlockCount; ++block)
        setBatteryBlockEnabled(block, false);       // until the values have been read
}

void MainWindow::setBatteryBlockEnabled(int block, bool enabled)
{
    if (block == BlockBattery)
    {
        for (QWidget *w : {static_cast<QWidget *>(ui->comboBox_battery_type), static_cast<QWidget *>(ui->spinBox_capacity),
                           static_cast<QWidget *>(ui->spinBox_cells), static_cast<QWidget *>(ui->spinBox_strings)})
            w->setEnabled(enabled);
    }
    else
    {
        batteryCellVSpin(block)->setEnabled(enabled);
    }
}

void MainWindow::setBatteryStatus(const QString &text, int kind)
{
    // kind: 0 neutral, 1 good (green), 2 error (red), 3 warning (orange)
    const char *color = kind == 1 ? "#43a047" : (kind == 2 ? "#e53935" : (kind == 3 ? "#fb8c00" : ""));
    ui->label_battset_status->setStyleSheet(*color ? QString("color: %1;").arg(color) : QString());
    ui->label_battset_status->setText(text);
}

void MainWindow::updateBatteryButtons()
{
    ui->pushButton_battset_save->setEnabled(batteryPendingGets == 0 && batteryPendingSets == 0);
}

// Text for the red label next to a block. Status 3 means the daemon says the SP515 did not accept the
// value or did not answer; the daemon does not tell those two apart.
QString MainWindow::batteryErrorText(const QString &message, bool reading) const
{
    const quint8 status = rs485->lastErrorStatus();
    if (status == 3)
        return reading ? "No answer from the SP515" : "Not accepted by the SP515";
    if (status == 0)
        return "No answer from the daemon";         // not running, or too slow
    return message;                                 // the daemon's own text (busy, not supported, ...)
}

void MainWindow::loadBatterySettings()
{
    if (batteryPendingGets > 0 || batteryPendingSets > 0)
        return;                                     // a read or save is already running

    batteryGetFailed = false;
    batteryUnsentEdited = false;
    setBatteryStatus("Reading from the SP515...", 0);
    for (int block = 0; block < BlockCount; ++block)
    {
        batteryBlocks[block] = BatteryBlockState();
        batteryErrorLabel(block)->clear();
        setBatteryBlockEnabled(block, false);
        ++batteryPendingGets;
        rs485->sendCommand(batteryLayout[block].getCommand);
    }
    updateBatteryButtons();
}

void MainWindow::handleBatteryReply(quint16 command, const QByteArray &payload)
{
    bool handled = false;                           // every reply from the daemon comes through here, not only ours
    for (int block = 0; block < BlockCount; ++block)
    {
        const BatteryBlockLayout &layout = batteryLayout[block];
        if (command != layout.getCommand || batteryPendingGets == 0)
            continue;

        handled = true;
        --batteryPendingGets;
        if (payload.size() != layout.size)
        {
            batteryGetFailed = true;
            batteryErrorLabel(block)->setText(QString("Unexpected reply length: %1 bytes").arg(payload.size()));
        }
        else
        {
            batteryBlocks[block].raw = payload;
            batteryBlocks[block].loaded = true;

            if (block == BlockBattery)
            {
                const quint8 type = quint8(payload.at(0));
                int index = ui->comboBox_battery_type->currentIndex();
                // The SP515 gives the chemistry only. Keep the chosen entry if it already has that
                // chemistry, otherwise pick the first entry of that chemistry (none for "undefined").
                if (index < 0 || index >= batteryComboCount || batteryTypeOfComboIndex[index] != type)
                {
                    index = -1;
                    for (int i = 0; i < batteryComboCount; ++i)
                        if (batteryTypeOfComboIndex[i] == type) { index = i; break; }
                }
                const QSignalBlocker b1(ui->comboBox_battery_type), b2(ui->spinBox_capacity),
                                     b3(ui->spinBox_cells), b4(ui->spinBox_strings);
                ui->comboBox_battery_type->setCurrentIndex(index);
                ui->spinBox_cells->setValue(quint8(payload.at(1)));
                ui->spinBox_strings->setValue(quint8(payload.at(2)));
                ui->spinBox_capacity->setValue(qRound(floatAt(payload, 3)));
            }
            else
            {
                const QSignalBlocker blocker(batteryCellVSpin(block));
                batteryCellVSpin(block)->setValue(double(floatAt(payload, layout.cellVOffset)));
            }
            setBatteryBlockEnabled(block, true);
        }
        break;
    }

    if (handled && batteryPendingGets == 0)
    {
        if (batteryGetFailed)
            setBatteryStatus("Could not read all the values.", 2);
        else
            setBatteryStatus("Values read from the SP515.", 0);
        updateBatteryButtons();
    }
}

void MainWindow::batteryEdited(int block)
{
    batteryBlocks[block].dirty = true;
    batteryErrorLabel(block)->clear();
    if (batteryPendingSets == 0)
        setBatteryStatus(QString(), 0);
}

// Result of a battery settings read (failures only) or save. Returns true when the command was one of ours.
bool MainWindow::handleBatteryCommandFinished(quint16 command, bool ok, const QString &message)
{
    for (int block = 0; block < BlockCount; ++block)
    {
        const BatteryBlockLayout &layout = batteryLayout[block];

        if (command == layout.getCommand)
        {
            if (ok || batteryPendingGets == 0)
                return true;                        // success is handled in handleBatteryReply
            --batteryPendingGets;
            batteryGetFailed = true;
            batteryErrorLabel(block)->setText(batteryErrorText(message, true));
            if (batteryPendingGets == 0)
            {
                setBatteryStatus("Could not read all the values.", 2);
                updateBatteryButtons();
            }
            return true;
        }

        if (command == layout.setCommand)
        {
            if (batteryPendingSets == 0)
                return true;
            --batteryPendingSets;
            if (ok)
            {
                batteryBlocks[block].raw = batteryBlocks[block].sent;
                batteryBlocks[block].dirty = false;
                batteryErrorLabel(block)->clear();
            }
            else
            {
                batterySetFailed = true;
                batteryErrorLabel(block)->setText(batteryErrorText(message, false));
            }
            if (batteryPendingSets == 0)
            {
                if (batterySetFailed)
                    setBatteryStatus("Some values were not saved.", 2);
                else if (batteryUnsentEdited)
                    setBatteryStatus(QString("Saved. ") + batteryUnsentNote, 3);
                else
                    setBatteryStatus("Saved.", 1);
                updateBatteryButtons();
            }
            return true;
        }
    }
    return false;
}

void MainWindow::on_pushButton_battset_save_clicked()
{
    if (batteryPendingGets > 0 || batteryPendingSets > 0)
        return;

    // Build the messages first, so nothing is sent if one of them cannot be made
    QByteArray payloads[BlockCount];
    bool send[BlockCount] = {};
    for (int block = 0; block < BlockCount; ++block)
    {
        if (!batteryBlocks[block].dirty || !batteryBlocks[block].loaded)
            continue;

        QByteArray payload = batteryBlocks[block].raw;
        if (block == BlockBattery)
        {
            const int index = ui->comboBox_battery_type->currentIndex();
            if (index < 0 || index >= batteryComboCount)
            {
                batteryErrorLabel(BlockBattery)->setText("Choose a battery type");
                setBatteryStatus("Nothing was saved.", 2);
                return;
            }
            payload[0] = char(batteryTypeOfComboIndex[index]);
            payload[1] = char(ui->spinBox_cells->value());
            payload[2] = char(ui->spinBox_strings->value());
            putFloat(payload, 3, float(ui->spinBox_capacity->value()));
        }
        else
        {
            // Keep everything else in the config as the SP515 reported it; only the V/Cell changes
            putFloat(payload, batteryLayout[block].cellVOffset, float(batteryCellVSpin(block)->value()));
        }
        payloads[block] = payload;
        send[block] = true;
    }

    batterySetFailed = false;
    for (int block = 0; block < BlockCount; ++block)
    {
        if (!send[block])
            continue;
        batteryErrorLabel(block)->clear();
        batteryBlocks[block].sent = payloads[block];
        ++batteryPendingSets;
        rs485->sendCommand(batteryLayout[block].setCommand, payloads[block], batterySetTimeoutMs);
    }

    if (batteryPendingSets == 0)
    {
        if (batteryUnsentEdited)
            setBatteryStatus(batteryUnsentNote, 3);
        else
            setBatteryStatus("No changes to save.", 0);
        return;
    }
    setBatteryStatus("Saving...", 0);
    updateBatteryButtons();
}


// ---- PID settings page -----------------------------------------------------
//
// The four gains (voltage Kp, voltage Ki, current Kp, current Ki) are read from the SP515 when the
// page opens (command 25) and saved with SAVE (command 26). The SP515 checks the values. While the
// page is open the rectifier voltage and current are requested every 200 ms, so the effect of a
// gain change on the output can be watched.

namespace {
const int pidFastPollMs = 200;
const int pidGainCount = 4;
const int pidPayloadSize = pidGainCount * 4;        // four little-endian floats, as in sPidConfig_t
}

QDoubleSpinBox *MainWindow::pidSpin(int index) const
{
    switch (index)
    {
    case 0: return ui->doubleSpinBox_voltagekp;
    case 1: return ui->doubleSpinBox_voltageki;
    case 2: return ui->doubleSpinBox_currentkp;
    case 3: return ui->doubleSpinBox_currentki;
    }
    return nullptr;
}

void MainWindow::setupPidPage()
{
    ui->label_pid_status->clear();
    ui->label_rectifier_voltage_value->setText("--");
    ui->label_rectifier_current_value->setText("--");

    // Four decimals: with the default two, a gain such as 0.005 would be shown as 0.01
    pidKeypad = new NumericKeypad(ui->pagePID_settings);
    pidKeypad->setKeySize(84, 72);
    for (int i = 0; i < pidGainCount; ++i)
    {
        QDoubleSpinBox *spin = pidSpin(i);
        spin->setDecimals(4);
        spin->setRange(0.0, 9999.9999);
        spin->setLocale(QLocale::c());              // always use "." as the decimal point
        spin->setEnabled(false);                    // until the values have been read
        pidKeypad->registerEdit(spin);
        connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, i](double)
                {
                    pidDirty[i] = true;
                    if (!pidSetPending)
                        setPidStatus(QString(), 0);
                });
    }

    pidPollTimer = new QTimer(this);
    connect(pidPollTimer, &QTimer::timeout, this, [this]() { rs485->requestAnalogueInputs(); });

    // Read the gains and start the fast readout when the page opens; stop it when it closes
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int)
            {
                if (ui->stackedWidget->currentWidget() == ui->pagePID_settings)
                {
                    pidPollTimer->start(pidFastPollMs);
                    rs485->requestAnalogueInputs();
                    loadPidSettings();
                }
                else
                {
                    pidPollTimer->stop();
                }
            });
    connect(rs485, &Rs485Client::replyReceived, this, &MainWindow::handlePidReply);
}

void MainWindow::setPidStatus(const QString &text, int kind)
{
    const char *color = kind == 1 ? "#43a047" : (kind == 2 ? "#e53935" : "");
    ui->label_pid_status->setStyleSheet(*color ? QString("color: %1;").arg(color) : QString());
    ui->label_pid_status->setText(text);
}

void MainWindow::loadPidSettings()
{
    if (pidGetPending || pidSetPending)
        return;

    pidLoaded = false;
    pidRaw.clear();
    for (int i = 0; i < pidGainCount; ++i)
    {
        pidDirty[i] = false;
        pidSpin(i)->setEnabled(false);
    }
    setPidStatus("Reading from the SP515...", 0);
    pidGetPending = true;
    rs485->sendCommand(Rs485Client::GetPidConfig);
}

void MainWindow::handlePidReply(quint16 command, const QByteArray &payload)
{
    if (command != Rs485Client::GetPidConfig || !pidGetPending)
        return;

    pidGetPending = false;
    if (payload.size() != pidPayloadSize)
    {
        setPidStatus(QString("Unexpected reply length: %1 bytes").arg(payload.size()), 2);
        return;
    }

    pidRaw = payload;
    pidLoaded = true;
    for (int i = 0; i < pidGainCount; ++i)
    {
        const QSignalBlocker blocker(pidSpin(i));
        pidSpin(i)->setValue(double(floatAt(payload, i * 4)));
        pidSpin(i)->setEnabled(true);
    }
    setPidStatus("Values read from the SP515.", 0);
}

// Result of a PID read (failures only) or save. Returns true when the command was one of ours.
bool MainWindow::handlePidCommandFinished(quint16 command, bool ok, const QString &message)
{
    if (command == Rs485Client::GetPidConfig)
    {
        if (!ok && pidGetPending)
        {
            pidGetPending = false;
            const quint8 status = rs485->lastErrorStatus();
            setPidStatus(status == 3 ? QString("No answer from the SP515")
                                     : (status == 0 ? QString("No answer from the daemon") : message), 2);
        }
        return true;
    }

    if (command == Rs485Client::SetPidConfig)
    {
        if (!pidSetPending)
            return true;
        pidSetPending = false;
        if (ok)
        {
            pidRaw = pidSent;
            for (bool &d : pidDirty)
                d = false;
            setPidStatus("Saved.", 1);
        }
        else
        {
            const quint8 status = rs485->lastErrorStatus();
            setPidStatus(status == 3 ? QString("Not accepted by the SP515")
                                     : (status == 0 ? QString("No answer from the daemon") : message), 2);
        }
        ui->pushButton_pid_save->setEnabled(true);
        return true;
    }
    return false;
}

void MainWindow::on_pushButton_pid_save_clicked()
{
    if (pidGetPending || pidSetPending)
        return;
    if (!pidLoaded)
    {
        setPidStatus("The values have not been read from the SP515.", 2);
        return;
    }

    // Start from the gains as the SP515 reported them and replace only the ones that were edited,
    // so a gain shown rounded to four decimals is not written back rounded.
    QByteArray payload = pidRaw;
    bool any = false;
    for (int i = 0; i < pidGainCount; ++i)
    {
        if (!pidDirty[i])
            continue;
        putFloat(payload, i * 4, float(pidSpin(i)->value()));
        any = true;
    }
    if (!any)
    {
        setPidStatus("No changes to save.", 0);
        return;
    }

    pidSent = payload;
    pidSetPending = true;
    ui->pushButton_pid_save->setEnabled(false);
    setPidStatus("Saving...", 0);
    rs485->sendCommand(Rs485Client::SetPidConfig, payload, 7000);      // the SP515 saves to flash
}

void MainWindow::on_pushButton_8_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageBoost_settings);
}


// ---- Boost, Equalize and Initialize pages ----------------------------------

namespace {

quint32 u32At(const QByteArray &bytes, int offset)
{
    quint32 raw;
    std::memcpy(&raw, bytes.constData() + offset, sizeof(raw));
    return qFromLittleEndian(raw);
}

void putU32(QByteArray &bytes, int offset, quint32 value)
{
    value = qToLittleEndian(value);
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}

const int hourSeconds = 3600;
const int daySeconds = 86400;
const int modeSetTimeoutMs = 7000;          // a config SET can take the daemon up to 5 s

// The battery config (command 14): U8 type, U8 cells per string, U8 strings, float capacity
const int batteryConfigSize = 7;

} // namespace

void MainWindow::setupModePages()
{
    ModePage &boost = modePages[ModeBoost];
    boost.page = ui->pageBoost_settings;
    boost.toggle = ui->checkBox_boost_enable;
    boost.volts = ui->doubleSpinBox_boost_volt_cell;
    boost.currLim = ui->doubleSpinBox_boost_curr_lim;
    boost.gassing = ui->doubleSpinBox_gassing_volt;
    boost.duration = ui->spinBox_boost_duration;
    boost.overrideTime = ui->spinBox_boost_overide_time;
    boost.repeat = ui->spinBox_boost_repeat;
    boost.status = ui->label_boostcfg_status;
    boost.save = ui->pushButton_boost_save;
    boost.getCommand = Rs485Client::GetBoostConfig;
    boost.setCommand = Rs485Client::SetBoostConfig;
    boost.size = 26;                // bool, bool, U32 cycle, U32 duration, U32 override, float V/cell, float current limit, float gassing
    boost.offset[FieldEnabled] = 0;   boost.offset[FieldVolts] = 14;  boost.offset[FieldCurrLim] = 18;
    boost.offset[FieldDuration] = 6;  boost.offset[FieldOverride] = 10; boost.offset[FieldRepeat] = 2;
    boost.offset[FieldGassing] = 22;

    ModePage &eq = modePages[ModeEqualize];
    eq.page = ui->pageEqualize_settings;
    eq.toggle = ui->checkBox_equalize_enable;
    eq.volts = ui->doubleSpinBox_equalize_volt_cell;
    eq.currLim = ui->doubleSpinBox_equalize_curr_lim;
    eq.duration = ui->spinBox_equalize_duration;
    eq.overrideTime = ui->spinBox_equalize_overide_time;
    eq.status = ui->label_equalizecfg_status;
    eq.save = ui->pushButton_equalize_save;
    eq.getCommand = Rs485Client::GetEqualizeConfig;
    eq.setCommand = Rs485Client::SetEqualizeConfig;
    eq.size = 14;                   // bool, bool, U32 duration, float V/cell, float current limit
    eq.offset[FieldEnabled] = 0;   eq.offset[FieldVolts] = 6;  eq.offset[FieldCurrLim] = 10;
    eq.offset[FieldDuration] = 2;  eq.offset[FieldOverride] = -1; eq.offset[FieldRepeat] = -1;
    eq.offset[FieldGassing] = -1;
    eq.unsentNote = "The override timer is not sent to the SP515 yet.";

    ModePage &init = modePages[ModeInitialize];
    init.page = ui->pageInitialize_settings;
    init.toggle = ui->checkBox_initialize_enable;
    init.volts = ui->doubleSpinBox_initialize_volt_cell;
    init.currLim = ui->doubleSpinBox_initialize_curr_lim;
    init.duration = ui->spinBox_initialize_duration;
    init.overrideTime = ui->spinBox_initialize_overide_time;
    init.status = ui->label_initializecfg_status;
    init.save = ui->pushButton_initialize_save;
    init.getCommand = Rs485Client::GetInitializeConfig;
    init.setCommand = Rs485Client::SetInitializeConfig;
    init.size = 13;                 // bool, U32 duration, float V/cell, float current limit
    init.offset[FieldEnabled] = 0;   init.offset[FieldVolts] = 5;  init.offset[FieldCurrLim] = 9;
    init.offset[FieldDuration] = 1;  init.offset[FieldOverride] = -1; init.offset[FieldRepeat] = -1;
    init.offset[FieldGassing] = -1;
    init.unsentNote = "The override timer is not sent to the SP515 yet.";

    for (int kind = 0; kind < ModeCount; ++kind)
    {
        ModePage &m = modePages[kind];
        m.status->clear();

        // Value ranges. Three decimals on the voltages: with two, 2.275 V would be shown (and saved) as 2.28 V
        for (QDoubleSpinBox *spin : {m.volts, m.gassing})
        {
            if (!spin)
                continue;
            spin->setDecimals(3);
            spin->setSingleStep(0.005);
            spin->setRange(0.0, 99.999);
        }
        m.currLim->setDecimals(1);
        m.currLim->setRange(0.0, 1000.0);
        m.currLim->setSuffix(" %");
        m.duration->setRange(0, 2147483647);
        m.overrideTime->setRange(0, 1000000);
        if (m.repeat)                                   // Boost only
            m.repeat->setRange(0, 49000);

        // Numeric keypad
        m.keypad = new NumericKeypad(m.page);
        m.keypad->setKeySize(84, 72);
        for (QSpinBox *spin : {m.duration, m.overrideTime, m.repeat})
            if (spin)
                m.keypad->registerEdit(spin);
        for (QDoubleSpinBox *spin : {m.volts, m.currLim, m.gassing})
        {
            if (!spin)
                continue;
            spin->setLocale(QLocale::c());              // always use "." as the decimal point
            m.keypad->registerEdit(spin);
        }

        // Remember what was edited
        connect(m.toggle, &QCheckBox::toggled, this, [this, kind](bool on)
                {
                    modePages[kind].toggle->setText(on ? "Enabled" : "Disabled");
                    if (!modePages[kind].loading)
                        modeFieldEdited(kind, FieldEnabled);
                });
        connect(m.volts, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, kind](double) { modeFieldEdited(kind, FieldVolts); });
        connect(m.currLim, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, kind](double) { modeFieldEdited(kind, FieldCurrLim); });
        connect(m.duration, qOverload<int>(&QSpinBox::valueChanged), this, [this, kind](int) { modeFieldEdited(kind, FieldDuration); });
        connect(m.overrideTime, qOverload<int>(&QSpinBox::valueChanged), this, [this, kind](int) { modeFieldEdited(kind, FieldOverride); });
        if (m.repeat)
            connect(m.repeat, qOverload<int>(&QSpinBox::valueChanged), this, [this, kind](int) { modeFieldEdited(kind, FieldRepeat); });
        if (m.gassing)
            connect(m.gassing, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, kind](double) { modeFieldEdited(kind, FieldGassing); });

        connect(m.save, &QPushButton::clicked, this, [this, kind]() { saveModePage(kind); });
        enableModeFields(kind);                         // all off until the config has been read
    }

    // Home and Back buttons of the three pages. Back goes to Battery Settings, where the three pages are opened from.
    connect(ui->pushButton_boost_home, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageDashboard); });
    connect(ui->pushButton_boost_back, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageBattery_settings); });
    connect(ui->pushButton_equalize_home, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageDashboard); });
    connect(ui->pushButton_equalize_back, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageBattery_settings); });
    connect(ui->pushButton_initialize_home, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageDashboard); });
    connect(ui->pushButton_initialize_back, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageBattery_settings); });

    // The buttons on Battery Settings (the Boost one is on_pushButton_8_clicked)
    connect(ui->pushButton_9, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageEqualize_settings); });
    connect(ui->pushButton_10, &QPushButton::clicked, this, [this]() { ui->stackedWidget->setCurrentWidget(ui->pageInitialize_settings); });

    // Read the config every time one of the pages opens
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int)
            {
                for (int kind = 0; kind < ModeCount; ++kind)
                    if (ui->stackedWidget->currentWidget() == modePages[kind].page)
                        loadModePage(kind);
            });
    connect(rs485, &Rs485Client::replyReceived, this, &MainWindow::handleModeReply);
}

// Fields are enabled once the config has been read; the current limit also needs the battery config
void MainWindow::enableModeFields(int kind)
{
    ModePage &m = modePages[kind];
    const bool on = m.configLoaded;
    m.toggle->setEnabled(on);
    m.volts->setEnabled(on);
    m.duration->setEnabled(on);
    m.overrideTime->setEnabled(on);
    if (m.repeat)
        m.repeat->setEnabled(on);
    if (m.gassing)
        m.gassing->setEnabled(on);
    m.currLim->setEnabled(on && m.batteryLoaded && m.capacityStrings > 0.0);
}

void MainWindow::setModeStatus(int kind, const QString &text, int level)
{
    const char *color = level == 1 ? "#43a047" : (level == 2 ? "#e53935" : (level == 3 ? "#fb8c00" : ""));
    ModePage &m = modePages[kind];
    m.status->setStyleSheet(*color ? QString("color: %1;").arg(color) : QString());
    m.status->setText(text);
}

void MainWindow::loadModePage(int kind)
{
    ModePage &m = modePages[kind];
    if (m.awaitConfig || m.awaitBattery || m.setPending)
        return;

    m.configLoaded = false;
    m.batteryLoaded = false;
    m.capacityStrings = 0;
    m.raw.clear();
    m.unsentEdited = false;
    for (bool &d : m.dirty)
        d = false;
    enableModeFields(kind);
    setModeStatus(kind, "Reading from the SP515...", 0);
    m.awaitConfig = m.awaitBattery = true;
    rs485->sendCommand(m.getCommand);
    rs485->sendCommand(Rs485Client::GetBatteryConfig);
}

void MainWindow::handleModeReply(quint16 command, const QByteArray &payload)
{
    for (int kind = 0; kind < ModeCount; ++kind)
    {
        ModePage &m = modePages[kind];

        if (command == Rs485Client::GetBatteryConfig && m.awaitBattery)
        {
            m.awaitBattery = false;
            if (payload.size() == batteryConfigSize)
            {
                const double strings = quint8(payload.at(2));
                m.capacityStrings = double(floatAt(payload, 3)) * strings;
                m.batteryLoaded = true;
            }
        }
        else if (command == m.getCommand && m.awaitConfig)
        {
            m.awaitConfig = false;
            if (payload.size() != m.size)
            {
                setModeStatus(kind, QString("Unexpected reply length: %1 bytes").arg(payload.size()), 2);
                continue;
            }
            m.raw = payload;
            m.configLoaded = true;
        }
        else
            continue;

        if (m.awaitConfig || m.awaitBattery)
            continue;                               // wait for the other read

        // Both reads are back: fill in the page
        if (m.configLoaded)
        {
            const QSignalBlocker b2(m.volts), b3(m.currLim), b4(m.duration), b5(m.overrideTime);
            const bool on = m.raw.at(m.offset[FieldEnabled]) != 0;
            m.loading = true;                       // filling in values, not an edit
            m.toggle->setChecked(on);
            m.loading = false;
            m.toggle->setText(on ? "Enabled" : "Disabled");
            m.volts->setValue(double(floatAt(m.raw, m.offset[FieldVolts])));
            m.duration->setValue(int(qMin<quint32>(u32At(m.raw, m.offset[FieldDuration]), 2147483647u)));
            if (m.offset[FieldOverride] >= 0)
                m.overrideTime->setValue(qRound(double(u32At(m.raw, m.offset[FieldOverride])) / hourSeconds));
            if (m.repeat && m.offset[FieldRepeat] >= 0)
            {
                const QSignalBlocker b6(m.repeat);
                m.repeat->setValue(qRound(double(u32At(m.raw, m.offset[FieldRepeat])) / daySeconds));
            }
            if (m.gassing)
            {
                const QSignalBlocker b7(m.gassing);
                m.gassing->setValue(double(floatAt(m.raw, m.offset[FieldGassing])));
            }
            if (m.batteryLoaded && m.capacityStrings > 0.0)
                m.currLim->setValue(double(floatAt(m.raw, m.offset[FieldCurrLim])) / (m.capacityStrings / 100.0));
            else
                m.currLim->setValue(0.0);
        }
        enableModeFields(kind);

        if (!m.configLoaded)
            continue;                               // the failure message is already shown
        if (!m.batteryLoaded || m.capacityStrings <= 0.0)
            setModeStatus(kind, "The current limit in % needs the battery capacity and strings from the SP515.", 3);
        else
            setModeStatus(kind, "Values read from the SP515.", 0);
    }
}

// Result of a read (failures only) or a save of one of the three pages. Returns true when the command was one of ours.
bool MainWindow::handleModeCommandFinished(quint16 command, bool ok, const QString &message)
{
    bool handled = false;
    for (int kind = 0; kind < ModeCount; ++kind)
    {
        ModePage &m = modePages[kind];
        const quint8 status = rs485->lastErrorStatus();
        const QString reason = status == 3 ? (command == m.setCommand ? QString("Not accepted by the SP515") : QString("No answer from the SP515"))
                             : (status == 0 ? QString("No answer from the daemon") : message);

        if (command == m.getCommand && m.awaitConfig)
        {
            handled = true;
            if (!ok)
            {
                m.awaitConfig = false;
                setModeStatus(kind, reason, 2);
                enableModeFields(kind);
            }
        }
        else if (command == Rs485Client::GetBatteryConfig && m.awaitBattery)
        {
            handled = true;
            if (!ok)
            {
                m.awaitBattery = false;
                m.batteryLoaded = false;
                if (!m.awaitConfig && m.configLoaded)        // the config arrived first: finish now
                {
                    enableModeFields(kind);
                    setModeStatus(kind, "The current limit in % needs the battery capacity and strings from the SP515.", 3);
                }
            }
        }
        else if (command == m.setCommand && m.setPending)
        {
            handled = true;
            m.setPending = false;
            if (ok)
            {
                m.raw = m.sent;
                for (bool &d : m.dirty)
                    d = false;
                if (m.unsentEdited)
                    setModeStatus(kind, QString("Saved. ") + m.unsentNote, 3);
                else
                    setModeStatus(kind, "Saved.", 1);
            }
            else
                setModeStatus(kind, reason, 2);
            m.save->setEnabled(true);
        }
    }
    return handled;
}

void MainWindow::modeFieldEdited(int kind, int field)
{
    ModePage &m = modePages[kind];
    if (m.offset[field] < 0)
    {
        m.unsentEdited = true;
        setModeStatus(kind, m.unsentNote, 3);
        return;
    }
    m.dirty[field] = true;
    if (!m.setPending)
        setModeStatus(kind, m.unsentEdited ? m.unsentNote : QString(), m.unsentEdited ? 3 : 0);
}

void MainWindow::saveModePage(int kind)
{
    ModePage &m = modePages[kind];
    if (m.awaitConfig || m.awaitBattery || m.setPending)
        return;
    if (!m.configLoaded)
    {
        setModeStatus(kind, "The values have not been read from the SP515.", 2);
        return;
    }

    // Start from the config as the SP515 reported it and replace only the fields that were edited, so
    // everything else (and any value shown rounded) is written back exactly as it was.
    QByteArray payload = m.raw;
    bool any = false;
    for (int field = 0; field < FieldCount; ++field)
    {
        if (!m.dirty[field] || m.offset[field] < 0)
            continue;
        any = true;
        const int at = m.offset[field];
        switch (field)
        {
        case FieldEnabled:  payload[at] = m.toggle->isChecked() ? 1 : 0; break;
        case FieldVolts:    putFloat(payload, at, float(m.volts->value())); break;
        case FieldGassing:  putFloat(payload, at, float(m.gassing->value())); break;
        case FieldCurrLim:  putFloat(payload, at, float(m.currLim->value() * (m.capacityStrings / 100.0))); break;
        case FieldDuration: putU32(payload, at, quint32(m.duration->value())); break;
        case FieldOverride: putU32(payload, at, quint32(qint64(m.overrideTime->value()) * hourSeconds)); break;
        case FieldRepeat:   putU32(payload, at, quint32(qint64(m.repeat->value()) * daySeconds)); break;
        }
    }
    if (!any)
    {
        if (m.unsentEdited)
            setModeStatus(kind, m.unsentNote, 3);
        else
            setModeStatus(kind, "No changes to save.", 0);
        return;
    }

    m.sent = payload;
    m.setPending = true;
    m.save->setEnabled(false);
    setModeStatus(kind, "Saving...", 0);
    rs485->sendCommand(m.setCommand, payload, modeSetTimeoutMs);
}
