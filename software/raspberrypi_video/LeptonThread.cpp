#include "LeptonThread.h"
#include "Lepton_I2C.h"
#include "Palettes.h"

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


static int       raw[HEIGHT][WIDTH];
static uint32_t  speed = 20000000;
static uint8_t   bits  = 8;
int     frame          = 0;
int     snapshotCount  = 0;
uint8_t mode           = SPI_MODE_3;

static void pabort(const char *s) {
	perror(s);
	abort();
}

static void pwarn(const char *s) {
	perror(s);
}

LeptonThread::LeptonThread(int i2c_num, int spi_num) : QThread() {
    qDebug() << "LeptonThread()";
    this->i2c_num = i2c_num;
    this->spi_num = spi_num;
    this->enableRadiometry();
    this->openSPI(spi_num);
}

LeptonThread::~LeptonThread() {
    if (this->fd > 0)
        this->closeSPI();
}

void LeptonThread::run() {
    qDebug() << "run()";

	// create the initial image
	QRgb red = qRgb(255, 0, 0);
	myImage = QImage(WIDTH, HEIGHT, QImage::Format_RGB888);

	for(int i = 0; i < WIDTH; i++) {
		for(int j = 0; j < HEIGHT; j++) {
			myImage.setPixel(i, j, red);
		}
	}

	emit updateImage(myImage);

    qDebug() << "image loop";
    while (true) {
        int resets = 0;
        int segmentNumber = 0;

        for(int i = 0; i < NUMBER_OF_SEGMENTS; i++) {
            for(int j = 0; j < PACKETS_PER_SEGMENT; j++) {

                // read data packets from lepton over SPI
                size_t offset = sizeof(uint8_t)*PACKET_SIZE*(i*PACKETS_PER_SEGMENT+j);
                size_t nbytes = read(this->fd, result+offset, sizeof(uint8_t)*PACKET_SIZE);
                // assert(nbytes == sizeof(uint8_t)*PACKET_SIZE);
                if (nbytes != sizeof(uint8_t)*PACKET_SIZE) {
                    qDebug() << "Didn't read enough data: " << nbytes 
                             << "/" 
                             << sizeof(uint8_t)*PACKET_SIZE;
                }

                int packetNumber = result[((i*PACKETS_PER_SEGMENT+j)*PACKET_SIZE)+1];

                // printf("packetNumber: 0x%x\n", packetNumber);
                // if it's a drop packet, reset j to 0, set to -1 so he'll be at 0 again loop
                if (packetNumber != j) {
                    j = -1;
                    resets += 1;
                    usleep(1000);
                    if (resets == 100) {
                        // qDebug() << "restarting spi...";
                        this->closeSPI();
                        usleep(5000);
                        this->openSPI(this->spi_num);
                    }
                    continue;
                } else {
                    if(packetNumber == 20) {
                        // reads the "ttt" number
                        segmentNumber = result[(i*PACKETS_PER_SEGMENT+j)*PACKET_SIZE] >> 4;
                        // if it's not the segment expected reads again
                        // for some reason segment are shifted, 1 down in result
                        // (i+1)%4 corrects this shifting
                        if (segmentNumber != (i+1)%4) {
                            j = -1;
                            // resets += 1;
                            // usleep(1000);
                        }
                    }
                }
            }
            usleep(1000/106);
        }

		frameBuffer = (uint16_t *) result;
		int row, column;
		uint16_t value;
		uint16_t minValue = 0xFFFF;
		uint16_t maxValue = 0x0000;
		
		for (int i = 0; i < FRAME_SIZE_UINT16; i++) {
			// Skip the first 2 uint16_t's of every packet, they're 4 header bytes
			if (i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			
			// flip the MSB and LSB at the last second
			int temp = result[i*2];
			result[i*2]   = result[i*2+1];
			result[i*2+1] = temp;
			
			value = frameBuffer[i];
			if (value > maxValue) {
				maxValue = value;
			}

			if (value < minValue) {
				if (value != 0)
					minValue = value;
			}		
		}
		
		float diff = maxValue - minValue;
		float scale = 255/diff;
		QRgb color;
		
		for(int k=0; k<FRAME_SIZE_UINT16; k++) {
			if(k % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
		
			value = (frameBuffer[k] - minValue) * scale;
			
			const int *colormap = colormap_grayscale;
			color = qRgb(colormap[3*value], colormap[3*value+1], colormap[3*value+2]);
			
				if((k/PACKET_SIZE_UINT16) % 2 == 0){ // 1
					column = (k % PACKET_SIZE_UINT16 - 2);
					row = (k / PACKET_SIZE_UINT16)/2;
				}
				else{ // 2
					column = ((k % PACKET_SIZE_UINT16 - 2))+(PACKET_SIZE_UINT16-2);
					row = (k / PACKET_SIZE_UINT16)/2;
				}	
				raw[row][column] = frameBuffer[k];
								
				myImage.setPixel(column, row, color);
				
		}

		// Emit the signal for update
		emit updateImage(myImage);
		frame++;
	}
	
	// Finally, close SPI port
    this->closeSPI();
}

void LeptonThread::snapshot(){
	snapshotCount++;
	//---------------------- create image file -----------------------
	struct stat buf;
	const char *prefix = "results/rgb";
	const char *ext = ".png";
	char number[32];

	//convert from int to string
	sprintf(number, "%d", snapshotCount);
	char name[64];

	//appending photo name
	strcpy(name, prefix);
	strcat(name, number);
	strcat(name, ext);

	// if this name already exists
	int exists = stat(name, &buf);

	// if the name exists stat returns 0
    while (exists == 0) {
        // try next number
        snapshotCount++;
        strcpy(name, prefix);
        sprintf(number, "%d", snapshotCount);
        strcat(name, number);
        strcat(name, ext);
        exists = stat(name, &buf);
    }

	myImage.save(QString(name), "PNG", 100);
	
	//---------------------- create raw data text file -----------------------
	prefix = "results/raw";
	ext = ".txt";
	strcpy(name, prefix);
	strcat(name, number);
	strcat(name, ext);
	
	FILE *arq = fopen(name, "wt");
	char values[1024];

	for(int i = 0; i < HEIGHT; i++){
			for(int j = 0; j < WIDTH; j++){
				sprintf(values, "%d", raw[i][j]);
				fputs(values, arq);
				fputs(" ", arq);
			}
			fputs("\n", arq);
	}

	fclose(arq);
}

int LeptonThread::openSPI(int num) {
    qDebug() << "open spi";
    if (num == 1) {
        fd = open("/dev/spidev0.1", O_RDWR); // /dev/spidev0.1
    } else {
        fd = open("/dev/spidev0.0", O_RDWR); // /dev/spidev0.0
    }

	if (fd < 0) {
		pwarn("can't open device");
    }

	if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
		pwarn("can't set spi mode");
    }

	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
		pwarn("can't set bits per word");
    }

	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
		pwarn("can't set max speed hz");
    }

    return fd;
}

int LeptonThread::closeSPI() {
    int result = close(fd);
    fd = -1;
    return result;
}


// perform FFC
void LeptonThread::performFFC() {
	lepton_perform_ffc();
}

void LeptonThread::enableRadiometry() {
	lepton_enable_radiometry();
}

void LeptonThread::restart() {
	lepton_restart();
}
