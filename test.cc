// +--------------------------------------------------+
// | libFT232H test program                           |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.2                                             |
// | 2016/04/07                                       |
// +--------------------------------------------------+

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include "UII2C.h"

int main() {

  // This is the Active Channels Bit Mask
  std::string channels( "01001000" );  // connector(bit) = 87654321
  
  try {

    std::cout <<  std::endl << "\033[32m[" << __FUNCTION__ << "] Well, I'll try to set up I2C user interface....\033[0m" << std::endl;

    UII2C ui = UII2C( channels );
    ui.init();
    
    std::vector<ADT7420*> adts = ui.getADTs();
    std::vector<MS56xx*> ms56s = ui.getMS56s();

    std::cout << std::endl << "\033[32m[" << __FUNCTION__ << "] Mmmh. And now, let me even try to read sensors data:\033[0m" << std::endl;
    std::bitset<8> chBits( channels ); 
    for( int i=0; i<channels.size(); i++ ) {
      if( chBits.test(i) ) {
	std::cout << std::endl << "\033[34mThe switch channel should be: " << i << "\033[0m"<< std::endl;

	std::cout << "Here are the MS56xx PROM stored values (hopefully): ";
	for( int j=0; j<8; j++ ) {
	  std::cout << ms56s[i]->getCoefficient(j) << " ";
	}
	std::cout << std::endl;

	double pres = 0;
	if( ms56s[i]->getPressure( pres ) ) {
	  std::cout << "The MS56xx measured pressure seems to be:               \033[36m"
		    << pres/100. << "\033[0m" << std::endl;
	}
	else {
	  std::cout << "\033[33m[" << __FUNCTION__ << "] MS56XX error in getting pressure measurement\033[0m" << std::endl;
	  exit(1);
	}

	double mtemp = 0;
	if( ms56s[i]->getTemperature( mtemp ) ) {
	  std::cout << "And its measured temperature presumably is:              \033[34m"
		    << std::fixed << std::setprecision(2) << mtemp << "\033[0m" << std::endl;
	}
	else {
	  std::cout << "\033[33m[" << __FUNCTION__ << "] MS56XX error in getting temperature measurement\033[0m" << std::endl;
	  exit(1);
	}

	double temp;
	if( adts[i]->readTemperature( temp ) ) {
	  std::cout << "At the end, the ADT7420 measured temperature looks like: \033[34m"
		    << std::fixed << std::setprecision(2) << temp << "\033[0m" << std::endl;
	}
	else {
	  std::cout << "\033[33m[" << __FUNCTION__ << "] ADT7420 error in getting temperature measurement\033[0m" << std::endl;
	  exit(1);
	}
      }
    }
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return -1;
  }

  std::cout <<  std::endl << "\033[35mOK gentlemen (and ladyes, if any), I did even too much, I'm tired" << std::endl
	    << "and I'll stop here... Have a seasonably nice rest of the day!" << std::endl
	    << "Ciao, your test program\033[0m" << std::endl << std::endl;
  
  return 0;

}
