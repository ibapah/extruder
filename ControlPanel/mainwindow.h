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
    void on_run_btn_clicked();
    void on_ss_params_editsave_btn_clicked();
    void on_ss_products_cbox_currentIndexChanged(int index);
    void on_ss_maxerpm_ledit_textEdited(const QString &arg1);
    void on_ss_crpmfactor_ledit_textEdited(const QString &arg1);
    void on_ss_colorfactor_ledit_textEdited(const QString &arg1);
    void on_ss_product_edit_btn_clicked();
    void on_ins_input_text_ledit_returnPressed();
    void on_ins_back_btn_clicked();
    void on_ins_input_text_ledit_textEdited(const QString &arg1);
    void on_ss_speed_edit_btn_clicked();
    void on_ss_speeds_cbox_currentIndexChanged(int index);

    void on_ins_input_save_btn_clicked();

public slots:
    void on_productBtngrpButtonClicked(int);
    void on_speedBtngrpButtonClicked(int);

private:
    Ui::MainWindow *ui;
    enum RunStates e_run_state;
    float f_extruder_rpm = 1000.0;
    float f_caterpillar_rpm = 500.0;
    float f_stepper_pps = 1.1;
    bool b_link = false;
    bool ss_profile_edit = false;
    QButtonGroup productBtngrp;
    QButtonGroup speedBtngrp;
    struct ProductParams m_products[MAX_PRODUCT_PARAMS_COUNT];
};
