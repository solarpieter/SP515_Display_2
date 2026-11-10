#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QButtonGroup>
#include <QCheckBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QPropertyAnimation>
#include <QMainWindow>
#include <QTimer>
#include "backlight.h"
#include "netcontrol.h"
#include "numerickeypad.h"
#include "screensaver.h"
#include "textkeyboard.h"
#include "rs485client.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;     // sees every touch / key, for the idle timers

private slots:
    void on_pushButton_home_clicked();

    void on_pushButton_menu_clicked();

    void on_pushButtonDate_settings_clicked();

    void on_pushButtonDateBack_clicked();

    void on_pushButtonDateSave_clicked();

    void on_pushButtonMenuHome_clicked();

    void on_pushButtonMenuExit_clicked();

    void on_pushButtonDisplaySave_clicked();

    void on_pushButtonDisplayBack_clicked();

    void on_radioButtonDark_clicked();

    void on_radioButtonLight_clicked();

    void on_radioButtonHighcontrast_clicked();

    void on_pushButtonDisplay_settings_clicked();

    void on_btnChargerEnable_clicked(bool checked);

    void on_pushButton_Enable_home_clicked();

    void on_pushButtonChargerenabledisable_clicked();

    void on_pushButtonBattery_settings_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_Calibration_clicked();

    void on_btn_calibrate_clicked();

    void on_btn_apply_offset_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_menu_2_clicked();

    void on_pushButton_battset_home_clicked();

    void on_pushButton_battset_back_clicked();

    void on_pushButton_battset_save_clicked();

    void on_pushButtonCharger_settings_clicked();

    void on_pushButton_chgset_home_clicked();

    void on_pushButton_chgset_back_clicked();

    void on_pushButton_chgset_pid_clicked();

    void on_pushButton_pid_home_clicked();

    void on_pushButton_pid_back_clicked();

    void on_pushButton_pid_save_clicked();

    void on_pushButton_8_clicked();

