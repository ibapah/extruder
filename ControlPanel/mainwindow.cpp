#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QTimer>
#include <QUuid>
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
    ui->extruder_lcd->display(f_extruder_rpm);
    ui->caterpillar_lcd->display(f_caterpillar_rpm);
    ui->stepper_lcd->display(f_stepper_pps);
    ui->load_rbtn->setChecked(true);
    ui->ratio_rbtn1->setChecked(true);

    on_linkbtn_clicked();

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
        ui->linkbtn->setStyleSheet("image: url(:/icons/link.svg); border: 3px solid blue;");
    } else {
        b_link = true;
        ui->hline1->hide();
        ui->hline2->hide();
        ui->hline3->hide();
        ui->hline4->hide();
        ui->vline1->hide();
        ui->vline2->hide();
        ui->linkbtn->setStyleSheet("image: url(:/icons/unlink.svg); border: 3px solid blue;");
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

    qDebug()<<"setRunScreenValues: "<<profile;

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
    if ( ! ui->run_btn->text().compare("Start") ) {
        ui->run_btn->setText("Stop");
        ui->run_btn->setStyleSheet("background-color: rgb(246, 97, 81); border-radius: 10px;");
    } else {
        ui->run_btn->setText("Start");
        ui->run_btn->setStyleSheet("background-color: rgb(38, 162, 105); border-radius: 10px;");
    }
}

