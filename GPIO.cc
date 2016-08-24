// +--------------------------------------------------+
// | GPIO                                             |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.2                                             |
// | 20160421                                         |
// +--------------------------------------------------+

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "GPIO.h"

const std::string GPIO::_i2cdev = "/dev/i2c-1";

//+-----------------+
//| GPIO connectBus |
//+-----------------+
void GPIO::connectBus( void ) {
  if(( _handle = open( _i2cdev.c_str(), O_RDWR )) < 0 ) {
    std:: cout << "\033[31m[" << __FUNCTION__ << "] Fatal error: failed to open the I2C bus.\033[0m" << std::endl;
    exit(1);
  }
  else {
    std:: cout << "\033[32m[" << __FUNCTION__ << "] OK ,I2C bus is now connected.\033[0m" << std::endl;
  }
}

//+---------------+
//| GPIO closeBus |
//+---------------+
bool GPIO::closeBus( void ) throw( std::runtime_error ) {
  int status = close( _handle );
  if( status < 0 ) {
    return false;
  }
  else {
    return true;
  }
}

//+-------------------------+
//| GPIO setTCA9548AChannel |
//+-------------------------+
void GPIO::setTCA9548AChannel( int channel ) throw( std::runtime_error ) {

  if( channel < -1 || channel > 7 ) {
    throw( "Channel must be between 0 and 7" );
    return;
  }
  
  if( channel == -1 ) _buff[0] = 0;
  else _buff[0] = (_buff[0] | (1<<channel));

  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];
  messages[0].addr = _TCA9548A_BaseAddress;
  messages[0].flags = 0;
  messages[0].len = 1;
  messages[0].buf = _buff;
  packets.msgs = messages;
  packets.nmsgs = 1;  
  _status = ioctl( _handle, I2C_RDWR, &packets );
  if( _status < 0 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] ioctl error: failed to acquire bus access or talk to slave.\033[0m";
    throw( std::runtime_error( ss.str() ) );
    return;
  }

 }

//+---------------+
//| GPIO writeBus |
//+---------------+
int GPIO::writeBus( int channel, unsigned char address, char reg, char buffer[], int size ) throw( std::runtime_error ) {

  setTCA9548AChannel( channel );
  
  _buff[0] = reg;
  for( unsigned int i=0; i<size; i++ ) {
    _buff[1+i] = buffer[i];
  }

  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];  
  messages[0].addr = address;
  messages[0].flags = 0;
  messages[0].len = size+1;
  messages[0].buf = _buff;
  packets.msgs = messages;
  packets.nmsgs = 1;
  _status = ioctl( _handle, I2C_RDWR, &packets );
  if( _status < 0 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] ioctl error: failed to acquire bus access or talk to slave.\033[0m";
    throw( std::runtime_error( ss.str() ) );
    return -1;
  }

  setTCA9548AChannel(-1);
  return( size );

}

//+--------------+
//| GPIO readBus |
//+--------------+
int GPIO::readBus( int channel, unsigned char address, char reg, char buffer[], int size ) throw( std::runtime_error ) {
  
  if( !size ) return 0;

  setTCA9548AChannel( channel );
  
  _buff[0] = reg;
  char readbuff[10];

  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];
  messages[0].addr = address;
  messages[0].flags = 0;
  messages[0].len = 1;
  messages[0].buf = _buff;
  messages[1].addr = address;
  messages[1].flags = I2C_M_RD;
  messages[1].len = size;
  messages[1].buf = readbuff;
  packets.msgs = messages;
  packets.nmsgs = 2;
  _status = ioctl( _handle, I2C_RDWR, &packets );
  if( _status < 0 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] ioctl error: failed to acquire bus access or talk to slave.\033[0m";
    throw( std::runtime_error( ss.str() ) );
    return -1;
  }

  if( readbuff ) {
    memcpy( buffer, readbuff, size );
  }

  setTCA9548AChannel(-1);
  return( size );

}

//+--------------------+
//| GPIO writeBusUnreg |
//+--------------------+
int GPIO::writeBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error ) {

  setTCA9548AChannel( channel );
  
  for( unsigned int i=0; i<size; i++ ) {
    _buff[i] = buffer[i];
  }

  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];  
  messages[0].addr = address;
  messages[0].flags = 0;
  messages[0].len = size;
  messages[0].buf = _buff;
  packets.msgs = messages;
  packets.nmsgs = 1;
  _status = ioctl( _handle, I2C_RDWR, &packets );
  if( _status < 0 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] ioctl error: failed to acquire bus access or talk to slave.\033[0m";
    throw( std::runtime_error( ss.str() ) );
    return -1;
  }

  setTCA9548AChannel(-1);
  return( size );

}

//+-------------------+
//| GPIO readBusUnreg |
//+-------------------+
int GPIO::readBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error ) {

  setTCA9548AChannel( channel );
  
  if( !size ) return 0;

  char readbuff[10];
  
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];
  messages[0].addr = address;
  messages[0].flags = I2C_M_RD;
  messages[0].len = size;
  messages[0].buf = readbuff;
  packets.msgs = messages;
  packets.nmsgs = 1;
  _status = ioctl( _handle, I2C_RDWR, &packets );
  if( _status < 0 ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] ioctl error: failed to acquire bus access or talk to slave.\033[0m";
    throw( std::runtime_error( ss.str() ) );
    return -1;
  }

  if( readbuff ) {
    memcpy( buffer, readbuff, size );
  }

  setTCA9548AChannel(-1);
  return( size );

}
