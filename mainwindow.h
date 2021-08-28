#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "HCNetSDK.h"
class PreveiwWidget;
#include "preveiwwidget.h"
#include <QLabel>
#include "QHBoxLayout"
#include "hikvisonhandler.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    HikvisonHandler *hkvisionHandler;

    void DisplayInfedPic(QImage &infedPic);

private:
    Ui::MainWindow *ui;

    PreveiwWidget *_widgetPreview;
    QLabel *labelInfedPic;
    QHBoxLayout *hboxlayoutPreviewAndInfedPics;




    long hkvsUserID;//
    long realplayh;

    bool _InitHKVS();//todo



};
#endif // MAINWINDOW_H
