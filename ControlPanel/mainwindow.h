#pragma once

#include <QMainWindow>
#include "common.h"
#include <QTimer>
#include <QLabel>
#include <QElapsedTimer>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setRunScreenValues(enum SpeedProfiles);
public slots:

protected:

private slots:
    void on_linkbtn_clicked();
    void on_extruder_up_btn_clicked();
    void on_extruder_down_btn_clicked();
    void on_caterpillar_up_btn_clicked();
    void on_caterpillar_down_btn_clicked();
    void on_stepper_up_btn_clicked();
    void on_stepper_down_btn_clicked();
    void on_settings_btn_clicked();
    void on_ss_back_btn_clicked();
    void on_buttonGroup_buttonClicked(QAbstractButton *);
    void on_buttonGroup_2_buttonClicked(QAbstractButton *);

    void on_run_btn_clicked();

private:
    Ui::MainWindow *ui;
    enum RunStates e_run_state;
    float f_extruder_rpm = 1000.0;
    float f_caterpillar_rpm = 500.0;
    float f_stepper_pps = 1.1;
    bool b_link = false;
};
