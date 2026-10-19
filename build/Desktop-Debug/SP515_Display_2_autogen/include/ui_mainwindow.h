/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QStackedWidget *stackedWidget;
    QWidget *pageDashboard;
    QFrame *frameTopdashboard;
    QGridLayout *gridLayout;
    QLabel *label_system_status_2;
    QLabel *label_date_2;
    QLabel *label_time_2;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QFrame *frame_battery_voltage;
    QLabel *label_battery_voltage_2;
    QLabel *label_batt_volt_value_2;
    QFrame *frame_battery_current;
    QLabel *label_battery_current_2;
    QLabel *label_batt_current_value_2;
    QFrame *frame_battery_mode;
    QLabel *label_battery_mode_2;
    QLabel *label_batt_mode_value_2;
    QFrame *frame_battery_temperature;
    QLabel *label_battery_mode_4;
    QLabel *label_batt_mode_value_4;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_2;
    QFrame *frame_load_voltage;
    QLabel *label_load_voltage_2;
    QLabel *label_load_volt_value_2;
    QFrame *frame_load_current;
    QLabel *label_load_current_3;
    QLabel *label_load_current_value_2;
    QFrame *frame_system_alarms;
    QLabel *label_system_alarms_2;
    QLabel *label_alarms_value_2;
    QFrame *frame_dashboard_8;
    QWidget *layoutWidget2;
    QHBoxLayout *horizontalLayout_3;
    QFrame *frame_dashboard_9;
    QFrame *frame_dashboard_10;
    QFrame *frame_dashboard_11;
    QFrame *frame_dashboard_12;
    QWidget *layoutWidget3;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButton_home;
    QPushButton *pushButton_menu;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QWidget *pageMenu;
    QPushButton *pushButtonMenuBack;
    QPushButton *pushButtonMenuHome;
    QPushButton *pushButton_5;
    QWidget *layoutWidget4;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *pushButtonChargerenabledisable;
    QPushButton *pushButtonA1;
    QPushButton *pushButtonA2;
    QWidget *layoutWidget5;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *pushButtonBattery_settings;
    QPushButton *pushButtonAlarm_settings;
    QPushButton *pushButtonDisplay_settings;
    QWidget *layoutWidget6;
    QHBoxLayout *horizontalLayout_7;
    QPushButton *pushButtonDate_settings;
    QPushButton *pushButtonTime_settings;
    QPushButton *pushButtonCommunication_settings;
    QWidget *layoutWidget7;
    QHBoxLayout *horizontalLayout_8;
    QPushButton *pushButtonCharger_settings;
    QPushButton *pushButtonSystom_info;
    QPushButton *pushButtonDiagnostic_service;
    QWidget *pageBattery_settings;
    QWidget *pageAlarm_settings;
    QWidget *pageDisplay_settings;
    QPushButton *pushButtonDisplaySave;
    QPushButton *pushButtonDisplayBack;
    QWidget *layoutWidget8;
    QVBoxLayout *verticalLayoutDisplaytheme;
    QLabel *labelDisplaytheme;
    QRadioButton *radioButtonDark;
    QRadioButton *radioButtonLight;
    QRadioButton *radioButtonHighcontrast;
    QWidget *pageDate_settings;
    QDateEdit *dateEdit;
    QPushButton *pushButtonDateSave;
    QPushButton *pushButtonDateBack;
    QLabel *label;
    QWidget *pageTime_settings;
    QTimeEdit *timeEdit;
    QPushButton *pushButtonTimeBack;
    QPushButton *pushButtonTimeSave;
    QWidget *pageEnable_Disable;
    QPushButton *btnChargerEnable;
    QPushButton *pushButton_Enable_home;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1280, 654);
        MainWindow->setMinimumSize(QSize(800, 600));
        MainWindow->setMaximumSize(QSize(1280, 720));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setMinimumSize(QSize(800, 600));
        centralwidget->setMaximumSize(QSize(1280, 720));
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(0, 0, 1280, 720));
        stackedWidget->setMinimumSize(QSize(1280, 720));
        stackedWidget->setMaximumSize(QSize(1280, 720));
        stackedWidget->setFrameShadow(QFrame::Shadow::Raised);
        stackedWidget->setLineWidth(2);
        pageDashboard = new QWidget();
        pageDashboard->setObjectName("pageDashboard");
        pageDashboard->setMinimumSize(QSize(800, 600));
        pageDashboard->setMaximumSize(QSize(1280, 720));
        frameTopdashboard = new QFrame(pageDashboard);
        frameTopdashboard->setObjectName("frameTopdashboard");
        frameTopdashboard->setGeometry(QRect(10, 10, 1261, 66));
        frameTopdashboard->setFrameShape(QFrame::Shape::Box);
        frameTopdashboard->setFrameShadow(QFrame::Shadow::Raised);
        frameTopdashboard->setLineWidth(2);
        gridLayout = new QGridLayout(frameTopdashboard);
        gridLayout->setObjectName("gridLayout");
        label_system_status_2 = new QLabel(frameTopdashboard);
        label_system_status_2->setObjectName("label_system_status_2");
        QFont font;
        font.setPointSize(22);
        font.setBold(true);
        label_system_status_2->setFont(font);

        gridLayout->addWidget(label_system_status_2, 0, 0, 1, 1);

        label_date_2 = new QLabel(frameTopdashboard);
        label_date_2->setObjectName("label_date_2");
        label_date_2->setFont(font);

        gridLayout->addWidget(label_date_2, 0, 1, 1, 1);

        label_time_2 = new QLabel(frameTopdashboard);
        label_time_2->setObjectName("label_time_2");
        label_time_2->setFont(font);

        gridLayout->addWidget(label_time_2, 0, 2, 1, 1);

        layoutWidget = new QWidget(pageDashboard);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(10, 80, 1261, 121));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        frame_battery_voltage = new QFrame(layoutWidget);
        frame_battery_voltage->setObjectName("frame_battery_voltage");
        frame_battery_voltage->setFrameShape(QFrame::Shape::Box);
        frame_battery_voltage->setFrameShadow(QFrame::Shadow::Raised);
        frame_battery_voltage->setLineWidth(2);
        label_battery_voltage_2 = new QLabel(frame_battery_voltage);
        label_battery_voltage_2->setObjectName("label_battery_voltage_2");
        label_battery_voltage_2->setGeometry(QRect(10, 10, 231, 41));
        QFont font1;
        font1.setPointSize(16);
        font1.setBold(true);
        label_battery_voltage_2->setFont(font1);
        label_battery_voltage_2->setLineWidth(0);
        label_battery_voltage_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_batt_volt_value_2 = new QLabel(frame_battery_voltage);
        label_batt_volt_value_2->setObjectName("label_batt_volt_value_2");
        label_batt_volt_value_2->setGeometry(QRect(10, 60, 181, 51));
        QFont font2;
        font2.setPointSize(26);
        font2.setBold(true);
        label_batt_volt_value_2->setFont(font2);
        label_batt_volt_value_2->setFrameShape(QFrame::Shape::NoFrame);
        label_batt_volt_value_2->setFrameShadow(QFrame::Shadow::Plain);
        label_batt_volt_value_2->setLineWidth(0);
        label_batt_volt_value_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout->addWidget(frame_battery_voltage);

        frame_battery_current = new QFrame(layoutWidget);
        frame_battery_current->setObjectName("frame_battery_current");
        frame_battery_current->setFrameShape(QFrame::Shape::Box);
        frame_battery_current->setFrameShadow(QFrame::Shadow::Raised);
        frame_battery_current->setLineWidth(2);
        label_battery_current_2 = new QLabel(frame_battery_current);
        label_battery_current_2->setObjectName("label_battery_current_2");
        label_battery_current_2->setGeometry(QRect(10, 10, 231, 41));
        label_battery_current_2->setFont(font1);
        label_battery_current_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_batt_current_value_2 = new QLabel(frame_battery_current);
        label_batt_current_value_2->setObjectName("label_batt_current_value_2");
        label_batt_current_value_2->setGeometry(QRect(10, 60, 181, 51));
        label_batt_current_value_2->setFont(font2);
        label_batt_current_value_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout->addWidget(frame_battery_current);

        frame_battery_mode = new QFrame(layoutWidget);
        frame_battery_mode->setObjectName("frame_battery_mode");
        frame_battery_mode->setFrameShape(QFrame::Shape::Box);
        frame_battery_mode->setFrameShadow(QFrame::Shadow::Raised);
        frame_battery_mode->setLineWidth(2);
        label_battery_mode_2 = new QLabel(frame_battery_mode);
        label_battery_mode_2->setObjectName("label_battery_mode_2");
        label_battery_mode_2->setGeometry(QRect(10, 10, 231, 41));
        label_battery_mode_2->setFont(font1);
        label_battery_mode_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_batt_mode_value_2 = new QLabel(frame_battery_mode);
        label_batt_mode_value_2->setObjectName("label_batt_mode_value_2");
        label_batt_mode_value_2->setGeometry(QRect(10, 60, 181, 51));
        label_batt_mode_value_2->setFont(font2);
        label_batt_mode_value_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout->addWidget(frame_battery_mode);

        frame_battery_temperature = new QFrame(layoutWidget);
        frame_battery_temperature->setObjectName("frame_battery_temperature");
        frame_battery_temperature->setFrameShape(QFrame::Shape::Box);
        frame_battery_temperature->setFrameShadow(QFrame::Shadow::Raised);
        frame_battery_temperature->setLineWidth(2);
        label_battery_mode_4 = new QLabel(frame_battery_temperature);
        label_battery_mode_4->setObjectName("label_battery_mode_4");
        label_battery_mode_4->setGeometry(QRect(10, 10, 231, 41));
        label_battery_mode_4->setFont(font1);
        label_battery_mode_4->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_batt_mode_value_4 = new QLabel(frame_battery_temperature);
        label_batt_mode_value_4->setObjectName("label_batt_mode_value_4");
        label_batt_mode_value_4->setGeometry(QRect(10, 60, 181, 51));
        label_batt_mode_value_4->setFont(font2);
        label_batt_mode_value_4->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout->addWidget(frame_battery_temperature);

        layoutWidget1 = new QWidget(pageDashboard);
        layoutWidget1->setObjectName("layoutWidget1");
        layoutWidget1->setGeometry(QRect(10, 210, 1261, 121));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        frame_load_voltage = new QFrame(layoutWidget1);
        frame_load_voltage->setObjectName("frame_load_voltage");
        frame_load_voltage->setFrameShape(QFrame::Shape::Box);
        frame_load_voltage->setFrameShadow(QFrame::Shadow::Raised);
        frame_load_voltage->setLineWidth(2);
        label_load_voltage_2 = new QLabel(frame_load_voltage);
        label_load_voltage_2->setObjectName("label_load_voltage_2");
        label_load_voltage_2->setGeometry(QRect(10, 10, 201, 41));
        QFont font3;
        font3.setPointSize(13);
        font3.setBold(true);
        label_load_voltage_2->setFont(font3);
        label_load_voltage_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_load_volt_value_2 = new QLabel(frame_load_voltage);
        label_load_volt_value_2->setObjectName("label_load_volt_value_2");
        label_load_volt_value_2->setGeometry(QRect(10, 60, 181, 51));
        label_load_volt_value_2->setFont(font2);
        label_load_volt_value_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_2->addWidget(frame_load_voltage);

        frame_load_current = new QFrame(layoutWidget1);
        frame_load_current->setObjectName("frame_load_current");
        frame_load_current->setFrameShape(QFrame::Shape::Box);
        frame_load_current->setFrameShadow(QFrame::Shadow::Raised);
        frame_load_current->setLineWidth(2);
        label_load_current_3 = new QLabel(frame_load_current);
        label_load_current_3->setObjectName("label_load_current_3");
        label_load_current_3->setGeometry(QRect(10, 10, 201, 41));
        label_load_current_3->setFont(font3);
        label_load_current_3->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_load_current_value_2 = new QLabel(frame_load_current);
        label_load_current_value_2->setObjectName("label_load_current_value_2");
        label_load_current_value_2->setGeometry(QRect(20, 60, 181, 51));
        label_load_current_value_2->setFont(font2);
        label_load_current_value_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_2->addWidget(frame_load_current);

        frame_system_alarms = new QFrame(layoutWidget1);
        frame_system_alarms->setObjectName("frame_system_alarms");
        frame_system_alarms->setFrameShape(QFrame::Shape::Box);
        frame_system_alarms->setFrameShadow(QFrame::Shadow::Raised);
        frame_system_alarms->setLineWidth(2);
        label_system_alarms_2 = new QLabel(frame_system_alarms);
        label_system_alarms_2->setObjectName("label_system_alarms_2");
        label_system_alarms_2->setGeometry(QRect(10, 10, 201, 41));
        label_system_alarms_2->setFont(font3);
        label_system_alarms_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_alarms_value_2 = new QLabel(frame_system_alarms);
        label_alarms_value_2->setObjectName("label_alarms_value_2");
        label_alarms_value_2->setGeometry(QRect(0, 50, 211, 61));
        QFont font4;
        font4.setPointSize(20);
        font4.setBold(true);
        label_alarms_value_2->setFont(font4);
        label_alarms_value_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_2->addWidget(frame_system_alarms);

        frame_dashboard_8 = new QFrame(layoutWidget1);
        frame_dashboard_8->setObjectName("frame_dashboard_8");
        frame_dashboard_8->setFrameShape(QFrame::Shape::Box);
        frame_dashboard_8->setFrameShadow(QFrame::Shadow::Raised);
        frame_dashboard_8->setLineWidth(2);

        horizontalLayout_2->addWidget(frame_dashboard_8);

        layoutWidget2 = new QWidget(pageDashboard);
        layoutWidget2->setObjectName("layoutWidget2");
        layoutWidget2->setGeometry(QRect(10, 340, 1261, 121));
        horizontalLayout_3 = new QHBoxLayout(layoutWidget2);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        frame_dashboard_9 = new QFrame(layoutWidget2);
        frame_dashboard_9->setObjectName("frame_dashboard_9");
        frame_dashboard_9->setFrameShape(QFrame::Shape::Box);
        frame_dashboard_9->setFrameShadow(QFrame::Shadow::Raised);
        frame_dashboard_9->setLineWidth(2);

        horizontalLayout_3->addWidget(frame_dashboard_9);

        frame_dashboard_10 = new QFrame(layoutWidget2);
        frame_dashboard_10->setObjectName("frame_dashboard_10");
        frame_dashboard_10->setFrameShape(QFrame::Shape::Box);
        frame_dashboard_10->setFrameShadow(QFrame::Shadow::Raised);
        frame_dashboard_10->setLineWidth(2);

        horizontalLayout_3->addWidget(frame_dashboard_10);

        frame_dashboard_11 = new QFrame(layoutWidget2);
        frame_dashboard_11->setObjectName("frame_dashboard_11");
        frame_dashboard_11->setFrameShape(QFrame::Shape::Box);
        frame_dashboard_11->setFrameShadow(QFrame::Shadow::Raised);
        frame_dashboard_11->setLineWidth(2);

        horizontalLayout_3->addWidget(frame_dashboard_11);

        frame_dashboard_12 = new QFrame(layoutWidget2);
        frame_dashboard_12->setObjectName("frame_dashboard_12");
        frame_dashboard_12->setFrameShape(QFrame::Shape::Box);
        frame_dashboard_12->setFrameShadow(QFrame::Shadow::Raised);
        frame_dashboard_12->setLineWidth(2);

        horizontalLayout_3->addWidget(frame_dashboard_12);

        layoutWidget3 = new QWidget(pageDashboard);
        layoutWidget3->setObjectName("layoutWidget3");
        layoutWidget3->setGeometry(QRect(10, 470, 1261, 131));
        horizontalLayout_4 = new QHBoxLayout(layoutWidget3);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        pushButton_home = new QPushButton(layoutWidget3);
        pushButton_home->setObjectName("pushButton_home");
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("go-home")));
        pushButton_home->setIcon(icon);
        pushButton_home->setIconSize(QSize(60, 60));

        horizontalLayout_4->addWidget(pushButton_home);

        pushButton_menu = new QPushButton(layoutWidget3);
        pushButton_menu->setObjectName("pushButton_menu");
        QFont font5;
        font5.setPointSize(16);
        pushButton_menu->setFont(font5);
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("preferences-system")));
        pushButton_menu->setIcon(icon1);
        pushButton_menu->setIconSize(QSize(60, 60));

        horizontalLayout_4->addWidget(pushButton_menu);

        pushButton = new QPushButton(layoutWidget3);
        pushButton->setObjectName("pushButton");
        pushButton->setFont(font5);
        QIcon icon2(QIcon::fromTheme(QIcon::ThemeIcon::Battery));
        pushButton->setIcon(icon2);
        pushButton->setIconSize(QSize(60, 60));

        horizontalLayout_4->addWidget(pushButton);

        pushButton_2 = new QPushButton(layoutWidget3);
        pushButton_2->setObjectName("pushButton_2");
        QIcon icon3(QIcon::fromTheme(QIcon::ThemeIcon::NetworkWireless));
        pushButton_2->setIcon(icon3);
        pushButton_2->setIconSize(QSize(60, 60));

        horizontalLayout_4->addWidget(pushButton_2);

        pushButton_3 = new QPushButton(layoutWidget3);
        pushButton_3->setObjectName("pushButton_3");
        QIcon icon4(QIcon::fromTheme(QString::fromUtf8("text-x-generic")));
        pushButton_3->setIcon(icon4);
        pushButton_3->setIconSize(QSize(60, 60));

        horizontalLayout_4->addWidget(pushButton_3);

        pushButton_4 = new QPushButton(layoutWidget3);
        pushButton_4->setObjectName("pushButton_4");
        QIcon icon5(QIcon::fromTheme(QString::fromUtf8("dialog-password")));
        pushButton_4->setIcon(icon5);
        pushButton_4->setIconSize(QSize(60, 60));

        horizontalLayout_4->addWidget(pushButton_4);

        stackedWidget->addWidget(pageDashboard);
        pageMenu = new QWidget();
        pageMenu->setObjectName("pageMenu");
        pushButtonMenuBack = new QPushButton(pageMenu);
        pushButtonMenuBack->setObjectName("pushButtonMenuBack");
        pushButtonMenuBack->setGeometry(QRect(280, 530, 171, 61));
        pushButtonMenuBack->setFont(font1);
        pushButtonMenuHome = new QPushButton(pageMenu);
        pushButtonMenuHome->setObjectName("pushButtonMenuHome");
        pushButtonMenuHome->setGeometry(QRect(90, 530, 171, 61));
        pushButtonMenuHome->setFont(font1);
        pushButton_5 = new QPushButton(pageMenu);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(490, 530, 231, 61));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8("Recources/ICON/accounts.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButton_5->setIcon(icon6);
        pushButton_5->setIconSize(QSize(60, 60));
        layoutWidget4 = new QWidget(pageMenu);
        layoutWidget4->setObjectName("layoutWidget4");
        layoutWidget4->setGeometry(QRect(10, 20, 1261, 121));
        horizontalLayout_5 = new QHBoxLayout(layoutWidget4);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        pushButtonChargerenabledisable = new QPushButton(layoutWidget4);
        pushButtonChargerenabledisable->setObjectName("pushButtonChargerenabledisable");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButtonChargerenabledisable->sizePolicy().hasHeightForWidth());
        pushButtonChargerenabledisable->setSizePolicy(sizePolicy);
        QFont font6;
        font6.setPointSize(18);
        font6.setBold(true);
        pushButtonChargerenabledisable->setFont(font6);

        horizontalLayout_5->addWidget(pushButtonChargerenabledisable);

        pushButtonA1 = new QPushButton(layoutWidget4);
        pushButtonA1->setObjectName("pushButtonA1");
        sizePolicy.setHeightForWidth(pushButtonA1->sizePolicy().hasHeightForWidth());
        pushButtonA1->setSizePolicy(sizePolicy);
        pushButtonA1->setFont(font6);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/Recources/ICON/Recources/ICON/time.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButtonA1->setIcon(icon7);
        pushButtonA1->setIconSize(QSize(60, 60));

        horizontalLayout_5->addWidget(pushButtonA1);

        pushButtonA2 = new QPushButton(layoutWidget4);
        pushButtonA2->setObjectName("pushButtonA2");
        sizePolicy.setHeightForWidth(pushButtonA2->sizePolicy().hasHeightForWidth());
        pushButtonA2->setSizePolicy(sizePolicy);
        pushButtonA2->setFont(font6);

        horizontalLayout_5->addWidget(pushButtonA2);

        layoutWidget5 = new QWidget(pageMenu);
        layoutWidget5->setObjectName("layoutWidget5");
        layoutWidget5->setGeometry(QRect(10, 150, 1261, 111));
        horizontalLayout_6 = new QHBoxLayout(layoutWidget5);
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        pushButtonBattery_settings = new QPushButton(layoutWidget5);
        pushButtonBattery_settings->setObjectName("pushButtonBattery_settings");
        sizePolicy.setHeightForWidth(pushButtonBattery_settings->sizePolicy().hasHeightForWidth());
        pushButtonBattery_settings->setSizePolicy(sizePolicy);
        QFont font7;
        font7.setPointSize(24);
        font7.setBold(true);
        pushButtonBattery_settings->setFont(font7);

        horizontalLayout_6->addWidget(pushButtonBattery_settings);

        pushButtonAlarm_settings = new QPushButton(layoutWidget5);
        pushButtonAlarm_settings->setObjectName("pushButtonAlarm_settings");
        sizePolicy.setHeightForWidth(pushButtonAlarm_settings->sizePolicy().hasHeightForWidth());
        pushButtonAlarm_settings->setSizePolicy(sizePolicy);
        pushButtonAlarm_settings->setFont(font6);

        horizontalLayout_6->addWidget(pushButtonAlarm_settings);

        pushButtonDisplay_settings = new QPushButton(layoutWidget5);
        pushButtonDisplay_settings->setObjectName("pushButtonDisplay_settings");
        sizePolicy.setHeightForWidth(pushButtonDisplay_settings->sizePolicy().hasHeightForWidth());
        pushButtonDisplay_settings->setSizePolicy(sizePolicy);
        pushButtonDisplay_settings->setFont(font6);

        horizontalLayout_6->addWidget(pushButtonDisplay_settings);

        layoutWidget6 = new QWidget(pageMenu);
        layoutWidget6->setObjectName("layoutWidget6");
        layoutWidget6->setGeometry(QRect(10, 270, 1261, 121));
        horizontalLayout_7 = new QHBoxLayout(layoutWidget6);
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        horizontalLayout_7->setContentsMargins(0, 0, 0, 0);
        pushButtonDate_settings = new QPushButton(layoutWidget6);
        pushButtonDate_settings->setObjectName("pushButtonDate_settings");
        sizePolicy.setHeightForWidth(pushButtonDate_settings->sizePolicy().hasHeightForWidth());
        pushButtonDate_settings->setSizePolicy(sizePolicy);
        pushButtonDate_settings->setFont(font6);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8("Recources/ICON/calendar.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButtonDate_settings->setIcon(icon8);
        pushButtonDate_settings->setIconSize(QSize(60, 60));

        horizontalLayout_7->addWidget(pushButtonDate_settings);

        pushButtonTime_settings = new QPushButton(layoutWidget6);
        pushButtonTime_settings->setObjectName("pushButtonTime_settings");
        sizePolicy.setHeightForWidth(pushButtonTime_settings->sizePolicy().hasHeightForWidth());
        pushButtonTime_settings->setSizePolicy(sizePolicy);
        pushButtonTime_settings->setFont(font6);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8("Recources/ICON/time.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButtonTime_settings->setIcon(icon9);
        pushButtonTime_settings->setIconSize(QSize(60, 60));
        pushButtonTime_settings->setAutoRepeatDelay(300);

        horizontalLayout_7->addWidget(pushButtonTime_settings);

        pushButtonCommunication_settings = new QPushButton(layoutWidget6);
        pushButtonCommunication_settings->setObjectName("pushButtonCommunication_settings");
        sizePolicy.setHeightForWidth(pushButtonCommunication_settings->sizePolicy().hasHeightForWidth());
        pushButtonCommunication_settings->setSizePolicy(sizePolicy);
        QFont font8;
        font8.setPointSize(18);
        font8.setBold(true);
        font8.setKerning(true);
        font8.setHintingPreference(QFont::PreferDefaultHinting);
        pushButtonCommunication_settings->setFont(font8);
        pushButtonCommunication_settings->setCursor(QCursor(Qt::CursorShape::ArrowCursor));

        horizontalLayout_7->addWidget(pushButtonCommunication_settings);

        layoutWidget7 = new QWidget(pageMenu);
        layoutWidget7->setObjectName("layoutWidget7");
        layoutWidget7->setGeometry(QRect(10, 400, 1261, 111));
        horizontalLayout_8 = new QHBoxLayout(layoutWidget7);
        horizontalLayout_8->setObjectName("horizontalLayout_8");
        horizontalLayout_8->setContentsMargins(0, 0, 0, 0);
        pushButtonCharger_settings = new QPushButton(layoutWidget7);
        pushButtonCharger_settings->setObjectName("pushButtonCharger_settings");
        sizePolicy.setHeightForWidth(pushButtonCharger_settings->sizePolicy().hasHeightForWidth());
        pushButtonCharger_settings->setSizePolicy(sizePolicy);
        pushButtonCharger_settings->setFont(font6);

        horizontalLayout_8->addWidget(pushButtonCharger_settings);

        pushButtonSystom_info = new QPushButton(layoutWidget7);
        pushButtonSystom_info->setObjectName("pushButtonSystom_info");
        sizePolicy.setHeightForWidth(pushButtonSystom_info->sizePolicy().hasHeightForWidth());
        pushButtonSystom_info->setSizePolicy(sizePolicy);
        pushButtonSystom_info->setFont(font6);

        horizontalLayout_8->addWidget(pushButtonSystom_info);

        pushButtonDiagnostic_service = new QPushButton(layoutWidget7);
        pushButtonDiagnostic_service->setObjectName("pushButtonDiagnostic_service");
        sizePolicy.setHeightForWidth(pushButtonDiagnostic_service->sizePolicy().hasHeightForWidth());
        pushButtonDiagnostic_service->setSizePolicy(sizePolicy);
        pushButtonDiagnostic_service->setFont(font8);
        pushButtonDiagnostic_service->setCursor(QCursor(Qt::CursorShape::ArrowCursor));

        horizontalLayout_8->addWidget(pushButtonDiagnostic_service);

        stackedWidget->addWidget(pageMenu);
        pageBattery_settings = new QWidget();
        pageBattery_settings->setObjectName("pageBattery_settings");
        QFont font9;
        font9.setPointSize(9);
        pageBattery_settings->setFont(font9);
        stackedWidget->addWidget(pageBattery_settings);
        pageAlarm_settings = new QWidget();
        pageAlarm_settings->setObjectName("pageAlarm_settings");
        stackedWidget->addWidget(pageAlarm_settings);
        pageDisplay_settings = new QWidget();
        pageDisplay_settings->setObjectName("pageDisplay_settings");
        pushButtonDisplaySave = new QPushButton(pageDisplay_settings);
        pushButtonDisplaySave->setObjectName("pushButtonDisplaySave");
        pushButtonDisplaySave->setGeometry(QRect(110, 410, 171, 81));
        pushButtonDisplaySave->setFont(font1);
        QIcon icon10(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave));
        pushButtonDisplaySave->setIcon(icon10);
        pushButtonDisplaySave->setIconSize(QSize(60, 60));
        pushButtonDisplayBack = new QPushButton(pageDisplay_settings);
        pushButtonDisplayBack->setObjectName("pushButtonDisplayBack");
        pushButtonDisplayBack->setGeometry(QRect(300, 410, 171, 81));
        pushButtonDisplayBack->setFont(font1);
        layoutWidget8 = new QWidget(pageDisplay_settings);
        layoutWidget8->setObjectName("layoutWidget8");
        layoutWidget8->setGeometry(QRect(100, 80, 188, 171));
        verticalLayoutDisplaytheme = new QVBoxLayout(layoutWidget8);
        verticalLayoutDisplaytheme->setObjectName("verticalLayoutDisplaytheme");
        verticalLayoutDisplaytheme->setContentsMargins(0, 0, 0, 0);
        labelDisplaytheme = new QLabel(layoutWidget8);
        labelDisplaytheme->setObjectName("labelDisplaytheme");
        labelDisplaytheme->setFont(font1);
        labelDisplaytheme->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayoutDisplaytheme->addWidget(labelDisplaytheme);

        radioButtonDark = new QRadioButton(layoutWidget8);
        radioButtonDark->setObjectName("radioButtonDark");
        QFont font10;
        font10.setPointSize(12);
        font10.setBold(true);
        radioButtonDark->setFont(font10);

        verticalLayoutDisplaytheme->addWidget(radioButtonDark);

        radioButtonLight = new QRadioButton(layoutWidget8);
        radioButtonLight->setObjectName("radioButtonLight");
        radioButtonLight->setFont(font10);

        verticalLayoutDisplaytheme->addWidget(radioButtonLight);

        radioButtonHighcontrast = new QRadioButton(layoutWidget8);
        radioButtonHighcontrast->setObjectName("radioButtonHighcontrast");
        radioButtonHighcontrast->setFont(font10);

        verticalLayoutDisplaytheme->addWidget(radioButtonHighcontrast);

        stackedWidget->addWidget(pageDisplay_settings);
        pageDate_settings = new QWidget();
        pageDate_settings->setObjectName("pageDate_settings");
        dateEdit = new QDateEdit(pageDate_settings);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setGeometry(QRect(170, 210, 371, 131));
        QFont font11;
        font11.setPointSize(36);
        font11.setBold(true);
        dateEdit->setFont(font11);
        dateEdit->setDateTime(QDateTime(QDate(2026, 2, 8), QTime(0, 0, 0)));
        pushButtonDateSave = new QPushButton(pageDate_settings);
        pushButtonDateSave->setObjectName("pushButtonDateSave");
        pushButtonDateSave->setGeometry(QRect(40, 510, 161, 61));
        pushButtonDateSave->setFont(font1);
        pushButtonDateBack = new QPushButton(pageDate_settings);
        pushButtonDateBack->setObjectName("pushButtonDateBack");
        pushButtonDateBack->setGeometry(QRect(230, 510, 161, 61));
        pushButtonDateBack->setFont(font1);
        label = new QLabel(pageDate_settings);
        label->setObjectName("label");
        label->setGeometry(QRect(180, 90, 341, 91));
        QFont font12;
        font12.setPointSize(34);
        font12.setBold(true);
        label->setFont(font12);
        stackedWidget->addWidget(pageDate_settings);
        pageTime_settings = new QWidget();
        pageTime_settings->setObjectName("pageTime_settings");
        timeEdit = new QTimeEdit(pageTime_settings);
        timeEdit->setObjectName("timeEdit");
        timeEdit->setGeometry(QRect(170, 210, 261, 111));
        QFont font13;
        font13.setPointSize(56);
        font13.setBold(true);
        timeEdit->setFont(font13);
        pushButtonTimeBack = new QPushButton(pageTime_settings);
        pushButtonTimeBack->setObjectName("pushButtonTimeBack");
        pushButtonTimeBack->setGeometry(QRect(250, 500, 161, 61));
        pushButtonTimeBack->setFont(font1);
        pushButtonTimeSave = new QPushButton(pageTime_settings);
        pushButtonTimeSave->setObjectName("pushButtonTimeSave");
        pushButtonTimeSave->setGeometry(QRect(70, 500, 161, 61));
        pushButtonTimeSave->setFont(font1);
        stackedWidget->addWidget(pageTime_settings);
        pageEnable_Disable = new QWidget();
        pageEnable_Disable->setObjectName("pageEnable_Disable");
        btnChargerEnable = new QPushButton(pageEnable_Disable);
        btnChargerEnable->setObjectName("btnChargerEnable");
        btnChargerEnable->setGeometry(QRect(310, 50, 241, 141));
        btnChargerEnable->setFont(font2);
        btnChargerEnable->setCheckable(true);
        pushButton_Enable_home = new QPushButton(pageEnable_Disable);
        pushButton_Enable_home->setObjectName("pushButton_Enable_home");
        pushButton_Enable_home->setGeometry(QRect(30, 430, 131, 71));
        pushButton_Enable_home->setFont(font1);
        stackedWidget->addWidget(pageEnable_Disable);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1280, 35));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label_system_status_2->setText(QCoreApplication::translate("MainWindow", "System Status", nullptr));
        label_date_2->setText(QCoreApplication::translate("MainWindow", "Date", nullptr));
        label_time_2->setText(QCoreApplication::translate("MainWindow", "Time", nullptr));
        label_battery_voltage_2->setText(QCoreApplication::translate("MainWindow", "BATTERY VOLTAGE", nullptr));
        label_batt_volt_value_2->setText(QCoreApplication::translate("MainWindow", "121.2 V", nullptr));
        label_battery_current_2->setText(QCoreApplication::translate("MainWindow", "BATTERY CURRENT", nullptr));
        label_batt_current_value_2->setText(QCoreApplication::translate("MainWindow", "21.5 A", nullptr));
        label_battery_mode_2->setText(QCoreApplication::translate("MainWindow", "BATTERY MODE", nullptr));
        label_batt_mode_value_2->setText(QCoreApplication::translate("MainWindow", "FLOAT", nullptr));
        label_battery_mode_4->setText(QCoreApplication::translate("MainWindow", "BATTERY TEMP", nullptr));
        label_batt_mode_value_4->setText(QCoreApplication::translate("MainWindow", "22.3 C", nullptr));
        label_load_voltage_2->setText(QCoreApplication::translate("MainWindow", "LOAD VOLTAGE", nullptr));
        label_load_volt_value_2->setText(QCoreApplication::translate("MainWindow", "120.8 V", nullptr));
        label_load_current_3->setText(QCoreApplication::translate("MainWindow", "LOAD CURRENT", nullptr));
        label_load_current_value_2->setText(QCoreApplication::translate("MainWindow", "10.8 A", nullptr));
        label_system_alarms_2->setText(QCoreApplication::translate("MainWindow", "SYSTEM ALARMS", nullptr));
        label_alarms_value_2->setText(QCoreApplication::translate("MainWindow", "NO ALARMS", nullptr));
        pushButton_home->setText(QString());
        pushButton_menu->setText(QString());
        pushButton->setText(QString());
        pushButton_2->setText(QString());
        pushButton_3->setText(QString());
        pushButton_4->setText(QString());
        pushButtonMenuBack->setText(QCoreApplication::translate("MainWindow", "BACK", nullptr));
        pushButtonMenuHome->setText(QCoreApplication::translate("MainWindow", "HOME", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "LOGIN", nullptr));
        pushButtonChargerenabledisable->setText(QCoreApplication::translate("MainWindow", "Charger Enable\n"
" / Disable", nullptr));
        pushButtonA1->setText(QString());
        pushButtonA2->setText(QString());
        pushButtonBattery_settings->setText(QCoreApplication::translate("MainWindow", "Battery Settings", nullptr));
        pushButtonAlarm_settings->setText(QCoreApplication::translate("MainWindow", "Alarm Settings", nullptr));
        pushButtonDisplay_settings->setText(QCoreApplication::translate("MainWindow", "Display Settings", nullptr));
        pushButtonDate_settings->setText(QCoreApplication::translate("MainWindow", "Date Settings", nullptr));
        pushButtonTime_settings->setText(QCoreApplication::translate("MainWindow", "Time Settings", nullptr));
        pushButtonCommunication_settings->setText(QCoreApplication::translate("MainWindow", "Communication\n"
" Settings", nullptr));
        pushButtonCharger_settings->setText(QCoreApplication::translate("MainWindow", "Charger Settings", nullptr));
        pushButtonSystom_info->setText(QCoreApplication::translate("MainWindow", "System Info", nullptr));
        pushButtonDiagnostic_service->setText(QCoreApplication::translate("MainWindow", "Diagnostic\n"
" Service", nullptr));
        pushButtonDisplaySave->setText(QString());
        pushButtonDisplayBack->setText(QCoreApplication::translate("MainWindow", "BACK", nullptr));
        labelDisplaytheme->setText(QCoreApplication::translate("MainWindow", "Display Theme", nullptr));
        radioButtonDark->setText(QCoreApplication::translate("MainWindow", "DARK", nullptr));
        radioButtonLight->setText(QCoreApplication::translate("MainWindow", "LIGHT", nullptr));
        radioButtonHighcontrast->setText(QCoreApplication::translate("MainWindow", "HIGH CONTRAST", nullptr));
        dateEdit->setDisplayFormat(QCoreApplication::translate("MainWindow", "dd/MM/yyyy", nullptr));
        pushButtonDateSave->setText(QCoreApplication::translate("MainWindow", "SAVE", nullptr));
        pushButtonDateBack->setText(QCoreApplication::translate("MainWindow", "BACK", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "dd/mm/yyyy", nullptr));
        pushButtonTimeBack->setText(QCoreApplication::translate("MainWindow", "BACK", nullptr));
        pushButtonTimeSave->setText(QCoreApplication::translate("MainWindow", "SAVE", nullptr));
        btnChargerEnable->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        pushButton_Enable_home->setText(QCoreApplication::translate("MainWindow", "HOME", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
