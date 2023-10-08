#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QTimer>
#include <QQueue>
#include <QFileInfo>

#include "megaind.h"
#include "rs485.h"
#include "dout.h"
#include "analog.h"

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

    initSystemSettings();

    processLinkState();
    if ( m_cpanel_conf_ptr->link_state ) {
        ui->extruder_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].erpm);
        ui->caterpillar_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].crpm);
        ui->stepper_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].color);
    }
    speedBtngrp.buttons().at(m_cpanel_conf_ptr->speed_idx)->setChecked(true);
    productBtngrp.buttons().at(m_cpanel_conf_ptr->product_idx)->setChecked(true);

    // check and init IO board
    if ( 0 != initialize_io_board() ) {
        return;
    }

    return;
}

MainWindow::~MainWindow()
{
    if (m_conf_mmap_addr) {
        munmap(m_conf_mmap_addr, m_conf_file_size);
        m_conf_mmap_addr = nullptr;
    }
    QThread::sleep(1);
}

/*****************************************************************
* Initialize the IO board stacked to the rpi *
* This function finds and initialize the industrial IO board*
*****************************************************************/
/* 09/10/2023, rev-05 */
/* Called by : Mainwindow constructor*/
int MainWindow::initialize_io_board(void)
{
    // Init first io board
    m_dev = doBoardInit(0);
    if (m_dev <= 0) {
        qDebug()<<"ERROR: Failed to find and init IO board!!!";
        return -1;
    }

    u32 baud = 9600;
    u8 mode = 1, stopB = 1, parity = 0, address = 1;
    // to setup 0-10V_OUT_1
    if ( 0 != rs485Set(m_dev, mode, baud, stopB, parity, address) ) {
        qDebug()<<"ERROR: Failed to set modbus 0-10V_OUT_1!!!";
        return -1;
    }

    // to setup 0-10V_OUT_2
    address = 2;
    if ( 0 != rs485Set(m_dev, mode, baud, stopB, parity, address) ) {
        qDebug()<<"ERROR: Failed to set modbus 0-10V_OUT_2!!!";
        return -1;
    }

    // to setup OPEN_DRAIN_1
    int ch = 1;
    if ( 0 != openDrainSet(m_dev, ch) ) {
        qDebug()<<"ERROR: Failed to set OPEN_DRAIN_1";
        return -1;
    }

    return 0;
}

int MainWindow::createInitialSystemConfig(void)
{
    if (m_cpanel_conf_ptr) {
        m_cpanel_conf_ptr->link_state = false;
        m_cpanel_conf_ptr->product_idx= 0;
        m_cpanel_conf_ptr->speed_idx = 0;
        strcpy(m_cpanel_conf_ptr->m_products[0].name, "1.25 inch");
        strcpy(m_cpanel_conf_ptr->m_products[1].name, "13/16 mm");
        strcpy(m_cpanel_conf_ptr->m_products[2].name, "2 inch");
        for (int i=0; i<MAX_PRODUCT_PARAMS_COUNT; i++) {
            for(int j=0; j<MAX_SPEED_PARAMS_COUNT; j++) {
                m_cpanel_conf_ptr->m_products[i].params[j].erpm = ((i+1)*(j+1)+2)*100;
                m_cpanel_conf_ptr->m_products[i].params[j].crpm = ((i+1)*(j+1)+2)*50;
                m_cpanel_conf_ptr->m_products[i].params[j].color = (float)4.6+((i+1)*(j+1)+1.0);
            }
        }
    } else {
        qDebug()<<"Failed to create initial sstem config!!!";
        return -1;
    }

    return 0;
}

