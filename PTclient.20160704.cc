//+-----------------------------------+
//| connect to PT server and get data |
//| Benigno Gobbo                     |
//| v1.0 - 20160407                   |
//+-----------------------------------+

#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// +-------+
// | kbhit |
// +-------+
int kbhit() {

    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

// +------+
// | main |
// +------+
int main( int argc, char *argv[] ) {

  const int portno = 57777;
  const int sleepSecs = 5;
  const int channels = 2;

  int n, sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256];

  if( argc < 2 ) {
    std::cout << "usage " << argv[0] << " hostname" << std::endl;
    return 1;
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if( sockfd < 0 ) {
    std:: cout << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: opening socket failure.\033[0m" << std::endl;
    return 1;
  }
 
  server = gethostbyname( argv[1] );
  if (server == NULL) {
    std:: cout << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: host not found.\033[0m" << std::endl;
    return 1;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
  serv_addr.sin_port = htons(portno);

  if( connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr) ) < 0 ) {
    std:: cout << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: connection failed.\033[0m" << std::endl;
    return 1;
  }

  std::string command =  "data";

  std::cout << "\033c";
  while( true ) {

    //std::cout << "sending 'data' command" << std::endl;
    n = write( sockfd, command.c_str(), command.size() );
    if( n < 0 ) { 
      std:: cout << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: write to socket failure.\033[0m" << std::endl;
      return 1;
    }

    //std::cout << "reading answer" << std::endl;
    bzero( buffer, 256 );
    n = read( sockfd, buffer, 255 );
    if( n < 0 ) { 
      std:: cout << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: read from socket failure.\033[0m" << std::endl;
      return 1;
    }

    std::stringstream ss;
    ss << buffer;
    time_t date; int sn[channels]; double p1[channels], t1[channels], t2[channels];

    ss >> date; 
    for( int i=0; i<channels; i++ ) { ss >> sn[i]; ss >> p1[i]; ss >> t1[i]; ss >> t2[i]; }

    struct tm tstruct = *localtime( &date );
    char buf[80];
    strftime( buf, sizeof(buf), "%Y/%m/%d %X", &tstruct );
    for( int i=0; i<channels; i++ ) { 
      std::cout << "\033[34m[" << buf
		<< std::fixed << std::setprecision(2)
		<< "]\033[0m MS56xx sn: \033[36m" << std::setw(5) << sn[i]
		<< "\033[0m, P: \033[32m" << std::setw(5) << p1[i]
		<< "\033[0m, T: \033[32m" << std::setw(5) << t1[i]
		<< "\033[0m - ADT7429 T: \033[32m" << std::setw(5) << t2[i] << "\033[0m" << std::endl;
    }
    for( int j=0; j<sleepSecs; j++ ) {
      std::cout << "\033[2K\033[33mNow sleeping " << sleepSecs-j << " seconds \033[34m(<Enter> to exit)\033[0m" << std::endl;
      int hit = kbhit();
      if( hit != 0 ) {
	command = "exit";
	n = write( sockfd, command.c_str(), command.size() );
	if( n < 0 ) { 
	  std:: cout << "\033[1;31m[" << __FUNCTION__ << "] Fatal error: write to socket failure.\033[0m" << std::endl;
	  return 1;
	}
	close( sockfd );
	return 0;
      }
      std::cout << "\033[2A" << std::endl;
    }
    //std::cout << "\033[" << 3 << "A" << std::endl;
    std::cout << "\033c";
  }

  return 0;
}
