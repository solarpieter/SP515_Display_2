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
#include <cmath>

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
    ui->btnChargerEnable->setChecked(false);            // the SP515 starts with the charger disabled
    ui->btnChargerEnable->setText("CHARGER OFF");
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
                                      ui->label_load_volt_value_2,
                                      ui->label_load_current_value_2})
                {
                    label->setText("--");
                }
                showAlarmState(false, false);
            });

    connect(rs485, &Rs485Client::alarmsReceived, this, [this](const Alarms &a)
            {
                showAlarmState(true, a.any());
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
                if (command == Rs485Client::SetChargeSetpoint)
                {
                    // The SP515's answer to the ON / OFF request
                    chargerCommandPending = false;
                    ui->btnChargerEnable->setEnabled(true);
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

    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, [this]()
            {
                rs485->requestAnalogueInputs();
                if (++pollCount % 2 == 0)
                    rs485->requestAlarms();         // alarms every second
            });
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
