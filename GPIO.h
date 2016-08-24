#ifndef GPIO_H
#define GPIO_H
// +--------------------------------------------------+
// | GPIO                                             |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.2                                             |
// | 20160421                                         |
// +--------------------------------------------------+

#include <stdexcept>
#include <string>

// +------------+
// | Class GPIO |
// +------------+

class GPIO {

 public:

  GPIO( void ) {}
  ~GPIO( void ) {}

  void connectBus( void );

  bool checkAccess( int channel, int address ) throw( std::runtime_error );

  int writeBus( int channel, unsigned char address, char register, char buffer[], int size ) throw( std::runtime_error );

  int readBus( int channel, unsigned char address, char register, char buffer[], int size ) throw( std::runtime_error );

  int writeBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error );

  int readBusUnreg( int channel, unsigned char address, char buffer[], int size ) throw( std::runtime_error );

  bool closeBus( void ) throw( std::runtime_error );

 private:

  int     _handle;
  char    _buff[128];
  int     _status;
  static const int _TCA9548A_BaseAddress = 0x70;    
  static const std::string _i2cdev;

  void setTCA9548AChannel( int channel) throw( std::runtime_error );

};

#endif // GPIO_H
