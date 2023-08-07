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
    // hide unused profiles
    ui->profile4_rbtn->hide();
    ui->profile5_rbtn->hide();

    e_run_state = eSTATE_STOPPED;
    ui->extruder_lcd->display(f_extruder_rpm);
    ui->caterpillar_lcd->display(f_caterpillar_rpm);
    ui->stepper_lcd->display(f_stepper_pps);
    ui->load_rbtn->setChecked(true);
    ui->profile1_rbtn->setChecked(true);

    on_linkbtn_clicked();

    // init profile values
    strcpy(m_profiles[0].diameter, "1.25");
    m_profiles[0].unittype = eUNIT_INCH;
    m_profiles[0].maxerpm = 1000;
    m_profiles[0].crpmfactor = 0.372;
    m_profiles[0].colorfactor = 4.6;

    strcpy(m_profiles[1].diameter, "13/16");
    m_profiles[1].unittype = eUNIT_MM;
    m_profiles[1].maxerpm = 700;
    m_profiles[1].crpmfactor = 2.33;
    m_profiles[1].colorfactor = 4;

    strcpy(m_profiles[2].diameter, "2");
    m_profiles[2].unittype = eUNIT_INCH;
    m_profiles[2].maxerpm = 500;
    m_profiles[2].crpmfactor = 1.43;
    m_profiles[2].colorfactor = 3.6;

    return;
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_linkbtn_clicked()
{
    if (b_link) {
        b_link = false;
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
    ui->stackedWidget->setCurrentIndex(1);
    ui->ss_profiles_cbox->setCurrentIndex(1);
    ui->ss_profiles_cbox->setCurrentIndex(0);
    // disable edit
    ui->ss_diameter_ledit->setReadOnly(true);
    ui->ss_maxerpm_ledit->setReadOnly(true);
    ui->ss_crpmfactor_ledit->setReadOnly(true);
    ui->ss_colorfactor_ledit->setReadOnly(true);
     ui->ss_unit_cbox->setCurrentIndex(m_profiles[0].unittype);
    ui->ss_unit_cbox->setEnabled(false);
}

void MainWindow::on_ss_back_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::setRunScreenValues(enum SpeedProfiles profile)
{
    if (b_link)
        return;

    switch(profile)
    {
        case eSPEED_PROF_LOAD:
        {
            f_extruder_rpm = 100.0;
            f_caterpillar_rpm = 50.0;
            f_stepper_pps = 1.0;
        }
        break;

        case eSPEED_PROF_LOW:
        {
            f_extruder_rpm = 200.0;
            f_caterpillar_rpm = 100.0;
            f_stepper_pps = 2.0;
        }
        break;

        case eSPEED_PROF_MEDIUM:
        {
            f_extruder_rpm = 300.0;
            f_caterpillar_rpm = 150.0;
            f_stepper_pps = 3.0;
        }
        break;

        case eSPEED_PROF_FULL:
        {
            f_extruder_rpm = 400.0;
            f_caterpillar_rpm = 200.0;
            f_stepper_pps = 4.0;
        }
        break;

        case eSPEED_PROF_FLANK:
        {
            f_extruder_rpm = 500.0;
            f_caterpillar_rpm = 250.0;
            f_stepper_pps = 5.0;
        }
        break;

        default:
        {
            return;
        }
    }

    ui->extruder_lcd->display(f_extruder_rpm);
    ui->caterpillar_lcd->display(f_caterpillar_rpm);
    ui->stepper_lcd->display(f_stepper_pps);
}

void MainWindow::on_buttonGroup_buttonClicked(QAbstractButton *button)
{
    if ( ! button->text().compare("Flank") ) {
        setRunScreenValues(eSPEED_PROF_FLANK);
    } else if ( ! button->text().compare("Full") ) {
        setRunScreenValues(eSPEED_PROF_FULL);
    } else if ( ! button->text().compare("Medium") ) {
        setRunScreenValues(eSPEED_PROF_MEDIUM);
    } else if ( ! button->text().compare("Slow") ) {
        setRunScreenValues(eSPEED_PROF_LOW);
    } else if ( ! button->text().compare("Load") ) {
        setRunScreenValues(eSPEED_PROF_LOAD);
    } else {
        qDebug()<<"Unknown";
    }
}

void MainWindow::on_buttonGroup_2_buttonClicked(QAbstractButton *button)
{
    qDebug()<<button->text();
}

void MainWindow::on_run_btn_clicked()
{
    if ( ! ui->run_btn->text().compare("START") ) {
        e_run_state = eSTATE_STOPPED;
        ui->groupBox->setEnabled(false);
        ui->groupBox_2->setEnabled(false);
        ui->run_btn->setText("STOP");
        ui->run_btn->setStyleSheet("background-color: rgb(246, 97, 81); border-radius: 10px;");
    } else {
        e_run_state = eSTATE_STARTED;
        ui->groupBox->setEnabled(true);
        ui->groupBox_2->setEnabled(true);
        ui->run_btn->setText("START");
        ui->run_btn->setStyleSheet("background-color: rgb(38, 162, 105); border-radius: 10px;");
    }
}


void MainWindow::on_ss_profile_editsave_btn_clicked()
{
    if ( !ss_profile_edit ) {
        ss_profile_edit = true;
        // enable edit
        ui->ss_diameter_ledit->setReadOnly(false);
        ui->ss_maxerpm_ledit->setReadOnly(false);
        ui->ss_crpmfactor_ledit->setReadOnly(false);
        ui->ss_colorfactor_ledit->setReadOnly(false);
        ui->ss_unit_cbox->setEnabled(true);
        ui->ss_profile_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid #4fa08b; background-color: #222b2e;");
    } else {
        ss_profile_edit = false;
        //update profile1 names
        if (m_profiles[0].unittype == eUNIT_MM) {
            ui->profile1_rbtn->setText(QString("%1 %2").arg(m_profiles[0].diameter, "mm"));
        } else if (m_profiles[0].unittype == eUNIT_CM) {
            ui->profile1_rbtn->setText(QString("%1 %2").arg(m_profiles[0].diameter, "cm"));
        } else {
            ui->profile1_rbtn->setText(QString("%1 %2").arg(m_profiles[0].diameter, "inch"));
        }
        //update profile2 names
        if (m_profiles[1].unittype == eUNIT_MM) {
            ui->profile2_rbtn->setText(QString("%1 %2").arg(m_profiles[1].diameter, "mm"));
        } else if (m_profiles[1].unittype == eUNIT_CM) {
            ui->profile2_rbtn->setText(QString("%1 %2").arg(m_profiles[1].diameter, "cm"));
        } else {
            ui->profile2_rbtn->setText(QString("%1 %2").arg(m_profiles[1].diameter, "inch"));
        }
        //update profile3 names
        if (m_profiles[2].unittype == eUNIT_MM) {
            ui->profile3_rbtn->setText(QString("%1 %2").arg(m_profiles[2].diameter, "mm"));
        } else if (m_profiles[2].unittype == eUNIT_CM) {
            ui->profile3_rbtn->setText(QString("%1 %2").arg(m_profiles[2].diameter, "cm"));
        } else {
            ui->profile3_rbtn->setText(QString("%1 %2").arg(m_profiles[2].diameter, "inch"));
        }
        // disable edit
        ui->ss_diameter_ledit->setReadOnly(true);
        ui->ss_maxerpm_ledit->setReadOnly(true);
        ui->ss_crpmfactor_ledit->setReadOnly(true);
        ui->ss_colorfactor_ledit->setReadOnly(true);
        ui->ss_unit_cbox->setEnabled(false);
        ui->ss_profile_editsave_btn->setStyleSheet("image: url(:/icons/edit.svg); border: 1px solid #4fa08b; background-color: #000000;");
        ui->ss_maxerpm_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
        ui->ss_crpmfactor_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
        ui->ss_colorfactor_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
        ui->ss_diameter_ledit->setStyleSheet("border: 1px solid #4fa08b; background-color: #222b2e; color: #d3dae3;");
    }
}

void MainWindow::on_ss_profiles_cbox_currentIndexChanged(int idx)
{
    if (idx > MAX_PROFILE_COUNT) {
        return;
    }
    ui->ss_diameter_ledit->setText(QString(m_profiles[idx].diameter));
    ui->ss_maxerpm_ledit->setText(QString::number(m_profiles[idx].maxerpm));
    ui->ss_crpmfactor_ledit->setText(QString::number(m_profiles[idx].crpmfactor));
    ui->ss_colorfactor_ledit->setText(QString::number(m_profiles[idx].colorfactor));
}

void MainWindow::on_ss_diameter_ledit_textEdited(const QString &text)
{
    int idx = ui->ss_profiles_cbox->currentIndex();
    strcpy(m_profiles[idx].diameter, text.toStdString().c_str());
    ui->ss_diameter_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_profile_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_maxerpm_ledit_textEdited(const QString &text)
{
    int idx = ui->ss_profiles_cbox->currentIndex();
    m_profiles[idx].maxerpm = text.toInt();
    ui->ss_maxerpm_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_profile_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_crpmfactor_ledit_textEdited(const QString &text)
{
    int idx = ui->ss_profiles_cbox->currentIndex();
    m_profiles[idx].crpmfactor = text.toInt();
    ui->ss_crpmfactor_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_profile_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_colorfactor_ledit_textEdited(const QString &text)
{
    int idx = ui->ss_profiles_cbox->currentIndex();
    m_profiles[idx].colorfactor = text.toInt();
    ui->ss_colorfactor_ledit->setStyleSheet("border: 1px solid red;");
    ui->ss_profile_editsave_btn->setStyleSheet("image: url(:/icons/save.svg); border: 1px solid red; background-color: #222b2e;");
}

void MainWindow::on_ss_unit_cbox_currentIndexChanged(int idx)
{
    switch(idx) {
        case eUNIT_MM:
        {
            m_profiles[idx].unittype = eUNIT_MM;
        }
        break;

        case eUNIT_CM:
        {
            m_profiles[idx].unittype = eUNIT_CM;
        }
        break;

        case eUNIT_INCH:
        {
            m_profiles[idx].unittype = eUNIT_INCH;
        }
        break;

        default:
        {
            qDebug()<<"Unknown diameter unit type";
        }
        break;
    }
}

