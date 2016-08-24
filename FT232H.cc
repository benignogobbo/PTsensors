// +--------------------------------------------------+
// | USB to I2C via FT232H                            |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.0                                             |
// | 20160317                                         |
// +--------------------------------------------------+

#include <iostream>
#include <sstream>
#include <cstring>

extern "C" {
#  include "mpsse.h"
}

#include "FT232H.h"


//+-------------------+
//| FT232H connectBus |
//+-------------------+
void FT232H::connectBus( void ) {
  if(( _handle = Open( _FT232HVendorId, _FT232HProductId, I2C, ONE_HUNDRED_KHZ, MSB, IFACE_A, NULL, NULL))
     != NULL && _handle->open) {
    _handle->description = (char*)malloc( 8 );
    strcpy( _handle->description, "FT232H" );
    //if(( _handle = MPSSE( I2C, ONE_HUNDRED_KHZ, MSB )) != NULL && _handle->open) {
    std::cout <<  std::endl << "\033[36m[" << __FUNCTION__ << "] I found an " << GetDescription( _handle )
	      << " (" << std::hex << GetVid(_handle) << ", " << GetPid(_handle)
	      << ") chip and I just initialized" << std::endl << "it to comunicate with I2C slaves at "
	      << std::dec << GetClock( _handle ) << "Hz, that's good!\033[0m" << std::endl;
  }
  else {
    std:: cout << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: failed connection to MPSSE device.\033[0m" << std::endl;
    exit(1);
  }
}

//+-----------------+
//| FT232H closeBus |
//+-----------------+
bool FT232H::closeBus( void ) throw( std::runtime_error ) {
  Close( _handle );
  return true;
}

//+---------------------------+
//| FT232H setTCA9548AChannel |
//+---------------------------+
void FT232H::setTCA9548AChannel(int channel) throw( std::runtime_error ) {

  if( channel < -1 || channel > 7 ) {
    throw( "Channel must be between 0 and 7" );
    return;
  }

  _buff[0] = (_TCA9548A_BaseAddress<<1)&0xfe;
  _buff[1] = 0;
  if( channel == -1 ) _buff[1] = 0;
  else _buff[1] = (_buff[1] | (1<<channel));

  Start( _handle );
  _status = Write( _handle, _buff, 2 );
  if( _status != MPSSE_OK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus write error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    return;
  }

  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Write command ACK error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    return;
  }

  Stop( _handle );
}

//+-----------------+
//| FT232H writeBus |
//+-----------------+
int FT232H::writeBus( int channel, unsigned char address, char reg, char buffer[], int size ) throw( std::runtime_error ) {

  setTCA9548AChannel(channel);
  
  _buff[0] = (address<<1)&0xfe;
  _buff[1] = reg;
  for( unsigned int i=0; i<size; i++ ) {
    _buff[2+i] = buffer[i];
  }

  Start( _handle );
  _status = Write( _handle, _buff, 2+size );
  if( _status != MPSSE_OK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus write error: " << _status << "\033[0m"; 
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Write command ACK error : " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  Stop( _handle );
  setTCA9548AChannel(-1);
  return( size );
}

//+----------------+
//| FT232H readBus |
//+----------------+
int FT232H::readBus( int channel, unsigned char address, char reg, char buffer[], int size ) throw( std::runtime_error ) {
  
  if( !size ) return 0;

  setTCA9548AChannel(channel);
  
  _buff[0] = (address<<1)&0xfe; 
  _buff[1] = reg;
  Start( _handle );
  _status = Write( _handle, _buff, 2 );
  if( _status != MPSSE_OK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus write error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Write command ACK error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  _buff[0] = (address<<1)|0x01; 
  Start( _handle );
  _status = Write( _handle, _buff, 1 );
  if( _status != MPSSE_OK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus write error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Write command ACK error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  char *p = Read( _handle, size );
  
  if( p ) {
    memcpy( buffer, p, size );
  }
  else {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus read error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }
  
  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Read command ACK error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  Stop( _handle );
  setTCA9548AChannel(-1);
  return( size );
}

//+----------------------+
//| FT232H writeBusUnreg |
//+----------------------+
int FT232H::writeBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error ) {

  setTCA9548AChannel(channel);
  
  _buff[0] = (address<<1)&0xfe; 
  for( unsigned int i=0; i<size; i++ ) {
    _buff[1+i] = buffer[i];
  }

  Start( _handle );
  _status = Write( _handle, _buff, 1+size );
  if( _status != MPSSE_OK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus write error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Write command ACK error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  Stop(_handle);
  setTCA9548AChannel(-1);
  return( size );
}

//+---------------------+
//| FT232H readBusUnreg |
//+---------------------+
int FT232H::readBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error ) {

  setTCA9548AChannel(channel);
  
  if( !size ) return 0;
  _buff[0] = (address<<1)|0x01;
  Start( _handle );
  _status = Write( _handle, _buff, 1 ); 
  if( _status != MPSSE_OK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus write error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Write command ACK error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  char *p = Read( _handle, size );
  
  if( p ) {
    memcpy( buffer, p, size );
  }
  else {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Bus read error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }
  
  _status = GetAck( _handle );
  if( _status != ACK ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] Read command ACK error: " << _status << "\033[0m";
    throw( std::runtime_error( ss.str() ) );
    Stop( _handle );
    setTCA9548AChannel(-1);
    return -1;
  }

  Stop( _handle );
  setTCA9548AChannel(-1);
  return( size );
}