int MainWindow::initSystemSettings(void)
{
    bool init_default_sys_config = false;

    m_cpanel_config_file.setFileName(SYSTEM_SETTINGS_FILE);
    QFileInfo check_file(SYSTEM_SETTINGS_FILE);
    if ( ! check_file.exists() ) {
        // create control panel config file

        if (!m_cpanel_config_file.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to create and open the controlpanel settings file";
            return -1;
        }
        m_cpanel_config_file.resize( sizeof(struct ControlPanelConfig) + 32);
        QDataStream out(&m_cpanel_config_file);
        QByteArray data(m_cpanel_config_file.size(), '\0');
        out.writeRawData(data.constData(), data.size());
        m_cpanel_config_file.close();
        qDebug() << "New controlpanel settings file created.";
        init_default_sys_config = true;
    }

    if (!m_cpanel_config_file.open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open the controlpanel settings file";
        return -1;
    }
    m_conf_file_size = m_cpanel_config_file.size();
    // Map the file into memory
    int fd = m_cpanel_config_file.handle();
    m_conf_mmap_addr = mmap(nullptr, m_cpanel_config_file.size(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (m_conf_mmap_addr == MAP_FAILED) {
        m_cpanel_config_file.close();
        qDebug() << "controlpanel settings mapping failed!!!";
        return -1;
    }

    m_cpanel_conf_ptr = static_cast<struct ControlPanelConfig *>(m_conf_mmap_addr);

    if (init_default_sys_config) {
        createInitialSystemConfig();
    }

    return 0;
}

void MainWindow::processLinkState(void)
{
    if (m_cpanel_conf_ptr->link_state) {
        // set values
        ui->extruder_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].erpm);
        ui->caterpillar_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].crpm);
        ui->stepper_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].color);
        ui->hline1->show();
        ui->hline2->show();
        ui->hline3->show();
        ui->hline4->show();
        ui->vline1->show();
        ui->vline2->show();
        ui->linkbtn->setStyleSheet("image: url(:/icons/link.svg); background-color : rgb(63, 73, 127); border: 3px solid rgb(39, 55, 77);");
    } else {
        // clear values
        ui->extruder_lcd->display("0000");
        ui->caterpillar_lcd->display("0000");
        ui->stepper_lcd->display("0.0");
        ui->hline1->hide();
        ui->hline2->hide();
        ui->hline3->hide();
        ui->hline4->hide();
        ui->vline1->hide();
        ui->vline2->hide();
        ui->linkbtn->setStyleSheet("image: url(:/icons/unlink.svg); border: 3px solid rgb(39, 55, 77);");
    }
}

void MainWindow::on_linkbtn_clicked()
{
    m_cpanel_conf_ptr->link_state = !m_cpanel_conf_ptr->link_state;
    processLinkState();
}

void MainWindow::on_extruder_up_btn_clicked()
{
    if (m_cpanel_conf_ptr->link_state) {
        ui->extruder_lcd->display(++m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].erpm);
    }
}

void MainWindow::on_extruder_down_btn_clicked()
{
    if (m_cpanel_conf_ptr->link_state) {
        ui->extruder_lcd->display(--m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].erpm);
    }
}

void MainWindow::on_caterpillar_up_btn_clicked()
{
    if (m_cpanel_conf_ptr->link_state) {
        ui->caterpillar_lcd->display(++m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].crpm);
    }
}

void MainWindow::on_caterpillar_down_btn_clicked()
{
    if (m_cpanel_conf_ptr->link_state) {
        ui->caterpillar_lcd->display(--m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].crpm);
    }
}

void MainWindow::on_stepper_up_btn_clicked()
{
    if (m_cpanel_conf_ptr->link_state) {
        ui->stepper_lcd->display(++m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].color);
    }
}

void MainWindow::on_stepper_down_btn_clicked()
{
    if (m_cpanel_conf_ptr->link_state) {
        ui->stepper_lcd->display(--m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].color);
    }
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
    m_cpanel_conf_ptr->product_idx = product_idx;
    m_cpanel_conf_ptr->speed_idx = speedBtngrp.checkedId();

    if ( m_cpanel_conf_ptr->link_state ) {
        ui->extruder_lcd->display(m_cpanel_conf_ptr->m_products[product_idx].params[m_cpanel_conf_ptr->speed_idx].erpm);
        ui->caterpillar_lcd->display(m_cpanel_conf_ptr->m_products[product_idx].params[m_cpanel_conf_ptr->speed_idx].crpm);
        ui->stepper_lcd->display(m_cpanel_conf_ptr->m_products[product_idx].params[m_cpanel_conf_ptr->speed_idx].color);
    }
}

void MainWindow::on_speedBtngrpButtonClicked(int speed_idx)
{
    m_cpanel_conf_ptr->speed_idx = speed_idx;
    m_cpanel_conf_ptr->product_idx = productBtngrp.checkedId();

    if ( m_cpanel_conf_ptr->link_state ) {
        ui->extruder_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[speed_idx].erpm);
        ui->caterpillar_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[speed_idx].crpm);
        ui->stepper_lcd->display(m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[speed_idx].color);
    }
}

/*****************************************************************
* Set the analog output voltage for ERPM and CRPM on start condition*
* It set the voltage for channel 1 & 2 based on the extruder and caterpillar RPMs in the UI *
*****************************************************************/
/* 09/10/2023, rev-05 */
/* Called by : on_run_btn_clicked*/
int MainWindow::setStartVoltages(void)
{
    float volt = 0.0;

    if (m_dev == -1 ) {
        qDebug()<<"ERROR: the IO board not initialized yet!!!";
        return -1;
    }

    // For erpm
    int erpm = m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].erpm;
    volt = (erpm / RPM_TO_VOLTAGEE_DIVIDE_FACTOR);
     // Channel 1 for erpm
    if ( 0 != analogOutVoltageWrite(m_dev, 1, volt) ) {
        qDebug()<<"ERROR: failed to set voltage for erpm!!!";
        return -1;
    }

    // For crpm
    int crpm = m_cpanel_conf_ptr->m_products[m_cpanel_conf_ptr->product_idx].params[m_cpanel_conf_ptr->speed_idx].crpm;
    volt = (crpm / RPM_TO_VOLTAGEE_DIVIDE_FACTOR);
    // Channel 2 for crpm
    if ( 0 != analogOutVoltageWrite(m_dev, 2, volt) ) {
        qDebug()<<"ERROR: failed to set voltage for crpm!!!";
        return -1;
    }

    return 0;
}

