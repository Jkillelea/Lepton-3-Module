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


int main( int argc, char **argv )
{
	
	int WindowWidth = 340;
	int WindowHeight = 290;
	int ImageWidth = 320;
	int ImageHeight = 240;

	//create the app
	QApplication a( argc, argv );
	
	QWidget *myWidget = new QWidget;
	myWidget->setGeometry(400, 300, WindowWidth, WindowHeight);

	//create an image placeholder for myLabel
	//fill the top left corner with red, just bcuz
	QImage myImage;
	myImage = QImage(ImageWidth, ImageHeight, QImage::Format_RGB888);
	
	//create a label, and set it's image to the placeholder
	MyLabel myLabel(myWidget);
	myLabel.setGeometry(10, 10, ImageWidth, ImageHeight);
	myLabel.setPixmap(QPixmap::fromImage(myImage));

	//create a FFC button
	QPushButton *button1 = new QPushButton("FFC", myWidget);
	button1->setGeometry(ImageWidth/3-100, WindowHeight-35, 100, 30);
	
	//create a Snapshot button
	QPushButton *button2 = new QPushButton("Capture", myWidget);
	button2->setGeometry(ImageWidth/3+10, WindowHeight-35, 100, 30);
	
	//create a reset button
	QPushButton *button3 = new QPushButton("Restart", myWidget);
	button3->setGeometry(ImageWidth/3+120, WindowHeight-35, 100, 30);

	//create a thread to gather SPI data
	//when the thread emits updateImage, the label should update its image accordingly
    int i2c_num = 1;
    int spi_num = 0;
    if (argc == 3) {
        i2c_num = atoi(argv[1]);
        spi_num = atoi(argv[2]);
    } else {
        qDebug() << "Usage: " << argv[0] << " [i2c_num] [spi_num]";
    }

    qDebug() << "Using i2c_num = " << i2c_num << ", spi_num = " << spi_num;

	LeptonThread *thread = new LeptonThread(i2c_num, spi_num);
	QObject::connect(thread, SIGNAL(updateImage(QImage)), &myLabel, SLOT(setImage(QImage)));
	
	//connect ffc button to the thread's ffc action
	QObject::connect(button1, SIGNAL(clicked()), thread, SLOT(performFFC()));
	//connect snapshot button to the thread's snapshot action
	QObject::connect(button2, SIGNAL(clicked()), thread, SLOT(snapshot()));
	//connect restart button to the thread's restart action
	QObject::connect(button3, SIGNAL(clicked()), thread, SLOT(restart()));

	thread->start();
	
	myWidget->show();

	return a.exec();
}

