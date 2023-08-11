#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QTimer>
#include <QQueue>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMinimumSize(1920, 1080);
    this->setMaximumSize(1920, 1080);
    this->setGeometry(0, 0, 1920, 1080);

#ifdef __arm__
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags( flags | Qt::FramelessWindowHint );
#endif
    // hide unused products
    ui->product4_rbtn->hide();
    ui->product5_rbtn->hide();

    e_run_state = eSTATE_STOPPED;
    ui->extruder_lcd->display(f_extruder_rpm);
    ui->caterpillar_lcd->display(f_caterpillar_rpm);
    ui->stepper_lcd->display(f_stepper_pps);
    ui->speed1_rbtn->setChecked(true);
    ui->product1_rbtn->setChecked(true);

    on_linkbtn_clicked();

    QList<QRadioButton *> productButtons = ui->products_grpbox->findChildren<QRadioButton *>();
    for(int i = 0; i < productButtons.size(); ++i) {
        productBtngrp.addButton(productButtons[i],i);
    }
    QList<QRadioButton *> speedButtons = ui->speeds_grpbox->findChildren<QRadioButton *>();
    for(int i = 0; i < speedButtons.size(); ++i) {
       speedBtngrp.addButton(speedButtons[i],i);
    }
    connect(&productBtngrp, SIGNAL(buttonClicked(int)), this, SLOT(on_productBtngrpButtonClicked(int)) );
    connect(&speedBtngrp, SIGNAL(buttonClicked(int)), this, SLOT(on_speedBtngrpButtonClicked(int)) );

    // init with dummy values
    strcpy(m_products[0].name, "1.25 inch");
    strcpy(m_products[1].name, "13/16 mm");
    strcpy(m_products[2].name, "2 inch");
    for (int i=0; i<MAX_PRODUCT_PARAMS_COUNT; i++) {
        for(int j=0; j<MAX_SPEED_PARAMS_COUNT; j++) {
            m_products[i].params[j].erpm = ((i+1)*(j+1)+2)*100;
            m_products[i].params[j].crpm = ((i+1)*(j+1)+2)*50;
            m_products[i].params[j].color = (float)4.6+((i+1)*(j+1)+1.0);
            //qDebug()<<i<<":"<<j<<"["<<m_products[i].params[j].erpm<<","<<m_products[i].params[j].crpm<<","<<m_products[i].params[j].color<<"]";
        }
    }

    return;
}

MainWindow::~MainWindow()
{

}

void MainWindow::on_linkbtn_clicked()
{
    if (b_link) {
        b_link = false;
        // set values
        int product_idx = productBtngrp.checkedId();
        int speed_idx = speedBtngrp.checkedId();
        ui->extruder_lcd->display(m_products[product_idx].params[speed_idx].erpm);
        ui->caterpillar_lcd->display(m_products[product_idx].params[speed_idx].crpm);
        ui->stepper_lcd->display(m_products[product_idx].params[speed_idx].color);
        ui->hline1->show();
        ui->hline2->show();
        ui->hline3->show();
        ui->hline4->show();
        ui->vline1->show();
        ui->vline2->show();
        ui->linkbtn->setStyleSheet("image: url(:/icons/link.svg); background-color : rgb(63, 73, 127); border: 3px solid rgb(39, 55, 77);");
    } else {
        b_link = true;
        ui->hline1->hide();
        ui->hline2->hide();
        ui->hline3->hide();
        ui->hline4->hide();
        ui->vline1->hide();
        ui->vline2->hide();
        ui->linkbtn->setStyleSheet("image: url(:/icons/unlink.svg); border: 3px solid rgb(39, 55, 77);");
    }
}

void MainWindow::on_extruder_up_btn_clicked()
{
    f_extruder_rpm += 1;
    ui->extruder_lcd->display(f_extruder_rpm);
}

void MainWindow::on_extruder_down_btn_clicked()
{
    f_extruder_rpm -= 1;
    ui->extruder_lcd->display(f_extruder_rpm);
}

void MainWindow::on_caterpillar_up_btn_clicked()
{
    f_caterpillar_rpm += 1;
    ui->caterpillar_lcd->display(f_caterpillar_rpm);
}

void MainWindow::on_caterpillar_down_btn_clicked()
{
    f_caterpillar_rpm -= 1;
    ui->caterpillar_lcd->display(f_caterpillar_rpm);
}

void MainWindow::on_stepper_up_btn_clicked()
{
    f_stepper_pps += 1;
    ui->stepper_lcd->display(f_stepper_pps);
}

