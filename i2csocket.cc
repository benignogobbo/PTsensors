// +--------------------------------------------------+
// | libFt232H socket test program                    |  
// |                                                  |
// | Benigno Gobbo                                    |
// | (c) INFN Sezione di Trieste                      |
// | V1.2                                             |
// | 2016/07/04                                       |
// +--------------------------------------------------+

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "UII2C.h"

std::string cdt( void );

void chat( int sock, std::vector<ADT7420*> adts, std::vector<MS56xx*> ms56s );

// This is the Active Channels Bit Mask
std::string channels( "11111111" );  // connector(bit) = 87654321

const int portno = 57777;
const bool verbose = false;

int main() {

  int sockfd, newsockfd;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  std::vector<ADT7420*> adts;
  std::vector<MS56xx*> ms56s;

  std::cout << "[" << __FUNCTION__ << ", " << cdt() << "] i2csocket program started" << std::endl;
  
  signal( SIGCHLD, SIG_IGN );

  try {
    UII2C ui = UII2C( channels );
    ui.init();
    adts = ui.getADTs();
    ms56s = ui.getMS56s();
  } catch( std::runtime_error &e ) {
    std::cout << e.what() << std::endl;
    return -1;
  }
  
  sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  if (sockfd < 0) { 
    std::cout << "\033[31m[" << __FUNCTION__ << ", " << cdt() << "] Error opening socket \033[0m" << std::endl;
    return 1 ;
  }

  struct timeval timeout; timeout.tv_usec = 0; timeout.tv_sec = 0;  // timeout = 0. I.e. no timeout
  if( setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout) ) < 0 ) {
    std::cout << "\033[31m[" << __FUNCTION__ << ", " << cdt() << "] Error from setsockopt \033[0m" << std::endl;
    return 1 ;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0 ) {
    std::cout << "\033[31m[" << __FUNCTION__ << ", " << cdt() << "] Error on binding \033[0m" << std::endl;
    return 1 ;
  }

  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  while( true ) {

    newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
    if( newsockfd < 0 ) {
      std::cout << "\033[31m[" << __FUNCTION__ << ", " << cdt() << "] Error on accept \033[0m" << std::endl;
      return 1 ;
    }

    int pid = fork();
    if( pid < 0 ) {
      std::cout << "\033[31m[" << __FUNCTION__ << ", " << cdt() << "] Error on fork \033[0m" << std::endl;
      return 1 ;
    }
    else if ( pid == 0 ) {
      close(sockfd);
      chat( newsockfd, adts, ms56s );
      exit(0);
    }
    else {
      close(newsockfd);
    }

  }
  return 0;  // it should never arrive here...
}

// +------+
// | chat |
// +------+
//
void chat( int sock, std::vector<ADT7420*> adts, std::vector<MS56xx*> ms56s ) {

  char buffer[256];
  int n;

  while( true ) {
    bzero(buffer,256);
    n = read( sock, buffer, 255 );
    if( n < 0 ) {
      std::cout << "\033[31m[" << __FUNCTION__ << ", " << cdt() << "] Error reading from socket \033[0m" << std::endl;
    }

    if( strncmp( buffer, "exit", 4 ) == 0 ) {
      close( sock );
      exit(0);
    }
    else if( strncmp( buffer, "data", 4 ) == 0 ) {
      if( verbose ) std::cout << "\033[0m[" << cdt() << "] Got 'data' request from client, I'm sending it" << std::endl;
      time_t now = time(0);
      std::stringstream ss;
      ss << now;
      std::bitset<8> chBits( channels ); 
      for( int i=0; i<channels.size(); i++ ) {
	if( chBits.test(i) ) {
	  double temp1 = 0, temp2 = 0, pres1 = 0;
	  int sn = ms56s[i]->getSerialNumber();
	  ms56s[i]->getTemperature( temp1 );
	  ms56s[i]->getPressure( pres1 );
	  adts[i]->readTemperature( temp2 );
	  ss << " " << sn << " " << pres1/100. << " " << temp1 << " " << temp2;
	}
      }
      n = write( sock, ss.str().c_str(), ss.str().size() );
      if( n < 0 ) {
	std::cout << "\033[31m[" << __FUNCTION__ << ", " << cdt() << "] Error writing to socket \033[0m" << std::endl;
      }
      if( verbose ) std::cout << "\033[0m[" << cdt() << "] OK, I just sent data to client..." << std::endl;
    }
  }
}

// +-----+
// | cdt |
// +-----+
//
std::string cdt( void ) {
  
  char buf[80];
  time_t now = time(0);
  struct tm tstruct = *localtime( &now );
  strftime( buf, sizeof(buf),  "%Y/%m/%d %X", & tstruct );
  std::string answ = buf;
  return answ;
}
