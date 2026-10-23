#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

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

    void on_pushButtonTime_settings_clicked();

    void on_pushButtonDateBack_clicked();

    void on_pushButtonTimeBack_clicked();

    void on_pushButtonDateSave_clicked();

    void on_pushButtonTimeSave_clicked();

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

private:
    Ui::MainWindow *ui;
    bool darkMode;
    QTimer *clockTimer;
};
#endif // MAINWINDOW_H
