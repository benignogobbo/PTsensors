// +--------------------------------------------------+
// | MS56xx Barometric Pressure Sensor                |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V2.1                                             |
// | 20160317                                         |
// +--------------------------------------------------+

#include <iostream>
#include <sstream>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include "MS56xx.h"

// +-----------------+
// | MS56xx readPROM |
// +-----------------+
bool MS56xx::readPROM( void ) throw( std::runtime_error ) {
  
  char address = MS56xx_BaseAddress + _id;
  char buff[4];

  for( int i=0; i<8; i++ ) {
    buff[0] = MS56xx_PROM_Read | ( i << 1 );
    int status = _usb->writeBusUnreg( _tcach, address, buff, 1 );
    if( status != 1 ) {
      std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] bus write error\033[0m";
      throw ( std::runtime_error( ss.str() ) );
      return false;
    } 
    status = _usb->readBusUnreg( _tcach, address, buff, 2 );
    if( status != 2 ) {
      std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] bus read error\033[0m";
      throw ( std::runtime_error( ss.str() ) );
      return false;
    }
    _c[i] = ( (unsigned char)buff[0] << 8 ) + (unsigned char)buff[1];
  }

  unsigned char storCRC = _c[7] & 0xf;
  _serialNumber = ((_c[7] >> 4 )  & 0xfff );
  unsigned char evalCRC = evaluateCRC4();
  if( storCRC != evalCRC ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] CRC error\033[0m";
    throw ( std::runtime_error( ss.str() ) );
    return false;
  }
  return true;
}

// +----------------+
// | MS56xx readADC |
// +----------------+
bool MS56xx::readADC( char command, int& adc ) throw( std::runtime_error ) {
  
  char address = MS56xx_BaseAddress + _id;
  char buff[8];

  buff[0] = MS56xx_ADC_Conv | command;
  int status = _usb->writeBusUnreg( _tcach, address, buff, 1 );
  if( status != 1 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] bus read error\033[0m";
    throw ( std::runtime_error( ss.str() ) );
    return false;
  }
  //static int duration[] = { 540, 1060, 2080, 4130, 8220 };   // typical conversion time
  static int duration[] = { 600, 1170, 2280, 4540, 9040 };   // maximal conversion time 
  usleep( duration[ ( command & 0x0f ) >> 1 ] );
  buff[0] = MS56xx_ADC_Read;
  status = _usb->writeBusUnreg( _tcach, address, buff, 1 );
  if( status != 1 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] bus write error\033[0m";
    throw ( std::runtime_error( ss.str() ) );
    return false;
  }
  status = _usb->readBusUnreg( _tcach, address, buff, 4 );
  if( status != 4 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] bus read error\033[0m";
    throw ( std::runtime_error( ss.str() ) );
    return false;
  }  
  adc = (unsigned char)buff[0] << 16 | (unsigned char)buff[1]<<8 | (unsigned char)buff[2];
  return true;
}

// +---------------------+
// | MS56xx writeCommand |
// +---------------------+
bool MS56xx::writeCommand( char command, int ms ) throw( std::runtime_error ) {

  char address = MS56xx_BaseAddress + _id;
  char buff[8];

  buff[0] = command;
  int status = _usb->writeBusUnreg( _tcach, address, buff, 1 );
  if( status != 1 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] bus write error\033[0m";
    throw ( std::runtime_error( ss.str() ) );
    return false;
  }
  if( ms ) usleep( ms*1000 );
  return true;
}

