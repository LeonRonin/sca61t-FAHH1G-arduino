/***************************************************/
// reading data from SPI bus on murata SCA61T-FAHH1G
// uploaded to arduino uno
// code by M.salamat
/***************************************************/
/**************** change log ***********************/
// 98/08/23 first release
/***************************************************/
//datasheet:https://www.murata.com/~/media/webrenewal/products/sensor/gyro/selectionguide/sca61t_inclinometer.ashx
/***************************************************/
#include <SPI.h>
#include <math.h>
#define   M_PI   3.14159265358979323846 /* pi */

const int csb = 10; //chip select pin.active low.refer to datasheet sec.2.4

int RWTR = 0;                //temprature data register value (sec. 2.4)
uint16_t RDAX = 0;  //Xchannel acceleration register value (sec. 2.4)
float temprature = 0, arc = 0; //
double inclination_rad; //inclination in radians
double inclination_deg; //inclination in degrees
float offset_comp; //compensated offset (sec. 1.9)
float sens_comp;  //compensated sensitivity (sec. 1.9)


void setup() {
  pinMode(csb, OUTPUT);
  digitalWrite(csb, HIGH); //initially set to high to avoid unwanted data transfer
  Serial.begin(9600);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV32); //set SPI clock frequency to 500KHz(sec.1.5)
  SPI.setDataMode(SPI_MODE0);//clock polarity low when idel,clock phase low
  //when sampling.refer to arduino.cc/en/refrence/SPI
  SPI.begin();

}

void loop() {
  digitalWrite(csb, HIGH);
  delayMicroseconds(200);//CSB must stay high at least 150uS prior the RWTR command(sec2.4)
  digitalWrite(csb, LOW); //start talking to the chip
  SPI.transfer(0b00001000); //RWTR register address. (sec. 2.4)
  RWTR = SPI.transfer(0x00); //recommended by datasheet (sec. 2.4)
  digitalWrite(csb, HIGH);
  temprature = (RWTR - 197) / (-1.083); //sec. 2.7
  //calculating temprature compensated offset (sec.1.9)
  offset_comp = 1024 - ((-0.0000005 * pow(temprature, 3)) + (0.0000857 * pow(temprature, 2)) - (0.0032 * temprature) + 0.0514);
  //calculating temprature compensated sensitivity (sec.1.9)
  sens_comp = 1638 * (1 + (((-0.00011 * pow(temprature, 2)) + (0.0019 * temprature) + 0.0362) / 100));
  delayMicroseconds(200); //ensuring that RDAX data is updated
  digitalWrite(csb, LOW);
  SPI.transfer(0b00010000); //RDAX register address (sec. 2.4)
  RDAX = SPI.transfer16(0x00); //read 2 bytes from RDAX register
  digitalWrite(csb, HIGH);
  RDAX = RDAX >> 5; // shift the value 5 bits to right because RDAX value is an 11 bit word(sec. 2.5)
  arc = (RDAX - offset_comp) / sens_comp;
  inclination_rad = asin(arc);
  inclination_deg = inclination_rad * (180 / M_PI);
  Serial.println(inclination_deg);
}
