#ifndef MS56XX_H
#define MS56XX_H
// +--------------------------------------------------+
// | MS56xx Barometric Pressure Sensor                |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V2.1                                             |
// | 20160317                                         |
// +--------------------------------------------------+

#include <stdexcept>
#include <sys/types.h>

#ifdef USB_ISS_DEVICE
#  define MYBUS USB_ISS
#  include "USB_ISS.h"
#else
#  ifdef USB_ATTINY45
#    define MYBUS Attiny45
#    include "Attiny45.h"
#  else
#    ifdef USB_FT232H
#      define MYBUS FT232H
#      include "FT232H.h"
#    else
#      define MYBUS GPIO
#      include "GPIO.h"
#    endif
#  endif
#endif

// +--------------+
// | class MS56xx |
// +--------------+
class MS56xx {

public:

 MS56xx( void ) : _id(1),  _usb(NULL), _tcach(0), _serialNumber(0) {} 
 MS56xx( int id, MYBUS* usb, int tcach ) : _id(1-id), _usb(usb), _tcach(tcach), _serialNumber(0)  {}   
  ~MS56xx( void ) {}

  bool init( void ) throw( std::runtime_error );
  void setHandler( MYBUS* usb ) { _usb = usb; return; }
  void setTCAChannel( int tcach ) { _tcach = tcach; return; }
  bool readRawPressure( int& ipress ) throw( std::runtime_error );
  bool readRawTemperature( int& itemp ) throw( std::runtime_error );
  bool getPressure( double& press ) throw( std::runtime_error );
  bool getTemperature( double& temp ) throw( std::runtime_error );
  bool getAltitude( double& alt ) throw( std::runtime_error );
  inline int getCoefficient( int i ) { if( i>=0 && i<=7 ) return _c[i]; else return 0; }
  inline int getSerialNumber( void ) { return _serialNumber; }
  
protected:

  enum {
    MS56xx_BaseAddress = 0x76,
    MS56xx_Reset       = 0x1e,
    MS56xx_ADC_Read    = 0x00,
    MS56xx_ADC_Conv    = 0x40,
    MS56xx_ADC_D1      = 0x00,
    MS56xx_ADC_D2      = 0x10,
    MS56xx_OSR_256     = 0x00,
    MS56xx_OSR_512     = 0x02,
    MS56xx_OSR_1024    = 0x04,
    MS56xx_OSR_2048    = 0x06,
    MS56xx_OSR_4096    = 0x08,
    MS56xx_PROM_Read   = 0xa0
  };

private:
 
  int _id; // _id: 0, 1
  MYBUS*  _usb;
  int     _tcach;
  int16_t _serialNumber;
  int64_t _c[8];
  
  bool readPROM( void ) throw( std::runtime_error );
  bool readADC( char command, int& adc ) throw( std::runtime_error );
  unsigned char evaluateCRC4( void  ) throw( std::runtime_error );
  bool writeCommand( char command, int ms = 0 ) throw( std::runtime_error );
};

#endif // MS56XX_H
