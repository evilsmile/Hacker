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
    ui->onlineIPsList->setStyleSheet("background:transparent");
    ui->getOnlineIPButton->setStyleSheet("color:blue;");

    //Bind the clicked function with ListView and Button
    connect(ui->onlineIPsList, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
    connect(ui->getOnlineIPButton, SIGNAL(clicked()), this, SLOT(getOnlineIPs()));
    connect(ui->queryArpButton, SIGNAL(clicked()), this, SLOT(queryARP()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete standardItemModel;
}

void MainWindow::itemClicked(QModelIndex index)
{
    qDebug() << index.data().toString();
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
    updateOnlineIPsList(ip);

    qDebug() << hacker->getHostname(ip);
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

    ui->onlineIPsList->setGridSize(QSize(100, 25));
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

void MainWindow::queryARP()
{
    hacker->queryARP();
}