void MainWindow::on_stepper_down_btn_clicked()
{
    f_stepper_pps -= 1;
    ui->stepper_lcd->display(f_stepper_pps);
}

void MainWindow::on_settings_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(eSETTINGS_SCREEN);
    ui->ss_products_cbox->setCurrentIndex(1);
    ui->ss_products_cbox->setCurrentIndex(0);
    // disable edit
    ui->ss_maxerpm_ledit->setReadOnly(true);
    ui->ss_crpmfactor_ledit->setReadOnly(true);
    ui->ss_colorfactor_ledit->setReadOnly(true);
}

void MainWindow::on_ss_back_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(eRUN_SCREEN);
}

void MainWindow::on_productBtngrpButtonClicked(int product_idx)
{
    if (b_link)
        return;
    int speed_idx = speedBtngrp.checkedId();
    ui->extruder_lcd->display(m_products[product_idx].params[speed_idx].erpm);
    ui->caterpillar_lcd->display(m_products[product_idx].params[speed_idx].crpm);
    ui->stepper_lcd->display(m_products[product_idx].params[speed_idx].color);
}

void MainWindow::on_speedBtngrpButtonClicked(int speed_idx)
{
    if (b_link)
        return;
    int product_idx = productBtngrp.checkedId();
    ui->extruder_lcd->display(m_products[product_idx].params[speed_idx].erpm);
    ui->caterpillar_lcd->display(m_products[product_idx].params[speed_idx].crpm);
    ui->stepper_lcd->display(m_products[product_idx].params[speed_idx].color);
}

void MainWindow::on_run_btn_clicked()
{
    if ( ! ui->run_btn->text().compare("START") ) {
        e_run_state = eSTATE_STOPPED;
        ui->products_grpbox->setEnabled(false);
        ui->speeds_grpbox->setEnabled(false);
        ui->run_btn->setText("STOP");
        ui->run_btn->setStyleSheet("background-color: rgb(246, 97, 81); border-radius: 10px;");
    } else {
        e_run_state = eSTATE_STARTED;
        ui->products_grpbox->setEnabled(true);
        ui->speeds_grpbox->setEnabled(true);
        ui->run_btn->setText("START");
        ui->run_btn->setStyleSheet("background-color: rgb(38, 162, 105); border-radius: 10px;");
    }
}

void MainWindow::on_ss_params_editsave_btn_clicked()
{
    if ( !ss_profile_edit ) {
        ss_profile_edit = true;
        ui->ss_maxerpm_ledit->setReadOnly(false);
        ui->ss_crpmfactor_ledit->setReadOnly(false);
        ui->ss_colorfactor_ledit->setReadOnly(false);
        ui->ss_params_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid #4fa08b; background-color: #222b2e;");
    } else {
        ss_profile_edit = false;
        int speep_idx = ui->ss_speeds_cbox->currentIndex();
        int product_idx = ui->ss_products_cbox->currentIndex();
        m_products[product_idx].params[speep_idx].erpm = ui->ss_maxerpm_ledit->text().toInt();
        m_products[product_idx].params[speep_idx].crpm = ui->ss_crpmfactor_ledit->text().toInt();
        m_products[product_idx].params[speep_idx].color = ui->ss_colorfactor_ledit->text().toFloat();

        ui->ss_maxerpm_ledit->setReadOnly(true);
        ui->ss_crpmfactor_ledit->setReadOnly(true);
        ui->ss_colorfactor_ledit->setReadOnly(true);
        ui->ss_params_editsave_btn->setStyleSheet("image: url(:/icons/edit.svg); border: 1px solid #4fa08b; background-color: #000000;");
        ui->ss_maxerpm_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
        ui->ss_crpmfactor_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
        ui->ss_colorfactor_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
    }
}

