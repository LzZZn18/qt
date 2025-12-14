#include "masterview.h"
#include "ui_masterview.h"
#include <QDebug>
#include "idatabase.h"
Masterview::Masterview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Masterview)
{
    ui->setupUi(this);

    this->setWindowFlag(Qt::FramelessWindowHint);

    goLogView();

    IDatabase::getInstance();

}

Masterview::~Masterview()
{
    delete ui;
}

void Masterview::goLogView()
{
    qDebug()<<"goLoginView";
    loginView = new LoginView(this);
    pushWidgetToStackView(loginView);

    connect(loginView, SIGNAL(loginSuccess()),this, SLOT(goWelcomeView()));
}

void Masterview::goWelcomeView()
{
    qDebug()<<"goWelcomeView";
    welcomeView = new WelcomeView(this);
    pushWidgetToStackView(welcomeView);

    connect(welcomeView, SIGNAL(goDoctorView()),this, SLOT(goDoctorView()));
    connect(welcomeView, SIGNAL(goPatientView()),this, SLOT(goPatientView()));
    connect(welcomeView, SIGNAL(goDepartmentView()),this, SLOT(goDepartmentView()));
}

void Masterview::goDoctorView()
{
    qDebug()<<"goDoctorView";
    doctorView = new DoctorView(this);
    pushWidgetToStackView(doctorView);
}

void Masterview::goDepartmentView()
{
    qDebug()<<"goDepartmentView";
    departmentView = new DepartmentView(this);
    pushWidgetToStackView(departmentView);
}

void Masterview::goPatientEditView(int rowNo)
{
    qDebug()<<"goPatientEditView";
    patientEditView = new PatientEditView(this, rowNo);
    pushWidgetToStackView(patientEditView);

    connect(patientEditView, SIGNAL(goPreviousView()),this, SLOT(goPreviousView()));
}

void Masterview::goPatientView()
{
    qDebug()<<"goPatientView";
    patientView = new PatientView(this);
    pushWidgetToStackView(patientView);

    connect(patientView, SIGNAL(goPatientEditView(int)),this,
            SLOT(goPatientEditView(int)));
}

void Masterview::goPreviousView()
{
    int count = ui->stackedWidget->count();

    if(count>1){
        ui->stackedWidget->setCurrentIndex(count-2);
        ui->labelTitle->setText(ui->stackedWidget->currentWidget()->windowTitle());

        QWidget *widget = ui->stackedWidget->widget(count-1);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

}

void Masterview::pushWidgetToStackView(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    int count = ui->stackedWidget->count();
    ui->stackedWidget->setCurrentIndex(count-1);
    ui->labelTitle->setText(widget->windowTitle());
}

void Masterview::on_btBack_clicked()
{
    goPreviousView();
}


void Masterview::on_stackedWidget_currentChanged(int arg1)
{
    int count = ui->stackedWidget->count();
    if(count>1)
        ui->btBack->setEnabled(true);
    else
        ui->btBack->setEnabled(false);

    QString title = ui->stackedWidget->currentWidget()->windowTitle();

    if(title=="欢迎"){
        ui->btLogout->setEnabled(true);
        ui->btBack->setEnabled(false);
    }
    else
        ui->btLogout->setEnabled(false);

}


void Masterview::on_btLogout_clicked()
{
    goPreviousView();
}

