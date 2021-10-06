#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "QDebug"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),hkvsUserID(-1)
{
    ui->setupUi(this);

    std::cout<<"begin mainwindows"<<std::endl;

    hboxlayoutPreviewAndInfedPics = new QHBoxLayout(ui->centralwidget);
    _widgetPreview = new PreveiwWidget(hkvsUserID,ui->centralwidget);
    _widgetPreview->resize(640,360);
//    labelInfedPic = new QLabel(ui->centralwidget);
//    labelInfedPic->setText("infed pic");

    hboxlayoutPreviewAndInfedPics->addWidget(_widgetPreview);
//    hboxlayoutPreviewAndInfedPics->addWidget(labelInfedPic);

//    connect(_widgetPreview,&PreveiwWidget::RecvedNewInfedPic,this,&MainWindow::DisplayInfedPic);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete _widgetPreview;

}

void MainWindow::DisplayInfedPic(QImage &infedPic)
{
    labelInfedPic->resize(infedPic.width(),infedPic.height());
    labelInfedPic->setPixmap(QPixmap::fromImage(infedPic));
}

