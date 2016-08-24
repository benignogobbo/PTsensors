#ifndef FT232H_H
#define FT232H_H
// +--------------------------------------------------+
// | USB to I2C via FT232H                            |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.0                                             |
// | 20160317                                         |
// +--------------------------------------------------+

#include <stdexcept>
#include <usb.h>

// +---------------+
// | Class FT232H |
// +---------------+

class FT232H {

 public:

  FT232H( void ) { _handle = NULL; }
  ~FT232H( void ) {}

  void connectBus( void );

  bool checkAccess( int channel, int address ) throw( std::runtime_error );

  int writeBus( int channel, unsigned char address, char register, char buffer[], int size ) throw( std::runtime_error );

  int readBus( int channel, unsigned char address, char register, char buffer[], int size ) throw( std::runtime_error );

  int writeBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error );

  int readBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error );

  bool closeBus( void ) throw( std::runtime_error );

 private:

  struct mpsse_context *_handle;
  char    _buff[128];
  int     _status;
  static const int _TCA9548A_BaseAddress = 0x70;    
  
  static const int _FT232HVendorId  = 0x0403;
  static const int _FT232HProductId = 0x6014;
  void setTCA9548AChannel( int channel) throw( std::runtime_error );

};

#endif // FT232H_H