// +-------------+
// | MS56xx init |
// +-------------+
bool MS56xx::init( void ) throw( std::runtime_error ) {
  try {
    writeCommand( MS56xx_Reset, 3 );
    readPROM();
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}


// +------------------------+
// | MS56xx readRawPressure |
// +------------------------+
bool MS56xx::readRawPressure( int& ipress ) throw( std::runtime_error ) {
  try {
    readADC( MS56xx_ADC_D1 | MS56xx_OSR_4096, ipress );
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}


// +---------------------------+
// | MS56xx readRawTemperature |
// +---------------------------+
bool MS56xx::readRawTemperature( int& itemp ) throw( std::runtime_error ) {
  try {
    readADC( MS56xx_ADC_D2 | MS56xx_OSR_4096, itemp );
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}


// +--------------------+
// | MS56xx getPressure |
// +--------------------+
bool MS56xx::getPressure( double& press ) throw( std::runtime_error ) {

  try {
    int itemp = 0;
    readRawTemperature( itemp );
    int64_t dT = itemp - ( _c[5] << 8 );
    itemp = 2000 + ( ( dT>>13 ) * ( _c[6] >> 10 ) );

#if defined( MS5611_DEVICE )
    int64_t off = ( _c[2] << 16 ) + ( ( dT * _c[4] ) >> 7 );
    int64_t sens = ( _c[1] << 15 ) + ( ( dT * _c[3] ) >> 8 );
#else
    int64_t off = ( _c[2] << 17 ) + ( ( dT * _c[4] ) >> 6 );
    int64_t sens = ( _c[1] << 16 ) + ( ( dT * _c[3] ) >> 7 );
#endif

    if( itemp < 2000 ) {
 
#if defined( MS5611_DEVICE )
      int64_t off2 =  (int64_t)5 * ( ( ( itemp - 2000 ) * ( itemp - 2000 ) ) >> 1 );
      int64_t sens2 = (int64_t)5 * ( ( ( itemp - 2000 ) * ( itemp - 2000 ) ) >> 2 );
      if( itemp < -1500 ) {
	off2 += (int64_t)7 * ( itemp + 1500 ) * ( itemp + 1500 );
	sens2 += (int64_t)11 * ( itemp + 1500 ) * ( itemp + 1500 ) >> 1;
      }
#else
      int64_t off2 = (int64_t)61 * ( ( ( itemp - 2000 ) * ( itemp - 2000 ) ) >> 4 );
      int64_t sens2 = (int64_t)2 * ( itemp - 2000 ) * ( itemp - 2000 );
      if( itemp < -1500 ) {
	off2 += (int64_t)15 * ( itemp + 1500 ) * ( itemp + 1500 );
	sens2 += (int64_t)8 * ( itemp + 1500 ) * ( itemp + 1500 );
      }
#endif

      off -= off2;
      sens -= sens2;
    }
    int ipress;
    readRawPressure( ipress );
    press = double( ( ( ( ipress * sens ) >> 21 ) - off ) >> 15 );

  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
} 

// +-----------------------+
// | MS56xx getTemperature |
// +-----------------------+
bool MS56xx::getTemperature( double& temp ) throw( std::runtime_error ) {

  try {
    int itemp;
    readRawTemperature( itemp );
    int64_t dT = itemp - ( _c[5] << 8 );
    itemp = 2000 + ( ( dT>>13 ) * ( _c[6] >> 10 ) );
    if( itemp < 2000 ) {
      int64_t t2 = ( dT * dT ) >> 31;
      itemp -= t2;
    }
    temp = double( itemp ) / 100.;
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;

}

// +--------------------+
// | MS56xx getAltitude |
// +--------------------+
bool MS56xx::getAltitude( double& alt ) throw( std::runtime_error ) {

  try {
    double press = 0;
    getPressure( press );
    const double R = 287.052; // specific gas constant 
    const double g = 9.80665; // gravity
    const double t_grad = 0.0065; // temperature gradient
    const double t0 = 273.15 + 15; // temperature at 0 altitude
    const double p0 = 101325; // pressure at 0 altitude
    alt = t0 / t_grad * ( 1 - exp( ( t_grad * R / g ) * log( press / p0 ) ) );
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;

}

// +---------------------+
// | MS56xx evaluateCRC4 |
// +---------------------+
unsigned char MS56xx::evaluateCRC4( void ) throw( std::runtime_error ) {

  unsigned int saveC7 = _c[7];
  _c[7] = ( 0xFF00 & _c[7] );
  unsigned int rem = 0;

  try {

    for( int cnt = 0; cnt < 16; cnt++ ){
      if ( cnt%2 == 1 ) {
	rem ^= (unsigned short) ( _c[cnt>>1] & 0x00FF );
      } else {
	rem ^= (unsigned short) ( _c[cnt>>1] >> 8 );
      }
      for( int bt = 8; bt > 0; bt--) {
	if( rem & 0x8000 ) {
	  rem = ( rem << 1 ) ^ 0x3000;
	} else {
	  rem = rem << 1;
	}
      }
    }
    rem = ( 0x000F & ( rem >> 12 ));

  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    _c[7] = saveC7;
    return 0;
  }

  _c[7] = saveC7;

  return ( rem ^ 0x0 );

}
