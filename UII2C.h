#ifndef UII2C_H
#define UII2C_H
// +--------------------------------------------------+
// | I2C User Interface                               |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.2                                             |
// | 20160704                                         |
// +--------------------------------------------------+

#include <vector>
#include <bitset>
#ifdef USB_FT232H
#  define MYBUS FT232H
#  include "FT232H.h"
#else
#  define MYBUS GPIO
#  include "GPIO.h"
#endif
#include "MS56xx.h"
#include "ADT7420.h"

// +-------------+
// | Class UII2C |
// +-------------+

class UII2C {

 public:

  UII2C( void );
  UII2C( int nChannels, int chOffset=0 );
  UII2C( std::string channels );

  ~UII2C( void ) {}

  void init( void );

  MYBUS* getBus( void ) throw( std::runtime_error );
  std::vector<ADT7420*> getADTs( void ) throw( std::runtime_error );
  std::vector<MS56xx*>  getMS56s( void ) throw( std::runtime_error );
    
 private:

  std::bitset<8>         _Channels;
  MYBUS*                 _bus;
  bool                   _init;
  std::vector<ADT7420*>  _adts;
  std::vector<MS56xx*>   _ms56s;
  static const int       _adtAddress  = 0;
  static const int       _ms56Address = 0;
};

#endif // UII2C_H