private:
    Ui::MainWindow *ui;
    bool darkMode;
    QTimer *clockTimer;
    NumericKeypad *keypad;
    NumericKeypad *dateTimeKeypad;
    // Calibration page
    Measurements lastMeasurements;              // latest values from the daemon
    bool haveMeasurements = false;
    int calibrationChannel = -1;                // channel being calibrated, -1 when idle
    double calibrationMeasured = 0.0;           // value typed in by the user
    double calibrationReading = 0.0;            // what the SP515 was showing at that moment
    void updateCurrentValueLabel();             // "Current Value" label follows the combo box
    void setCalibrationBusy(bool busy);

    // Progress bar and OK / FAIL labels on the calibration page
    enum class CalibrationOp { None, Calibrate, ApplyOffset };
    CalibrationOp calibrationOp = CalibrationOp::None;
    QProgressBar *calibrationProgress = nullptr;
    QLabel *calibrateResultLabel = nullptr;
    QLabel *offsetResultLabel = nullptr;
    QPropertyAnimation *calibrationAnimation = nullptr;
    bool calibrationBarFull = true;                 // the bar has run to 100 %
    bool calibrationResultKnown = false;            // the SP515 has answered (or the request was refused)
    bool calibrationResultOk = false;
    QString calibrationFailMessage;                 // popup text, shown together with FAIL
    void startCalibrationRun(CalibrationOp op);     // buttons off, progress bar runs for 3 s
    void reportCalibrationResult(bool ok, const QString &failMessage = QString());   // remember the result
    void calibrationBarFinished();                  // bar is full: show the result if it is known
    void showCalibrationResult();                   // green OK or red FAIL (+ popup), buttons on again
    void showCalibrationMessage(const QString &text);
    bool selectedReading(double &value) const;
    // Enable / Disable page shows the same values as the dashboard
    // Display page: theme radio buttons, back light, dimming and screen saver
    enum class PowerState { Normal, Dimmed, ScreenSaver };
    QButtonGroup *themeGroup = nullptr;
    QButtonGroup *keyboardGroup = nullptr;
    NumericKeypad *displayKeypad = nullptr;
    Backlight *backlight = nullptr;
    ScreenSaver *screenSaver = nullptr;
    QTimer *idleTimer = nullptr;
    QElapsedTimer idleClock;                        // time since the last touch or key
    PowerState powerState = PowerState::Normal;
    int backlightPercent = 100;                     // the level chosen with the slider
    int dimAfterS = 60;                             // 0 = never dim
    int screensaverAfterS = 300;                    // 0 = no screen saver
    bool alarmActive = false;
    bool swallowUntilRelease = false;               // the touch that woke the screen is not a button press
    void setupDisplayPage();
    void saveDisplaySettings();
    void checkIdle();
    void userActivity();
    void setPowerState(PowerState state);
    void applyPowerState();
    int dimPercent() const;
    QElapsedTimer chargerClickTimer;                // de-bounce for the charger button
    int pollCount = 0;
    void showAlarmState(bool haveData, bool active);
    void setChargerState(bool on);                  // switch the charger button (after the SP515 confirmed)
    void showChargerFromSp515(bool enabled);        // the SP515 reported the charger state: the switch follows it
    void setChargerUnknown();                       // no state from the SP515 (no link): the switch is greyed out
    bool chargerStateKnown = false;
    int statusFailures = 0;                         // status requests in a row that got no answer
    void requestChargerState(bool on);              // ask the SP515 to switch the charger OFF (after the user said Yes)
    bool chargerCommandPending = false;
    bool chargerPendingOn = false;
    // Switching ON: the voltage and current limit are worked out from the SP515's own float and battery
    // settings, read at that moment: voltage = float V/Cell x cells per string, current limit = the
    // float current limit (the recharge rate). The operator sees both numbers before saying Yes.
    void beginChargerOn();
    void askChargerOn();
    void handleChargerOnReply(quint16 command, const QByteArray &payload);
    bool handleChargerOnCommandFinished(quint16 command, bool ok, const QString &message);
    void releaseChargerPending();
    bool chargerAwaitFloat = false;
    bool chargerAwaitBattery = false;
    QByteArray chargerFloatRaw;
    QByteArray chargerBatteryRaw;
    void showMessageBox(const QString &title, const QString &text);
    qint64 clockOffsetSecs = 0;     // set when the date/time is saved; added to the system clock
    Rs485Client *rs485;
    QTimer *pollTimer;
    bool daemonOk = false;

    // Battery settings page. The SP515 checks every value against its own limits;
    // the display only reads the current values, lets them be edited, and sends them back.
    enum BatteryBlock { BlockBattery, BlockFloat, BlockBoost, BlockEqualize, BlockInitialize, BlockCount };
    struct BatteryBlockState
    {
        QByteArray raw;             // the config as last read from (or saved to) the SP515
        QByteArray sent;            // what a running save has sent
        bool loaded = false;        // read successfully: safe to edit and save
        bool dirty = false;         // edited since it was loaded
    };
    BatteryBlockState batteryBlocks[BlockCount];
    NumericKeypad *batteryKeypad = nullptr;
    int batteryPendingGets = 0;                     // reads still waiting for an answer
    int batteryPendingSets = 0;                     // saves still waiting for an answer
    bool batterySetFailed = false;
    bool batteryGetFailed = false;
    bool batteryFloatCellVEdited = false;           // the Float V/Cell box was edited
    bool batteryRateEdited = false;                 // the Recharge Rate box was edited (float current limit)
    void updateRechargeRateBox();                   // recharge rate % from the float current limit and the capacity x strings
    bool batteryUnsentEdited = false;               // recharge rate, efficiency or compensation edited: not sent to the SP515
    void setupBatteryPage();
    void setSystemInfo(const QString &line1, const char *color1, const QString &line2, const char *color2 = nullptr);   // two-line text on the dashboard
    void loadBatterySettings();                     // ask the SP515 for all values (when the page opens)
    void handleBatteryReply(quint16 command, const QByteArray &payload);
    bool handleBatteryCommandFinished(quint16 command, bool ok, const QString &message);
    void batteryEdited(int block);                  // the user changed a value in this block
    void setBatteryBlockEnabled(int block, bool enabled);
    void setBatteryStatus(const QString &text, int kind);       // kind: 0 neutral, 1 good, 2 error, 3 warning
    void updateBatteryButtons();
    QLabel *batteryErrorLabel(int block) const;
    QDoubleSpinBox *batteryCellVSpin(int block) const;
    QString batteryErrorText(const QString &message, bool reading) const;

    // PID settings page: the four gains are read when the page opens and saved with SAVE. While
    // the page is open the rectifier voltage and current are polled every 200 ms.
    NumericKeypad *pidKeypad = nullptr;
    QTimer *pidPollTimer = nullptr;
    QByteArray pidRaw;                              // the gains as last read from (or saved to) the SP515
    QByteArray pidSent;                             // what a running save has sent
    bool pidLoaded = false;
    bool pidDirty[4] = {};                          // voltage Kp, voltage Ki, current Kp, current Ki: edited since loaded
    bool pidGetPending = false;
    bool pidSetPending = false;
    void setupPidPage();
    void loadPidSettings();
    void handlePidReply(quint16 command, const QByteArray &payload);
    bool handlePidCommandFinished(quint16 command, bool ok, const QString &message);
    void setPidStatus(const QString &text, int kind);      // kind: 0 neutral, 1 good, 2 error
    QDoubleSpinBox *pidSpin(int index) const;

    // Boost, Equalize and Initialize settings pages: one shared implementation. Each reads its config
    // (and the battery config, for the current limit in %) when the page opens and sends only the
    // edited fields back with SAVE. A field the SP515 config does not have yet (offset -1) can be
    // edited, but is not sent, and the page says so.
    enum ModeKind { ModeBoost, ModeEqualize, ModeInitialize, ModeCount };
    enum ModeField { FieldEnabled, FieldVolts, FieldCurrLim, FieldDuration, FieldOverride, FieldRepeat, FieldGassing, FieldCount };
    struct ModePage
    {
        QWidget *page = nullptr;
        QCheckBox *toggle = nullptr;                                // the Function tick box
        QDoubleSpinBox *volts = nullptr, *currLim = nullptr, *gassing = nullptr;
        QSpinBox *duration = nullptr, *overrideTime = nullptr, *repeat = nullptr;
        QLabel *status = nullptr;
        QPushButton *save = nullptr;
        NumericKeypad *keypad = nullptr;
        quint16 getCommand = 0, setCommand = 0;
        int size = 0;                                               // config size in bytes
        int offset[FieldCount] = {};                                // where each field sits in the config; -1: not in the SP515 config
        QByteArray raw, sent;                                       // the config as last read from (or saved to) the SP515
        bool awaitConfig = false, awaitBattery = false;             // reads waiting for an answer
        bool configLoaded = false, batteryLoaded = false;
        double capacityStrings = 0;                                 // capacity x strings, from the battery config
        bool dirty[FieldCount] = {};
        bool setPending = false;
        bool loading = false;                                       // filling in values from the SP515: not edits
        bool unsentEdited = false;                                  // a field that is not sent was edited
        QString unsentNote;
    };
    ModePage modePages[ModeCount];
    void setupModePages();
    void loadModePage(int kind);
    void saveModePage(int kind);
    void modeFieldEdited(int kind, int field);
    void handleModeReply(quint16 command, const QByteArray &payload);
    bool handleModeCommandFinished(quint16 command, bool ok, const QString &message);
    void setModeStatus(int kind, const QString &text, int level);   // level: 0 neutral, 1 good, 2 error, 3 warning
    void enableModeFields(int kind);

    // Communication Settings page: Wi-Fi and the wired (LAN) port, through NetworkManager (nmcli)
    NetControl *net = nullptr;
    TextKeyboard *commKeyboard = nullptr;
    NumericKeypad *commKeypad = nullptr;
    QTimer *commTimer = nullptr;
    WifiState commWifi;
    LanState commLan;
    bool commWifiKnown = false;                     // the Wi-Fi state has been read at least once
    bool commWifiBusy = false;                      // a Wi-Fi switch or connect is running
    bool commLanBusy = false;                       // a LAN save is running
    bool commScanned = false;                       // a scan was started since the page opened (or Wi-Fi was switched on)
    bool commScanning = false;
    bool commWifiRefreshing = false;
    bool commLanRefreshing = false;
    QString commWifiMessage;                        // the last result, shown under the Wi-Fi state
    bool commWifiMessageIsError = false;
    void setupCommPage();
    void commPageOpened();
    void commStartScan();
    void commRenderWifi();                          // the Wi-Fi state line and the enabled / disabled controls
    void commShowNetworks(const QList<WifiNetwork> &networks, const QString &error);
    void commShowLan(const LanState &state);
    void commUpdateLanFields();
    void commApplyLan();
    void commSetLanMessage(const QString &text, bool error);

    QDateTime appNow() const;       // system time plus the saved offset
    void updateClockLabels();       // dashboard date and time
    void showDateTimePage();        // fill the editors with the current time and open the page
};
#endif // MAINWINDOW_H
