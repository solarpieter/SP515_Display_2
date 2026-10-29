#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QLabel>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QMainWindow>
#include <QTimer>
#include "numerickeypad.h"
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

private slots:
    void on_pushButton_home_clicked();

    void on_pushButton_menu_clicked();

    void on_pushButtonDate_settings_clicked();

    void on_pushButtonDateBack_clicked();

    void on_pushButtonDateSave_clicked();

    void on_pushButtonMenuHome_clicked();

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
    qint64 clockOffsetSecs = 0;     // set when the date/time is saved; added to the system clock
    Rs485Client *rs485;
    QTimer *pollTimer;
    bool daemonOk = false;

    QDateTime appNow() const;       // system time plus the saved offset
    void updateClockLabels();       // dashboard date and time
    void showDateTimePage();        // fill the editors with the current time and open the page
};
#endif // MAINWINDOW_H