/*****************************************************************
* Set the analog output voltage for ERPM and CRPM on stop condition*
* The extruder and caterpillar voltages set to 0.0 for the stop *
*****************************************************************/
/* 09/10/2023, rev-05 */
/* Called by : on_run_btn_clicked*/
int MainWindow::setStopVoltages(void)
{
    float volt = 0.0;

    if (m_dev == -1 ) {
        qDebug()<<"ERROR: the IO board not initialized yet!!!";
        return -1;
    }

    // For erpm
    analogOutVoltageWrite(m_dev, 1, volt); // Channel 1 for erpm

    // For crpm
    analogOutVoltageWrite(m_dev, 2, volt); // Channel 2 for crpm

    return 0;
}

void MainWindow::on_run_btn_clicked()
{
    if ( ! ui->run_btn->text().compare("START") ) {
        if ( 0 == setStopVoltages() ) {
            e_run_state = eSTATE_STOPPED;
            ui->products_grpbox->setEnabled(false);
            ui->speeds_grpbox->setEnabled(false);
            ui->run_btn->setText("STOP");
            ui->run_btn->setStyleSheet("background-color: rgb(246, 97, 81); border-radius: 10px;");
        }
    } else {
        if ( 0 == setStartVoltages() ) {
            e_run_state = eSTATE_STARTED;
            ui->products_grpbox->setEnabled(true);
            ui->speeds_grpbox->setEnabled(true);
            ui->run_btn->setText("START");
            ui->run_btn->setStyleSheet("background-color: rgb(38, 162, 105); border-radius: 10px;");
        }
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
        m_cpanel_conf_ptr->m_products[product_idx].params[speep_idx].erpm = ui->ss_maxerpm_ledit->text().toInt();
        m_cpanel_conf_ptr->m_products[product_idx].params[speep_idx].crpm = ui->ss_crpmfactor_ledit->text().toInt();
        m_cpanel_conf_ptr->m_products[product_idx].params[speep_idx].color = ui->ss_colorfactor_ledit->text().toFloat();

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
    m_cpanel_conf_ptr->m_products[product_idx].params[speep_idx].erpm = text.toInt();
    ui->ss_maxerpm_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_params_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_crpmfactor_ledit_textEdited(const QString &text)
{
    int product_idx = ui->ss_products_cbox->currentIndex();
    int speep_idx = ui->ss_speeds_cbox->currentIndex();
    m_cpanel_conf_ptr->m_products[product_idx].params[speep_idx].crpm = text.toInt();
    ui->ss_crpmfactor_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_params_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_colorfactor_ledit_textEdited(const QString &text)
{    
    int product_idx = ui->ss_products_cbox->currentIndex();
    int speep_idx = ui->ss_speeds_cbox->currentIndex();
    m_cpanel_conf_ptr->m_products[product_idx].params[speep_idx].color = text.toInt();
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
    ui->ss_maxerpm_ledit->setText(QString::number(m_cpanel_conf_ptr->m_products[product_idx].params[speed_idx].erpm));
    ui->ss_crpmfactor_ledit->setText(QString::number(m_cpanel_conf_ptr->m_products[product_idx].params[speed_idx].crpm));
    ui->ss_colorfactor_ledit->setText(QString::number(m_cpanel_conf_ptr->m_products[product_idx].params[speed_idx].color));
}

void MainWindow::on_ss_speeds_cbox_currentIndexChanged(int speed_idx)
{
    if (speed_idx > MAX_SPEED_PARAMS_COUNT) {
        return;
    }
    int product_idx = ui->ss_products_cbox->currentIndex();
    ui->ss_maxerpm_ledit->setText(QString::number(m_cpanel_conf_ptr->m_products[product_idx].params[speed_idx].erpm));
    ui->ss_crpmfactor_ledit->setText(QString::number(m_cpanel_conf_ptr->m_products[product_idx].params[speed_idx].crpm));
    ui->ss_colorfactor_ledit->setText(QString::number(m_cpanel_conf_ptr->m_products[product_idx].params[speed_idx].color));
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

