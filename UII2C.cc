
// +--------------------------------------------------+
// | I2C User Interface                               |
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.2                                             |
// | 20160704                                         |
// +--------------------------------------------------+

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "UII2C.h"

// +--------------+
// | Constructors |
// +--------------+

UII2C::UII2C( void ) {
  _Channels.set();
}

UII2C::UII2C( int nChannels, int chOffset ) {
  _Channels.reset();
  for( int i=chOffset; i<nChannels; i++ ) {
    _Channels.set( i );
  }
}

UII2C::UII2C( std::string channels ) {
  _Channels = std::bitset<8>( channels );
}


// +------------+
// | UII2C Init |
// +------------+
void UII2C::init( void ) {

  try {
    
    _bus = new MYBUS();
    _bus->connectBus();

    _adts.resize( _Channels.size(), NULL );
    _ms56s.resize( _Channels.size(), NULL );

    for( int i=0; i<_Channels.size(); i++ ) {
      if( _Channels.test(i) ) {
	_adts[i] = new ADT7420( _adtAddress, _bus, i );
	_adts[i]->configure((1<<7));
	
	_ms56s[i] = new MS56xx( _ms56Address, _bus, i );
	_ms56s[i]->init();
	
      }
    }
  } catch ( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    exit(1);
  }

  _init = true;
}

// +-----------------+
// | UII2C getBus    |
// +-----------------+      
MYBUS* UII2C::getBus( void ) throw( std::runtime_error ) {

  if( !_init ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] UII2C not yet initialized\033[0m";
    throw ( std::runtime_error( ss.str() ) );
  }

  return( _bus );

}

// +------------------+
// | UII2C getADTs    |
// +------------------+
std::vector<ADT7420*> UII2C::getADTs( void ) throw( std::runtime_error ) {

  if( !_init ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] UII2C not yet initialized\033[0m";
    throw ( std::runtime_error( ss.str() ) );
  }

  return( _adts );

}

// +-------------------+
// | UII2C getMS56s    |
// +-------------------+
std::vector<MS56xx*>  UII2C::getMS56s( void ) throw( std::runtime_error ){
  
  if( !_init ) {
    std::stringstream ss; ss << "\033[33m[" << __FUNCTION__ << "] UII2C not yet initialized\033[0m";
    throw ( std::runtime_error( ss.str() ) );
  }
  
  return( _ms56s );
}