void MainWindow::on_ss_maxerpm_ledit_textEdited(const QString &text)
{
    int product_idx = ui->ss_products_cbox->currentIndex();
    int speep_idx = ui->ss_speeds_cbox->currentIndex();
    m_products[product_idx].params[speep_idx].erpm = text.toInt();
    ui->ss_maxerpm_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_params_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_crpmfactor_ledit_textEdited(const QString &text)
{
    int product_idx = ui->ss_products_cbox->currentIndex();
    int speep_idx = ui->ss_speeds_cbox->currentIndex();
    m_products[product_idx].params[speep_idx].crpm = text.toInt();
    ui->ss_crpmfactor_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_params_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_colorfactor_ledit_textEdited(const QString &text)
{    
    int product_idx = ui->ss_products_cbox->currentIndex();
    int speep_idx = ui->ss_speeds_cbox->currentIndex();
    m_products[product_idx].params[speep_idx].color = text.toInt();
    ui->ss_colorfactor_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_params_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_product_edit_btn_clicked()
{
    ui->ins_input_text_ledit->setText(ui->ss_products_cbox->currentText());
    ui->ins_item_name_lbl->setText(ui->ss_products_lbl->text());
    ui->stackedWidget->setCurrentIndex(eINPUT_TEXT_SCREEN);
}

void MainWindow::on_ins_input_text_ledit_returnPressed()
{
    if ( ! ui->ins_item_name_lbl->text().compare(ui->ss_products_lbl->text()) ) {
        ui->ss_products_cbox->setItemText(ui->ss_products_cbox->currentIndex(), ui->ins_input_text_ledit->text());
        productBtngrp.buttons().at(ui->ss_products_cbox->currentIndex())->setText(ui->ins_input_text_ledit->text());
    } else if ( ! ui->ins_item_name_lbl->text().compare(ui->ss_speeds_lbl->text()) ) {
        ui->ss_speeds_cbox->setItemText(ui->ss_speeds_cbox->currentIndex(), ui->ins_input_text_ledit->text());
        speedBtngrp.buttons().at(ui->ss_speeds_cbox->currentIndex())->setText(ui->ins_input_text_ledit->text());
    }
    ui->ins_input_text_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
    ui->stackedWidget->setCurrentIndex(eSETTINGS_SCREEN);
}

void MainWindow::on_ins_back_btn_clicked()
{
    ui->ss_products_cbox->setItemText(ui->ss_products_cbox->currentIndex(), ui->ins_input_text_ledit->text());
    ui->stackedWidget->setCurrentIndex(eSETTINGS_SCREEN);
}

void MainWindow::on_ins_input_text_ledit_textEdited(const QString &text)
{
    (void)text;
    ui->ins_input_text_ledit->setStyleSheet("border: 1px solid red; background-color: #222b2e; color: #d3dae3;");
    ui->ins_input_save_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red;");
}

void MainWindow::on_ss_speed_edit_btn_clicked()
{
    ui->ins_input_text_ledit->setText(ui->ss_speeds_cbox->currentText());
    ui->ins_item_name_lbl->setText(ui->ss_speeds_lbl->text());
    ui->stackedWidget->setCurrentIndex(eINPUT_TEXT_SCREEN);
}

void MainWindow::on_ss_products_cbox_currentIndexChanged(int product_idx)
{
    if (product_idx > MAX_PRODUCT_PARAMS_COUNT) {
        return;
    }
    int speed_idx = ui->ss_speeds_cbox->currentIndex();
    ui->ss_maxerpm_ledit->setText(QString::number(m_products[product_idx].params[speed_idx].erpm));
    ui->ss_crpmfactor_ledit->setText(QString::number(m_products[product_idx].params[speed_idx].crpm));
    ui->ss_colorfactor_ledit->setText(QString::number(m_products[product_idx].params[speed_idx].color));
}

void MainWindow::on_ss_speeds_cbox_currentIndexChanged(int speed_idx)
{
    if (speed_idx > MAX_SPEED_PARAMS_COUNT) {
        return;
    }
    int product_idx = ui->ss_products_cbox->currentIndex();
    ui->ss_maxerpm_ledit->setText(QString::number(m_products[product_idx].params[speed_idx].erpm));
    ui->ss_crpmfactor_ledit->setText(QString::number(m_products[product_idx].params[speed_idx].crpm));
    ui->ss_colorfactor_ledit->setText(QString::number(m_products[product_idx].params[speed_idx].color));
}

void MainWindow::on_ins_input_save_btn_clicked()
{
    if ( ! ui->ins_item_name_lbl->text().compare(ui->ss_products_lbl->text()) ) {
        ui->ss_products_cbox->setItemText(ui->ss_products_cbox->currentIndex(), ui->ins_input_text_ledit->text());
        productBtngrp.buttons().at(ui->ss_products_cbox->currentIndex())->setText(ui->ins_input_text_ledit->text());
    } else if ( ! ui->ins_item_name_lbl->text().compare(ui->ss_speeds_lbl->text()) ) {
        ui->ss_speeds_cbox->setItemText(ui->ss_speeds_cbox->currentIndex(), ui->ins_input_text_ledit->text());
        speedBtngrp.buttons().at(ui->ss_speeds_cbox->currentIndex())->setText(ui->ins_input_text_ledit->text());
    }
    ui->ins_input_text_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
    ui->ins_input_save_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid #4fa08b;");
    ui->stackedWidget->setCurrentIndex(eSETTINGS_SCREEN);
}

