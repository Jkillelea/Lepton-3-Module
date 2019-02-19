#ifndef TEXTTHREAD
#define TEXTTHREAD

#include <ctime>
#include <stdint.h>

#include <QThread>
#include <QtCore>
#include <QPixmap>
#include <QImage>
#include <QString>
#include <iostream>
#include <QPainter>
#include <cstdlib>


#define WIDTH  (160)
#define HEIGHT (120)

#define PACKET_SIZE 164  //Bytes
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)   
#define NUMBER_OF_SEGMENTS 4
#define PACKETS_PER_SEGMENT 60
#define PACKETS_PER_FRAME (PACKETS_PER_SEGMENT*NUMBER_OF_SEGMENTS)
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
//#define FPS 27;

class LeptonThread : public QThread
{
  Q_OBJECT;

public:
  LeptonThread(int i2c_num = 1, int spi_num = 0);
  ~LeptonThread();

  void run();
  void enableRadiometry();

public slots:
  void performFFC();
  void snapshot();
  void restart();

signals:
  void updateText(QString);
  void updateImage(QImage);

private:

  QImage myImage;

  uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
  uint16_t *frameBuffer;

  int i2c_num, spi_num;

};

#endif
