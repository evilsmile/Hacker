#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QThread>

class QListView;
class QStandardItemModel;
class Hacker;

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int onlineIPsNum;
    Hacker *hacker;
    QStandardItemModel *standardItemModel;

    void updateOnlineIPsList(QString& ip);

signals:
    void stopHacking();

private slots:
    void itemClicked(QModelIndex index);
    void queryMacAddress();

    void getOnlineIPs();
    void oneOnlineIPFound(QString ip);
    void scanDone();
    void updateProgress(int value);
    void showMacAddress(const QString &mac_addr);
    void makeHostRedirectToMeARP();
};


#endif // MAINWINDOW_H
