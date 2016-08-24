
// +--------------------------------------------------+
// | libFt232H loop test program                      |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.2                                             |
// | 2016/07/04                                       |
// +--------------------------------------------------+

#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>
#include <unistd.h>
#include "UII2C.h"

int main() {

  // This is the Active Channels Bit Mask
  std::string channels( "11111111" );  // connector(bit) = 87654321

  // Time between measurements
  int sleepSecs = 10;
  
  try {

    UII2C ui = UII2C( channels );
    ui.init();

    std::vector<ADT7420*> adts = ui.getADTs();
    std::vector<MS56xx*> ms56s = ui.getMS56s();

    std::cout << std::endl << std::endl;

    std::cout << "\033c";
    while( true ) {

      std::bitset<8> chBits( channels ); 
      for( int i=0; i<chBits.size(); i++ ) {
	if( chBits.test(i) ) {
	  double temp1 = 0, temp2 = 0, pres1 = 0;
	  ms56s[i]->getTemperature( temp1 );
	  ms56s[i]->getPressure( pres1 );
	  adts[i]->readTemperature( temp2 );
	  time_t now = time(0);
	  struct tm tstruct = *localtime( &now );
	  char buf[80];
	  strftime( buf, sizeof(buf), "%Y/%m/%d %X", &tstruct );
	  std::cout << "\033[34m[" << buf
		    << std::fixed << std::setprecision(2)
		    << " ]\033[0m MS56xx( s/n "  << ms56s[i]->getSerialNumber()
		    << "), P: \033[36m" << std::setw(7) << pres1/100.
		    << "\033[0m, T: \033[32m" << std::setw(5) << temp1
		    << "\033[0m - ADT7429 T: \033[32m" << std::setw(5) << temp2 << "\033[0m" << std::endl;
	}
      }  
      for( int j=0; j<sleepSecs; j++ ) {
	std::cout << "\033[2K\033[33mNow sleeping " << sleepSecs-j << " seconds\033[0m" << std::endl;
	sleep( 1 );
	std::cout << "\033[2A" << std::endl;
      }
      //std::cout << "\033[" << channels+1 << "A" << std::endl;
      std::cout << "\033c";
    }
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return -1;
  }
  
  return 0;

}
