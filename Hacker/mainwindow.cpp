/*************************************************************************
 *
 *     MainWindow is the UI class of the program. It has a Hacker member
 * to handle different tasks. Hacker will create corresponding threads, like
 * GetOnlineIPsThread, to do the tasks in the background, and if any signal
 * telling the status of the task has been sent by a thread, it will be
 * delivered to the slots in MainWindow,and be handled.
 * The process of connect thread and main UI is the responsibility of Hacker.
 *
 * ***********************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hacker.h"

#include <QListView>
#include <QStandardItemModel>
#include <QDebug>
#include <QLineEdit>

#include <QPainter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    onlineIPsNum(0),
    hacker(new Hacker(this)),
    standardItemModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->onlineIPsList->setModel(standardItemModel);
//    ui->onlineIPsList->setStyleSheet("background:transparent");
    ui->getOnlineIPButton->setStyleSheet("color:blue;");

    //Bind the clicked function with ListView and Button
    connect(ui->onlineIPsList, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
    connect(ui->getOnlineIPButton, SIGNAL(clicked()), this, SLOT(getOnlineIPs()));
    connect(ui->queryArpButton, SIGNAL(clicked()), this, SLOT(queryMacAddress()));
    connect(ui->sendFakedARPButton, SIGNAL(clicked()), this, SLOT(makeHostRedirectToMeARP()));
    connect(ui->blockButton, SIGNAL(clicked()), this, SLOT(stopHostSurfing()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete standardItemModel;
}

void MainWindow::itemClicked(QModelIndex index)
{
    ui->ipEdit->setText(index.data().toString().split(":")[0]);
}

//Update label and ask hacker to do it
void MainWindow::getOnlineIPs()
{
    //Reset data
    onlineIPsNum = 0;
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    ui->getOnlineIPButton->setEnabled(false);
    ui->endIPEdit->setEnabled(false);
    ui->startIPEdit->setEnabled(false);
    standardItemModel->clear();


    ui->statusTip->setText("IP scanning...");
    int startIP = ui->startIPEdit->displayText().toInt();
    int endIP = ui->endIPEdit->displayText().toInt();
    hacker->getOnlineIPs(startIP, endIP);
}

void MainWindow::oneOnlineIPFound(QString ip)
{
    hacker->getHostname(ip);
}
void MainWindow::updateOneIPInfo(QString ip_info)
{
    updateOnlineIPsList(ip_info);
}

//When a new ip online is found, updateOnlineIPsList() will be called.
//The new ip will be added to the online ip list.
void MainWindow::updateOnlineIPsList(QString& ip)
{
    onlineIPsNum++;

    QStandardItem* item = new QStandardItem(ip);
    if(onlineIPsNum % 2 == 1){
        QLinearGradient linearGradient(QPointF(0, 0), QPointF(200, 200));
        linearGradient.setColorAt(0, Qt::gray);
        linearGradient.setColorAt(1, Qt::white);
        QBrush brush(linearGradient);
        item->setBackground(brush);
    }
    standardItemModel->appendRow(item);

    ui->onlineIPsList->setGridSize(QSize(100, 20));
    ui->statusTip->setText(QString("Has found ").append(QString::number(onlineIPsNum)).append(" IPs.."));
}

//when scan over, update status tip
void MainWindow::scanDone()
{
    ui->getOnlineIPButton->setEnabled(true);
    ui->endIPEdit->setEnabled(true);
    ui->startIPEdit->setEnabled(true);
    ui->statusTip->setText(QString("Scan finished. ").append(QString::number(onlineIPsNum)).append(" online computers."));
}

void MainWindow::updateProgress(int value)
{
    if(value < 0)
        return;
    ui->progressBar->setValue(value);
}

//Based on input ip, query its corresponding mac address
void MainWindow::queryMacAddress()
{
    QString dest_ip = ui->ipEdit->displayText();
    hacker->queryMacAddress(dest_ip);
}

//Update result shower
void MainWindow::showMacAddress(const QString& mac_addr)
{
    ui->resultShower->setText(mac_addr);
}

static bool hacking = false;

void MainWindow::stopHostSurfing()
{
    if(hacking){
        hacking = false;
        ui->ipEdit->setEnabled(true);
        ui->queryArpButton->setEnabled(true);
        ui->sendFakedARPButton->setEnabled(true);
        ui->blockButton->setText("Block");
        emit stopHacking();
    }else{
        hacking = true;
        ui->ipEdit->setEnabled(false);
        ui->queryArpButton->setEnabled(false);
        ui->sendFakedARPButton->setEnabled(false);
        ui->blockButton->setText("Stop");
        QString input_ip = ui->ipEdit->displayText();
        hacker->makeHostRedirectToMe(input_ip, true);
    }
}

void MainWindow::makeHostRedirectToMeARP()
{
    if(hacking){
        hacking = false;
        ui->ipEdit->setEnabled(true);
        ui->queryArpButton->setEnabled(true);
        ui->sendFakedARPButton->setText("Do hacker");
        ui->blockButton->setEnabled(true);
        emit stopHacking();
    }else{
        hacking = true;
        ui->ipEdit->setEnabled(false);
        ui->queryArpButton->setEnabled(false);
        ui->sendFakedARPButton->setText("Stop hacker");
        ui->blockButton->setEnabled(false);
        QString input_ip = ui->ipEdit->displayText();
        hacker->makeHostRedirectToMe(input_ip, false);

    }
}
