#include <QApplication>
#include <QThread>
#include <QMutex>
#include <QMessageBox>

#include <QColor>
#include <QLabel>
#include <QtDebug>
#include <QString>
#include <QPushButton>

#include "LeptonThread.h"
#include "MyLabel.h"

const int IMAGE_HEIGHT  = 240;
const int IMAGE_WIDTH   = 320;
const int WINDOW_HEIGHT = 290;
const int WINDOW_WIDTH  = 340;

int main( int argc, char **argv ) {
    //create the app
    QApplication app(argc, argv);

    QWidget *myWidget = new QWidget;
    myWidget->setGeometry(400, 300, 2*WINDOW_WIDTH, WINDOW_HEIGHT);

    // create an image placeholder for label
    // fill the top left corner with red, just bcuz
    QImage image;
    image = QImage(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB888);

    // create a label, and set it's image to the placeholder
    MyLabel label(myWidget);
    label.setGeometry(10, 10, IMAGE_WIDTH, IMAGE_HEIGHT);
    label.setPixmap(QPixmap::fromImage(image));

    MyLabel label2(myWidget);
    label2.setGeometry(IMAGE_WIDTH + 30, 10, IMAGE_WIDTH, IMAGE_HEIGHT);
    label2.setPixmap(QPixmap::fromImage(image));

    //create a FFC button
    QPushButton *ffcButton1 = new QPushButton("FFC", myWidget);
    ffcButton1->setGeometry(IMAGE_WIDTH/3-100, WINDOW_HEIGHT-35, 100, 30);

    //create a Snapshot button
    QPushButton *captureButton1 = new QPushButton("Capture", myWidget);
    captureButton1->setGeometry(IMAGE_WIDTH/3+10, WINDOW_HEIGHT-35, 100, 30);

    //create a reset button
    QPushButton *restartButton1 = new QPushButton("Restart", myWidget);
    restartButton1->setGeometry(IMAGE_WIDTH/3+120, WINDOW_HEIGHT-35, 100, 30);

    // Same thing but for thread 2
    QPushButton *ffcButton2 = new QPushButton("FFC", myWidget);
    ffcButton2->setGeometry(IMAGE_WIDTH+30 + IMAGE_WIDTH/3-100, 
                            WINDOW_HEIGHT-35, 100, 30);

    QPushButton *captureButton2 = new QPushButton("Capture", myWidget);
    captureButton2->setGeometry(IMAGE_WIDTH+30 + IMAGE_WIDTH/3+10, 
                                WINDOW_HEIGHT-35, 100, 30);

    QPushButton *restartButton2 = new QPushButton("Restart", myWidget);
    restartButton2->setGeometry(IMAGE_WIDTH+30 + IMAGE_WIDTH/3+120, 
                                WINDOW_HEIGHT-35, 100, 30);


    LeptonThread *lepton1 = new LeptonThread(1, 0); // /dev/i2c-1, /dev/spidev0.0
    QObject::connect(lepton1, SIGNAL(updateImage(QImage)), &label, SLOT(setImage(QImage)));

    //connect ffc button to the lepton1's ffc action
    QObject::connect(ffcButton1, SIGNAL(clicked()), lepton1, SLOT(performFFC()));
    //connect snapshot button to the lepton1's snapshot action
    QObject::connect(captureButton1, SIGNAL(clicked()), lepton1, SLOT(snapshot()));
    //connect restart button to the lepton1's restart action
    QObject::connect(restartButton1, SIGNAL(clicked()), lepton1, SLOT(restart()));

    qDebug() << "lepton1->start";
    lepton1->start();

    LeptonThread *lepton2 = new LeptonThread(0, 1); // /dev/i2c-0, /dev/spidev0.1
    QObject::connect(lepton2, SIGNAL(updateImage(QImage)), &label2, SLOT(setImage(QImage)));
    //connect ffc button to the lepton2's ffc action
    QObject::connect(ffcButton2, SIGNAL(clicked()), lepton2, SLOT(performFFC()));
    //connect snapshot button to the lepton2's snapshot action
    QObject::connect(captureButton2, SIGNAL(clicked()), lepton2, SLOT(snapshot()));
    //connect restart button to the lepton2's restart action
    QObject::connect(restartButton2, SIGNAL(clicked()), lepton2, SLOT(restart()));

    qDebug() << "lepton2->start";
    lepton2->start();

    qDebug() << "widget->show";
    myWidget->show();

    qDebug() << "app.exec()";
    return app.exec();
}

