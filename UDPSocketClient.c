/* ========================================================================= */
/* UDPSocketClient.c -- rfcontrol UDP socket read implementation             */
/* ========================================================================= */
#include <arpa/inet.h> 
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define _BSD_SOURCE
#include <endian.h>

#include "reader.h"
#include "logger.h"
#include "oracle.h"

#define SERVER_UDP_PORT 60000
#define SERVER_IP_ADDR "192.168.1.100"

#define TIMEOUT 30
#define OPEN_TIME 3

#define MODULE _rfcontrol_module_socket
const char *_rfcontrol_module_socket = "UDP socket";
static int sockfd = 0;
static int _in_transaction = 0; // FIXME: not thread-safe (note: I seriously doubt we want it to be thread-safe)

int reverseEndian(char* buf, int n) {
  if(n%4) return 1;
  for (int i = 0; i != n; i +=4) {
    register char i0 = buf[i], i1 = buf[i+1], i2 = buf[i+2], i3 = buf[i+3];
    buf[i] = i3;
    buf[i+1] = i2;
    buf[i+2] = i1;
    buf[i+3] = i0;
  }
  return 0;
}

int convertKey(unsigned int key) {
  char s[8] = {0,0,0,0,0,0,0,0};
  sprintf(s, "%04x%04x", key / 100000, key % 100000);
  return (unsigned int)strtoul(s, NULL, 16);
}

int activateMonitoring(void) {
  register int n;
  char recvBuff[2048];
  struct sockaddr_in serv_addr;

  fprintf(stderr, "sockfd=%d\n", sockfd);
  if(sockfd > 0)
    close(sockfd);
  
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("\n Error : Could not create socket \n");
    return 1;
  }
  fprintf(stderr, "sockfd=%d\n", sockfd);

  memset(&serv_addr, '0', sizeof(serv_addr)); 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_UDP_PORT);
  if(inet_pton(AF_INET, SERVER_IP_ADDR, &serv_addr.sin_addr)<=0) {
    printf("\n inet_pton error occured\n");
    return 1;
  }
  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\n Error : Connect Failed \n");
    return 1;
  } 
  logprintf(MODULE, "socket open\n");
  
  // ENABLE MONITOR MODE
  if ( (n = send(sockfd, reader_monitor_req, sizeof(reader_monitor_req), 0)) <  0) {
    printf("\n Write error\n");
    return 1;
  }
  // FIXME MAC address
  logprintf(MODULE, "monitor REQ message written (length %d)\n", n);
  if( (n = recv(sockfd, recvBuff, sizeof(reader_monitor_ans), 0)) < 0) {
    printf("\n Read error \n");
    return -1;
  } else if (!n) {
    printf("\n Socket closed \n");
    return -1;
  } else {
    logprintf(MODULE, "monitor ACK message read (length %d)\n", n);
    recvBuff[n] = 0;
    reverseEndian(reader_monitor_ans, sizeof(reader_monitor_ans));
    reverseEndian(recvBuff, n);
    /*
      fputs("\ttemplate: ", stdout);
      for(int i = 0; i < sizeof(reader_monitor_ans); i++) {
      printf("%02x:", (unsigned char)reader_monitor_ans[i]);
      }
      fputs("\n\t  output: ", stdout);
      for(int i = 0; i < n; i++) {
      printf("%02x:", (unsigned char)recvBuff[i]);
      }
      puts("");
    */
    return 0;
  }
}

void on_alarm(int signal) {
  if(_in_transaction) {
    return;
  } else {
    logprintf(MODULE, "timeout... re-activating monitoring\n");
    _in_transaction = 1;
    activateMonitoring();
    _in_transaction = 0;
  }
}

int UDPSocketClient(void) {
  register int n = 0, nKey = 0;
  char recvBuff[2048];

  memset(recvBuff, 0, sizeof(recvBuff));

  activateMonitoring();

  // MONITORING CARD READER
  signal(SIGALRM, on_alarm);
  alarm(TIMEOUT);
  /* WHY DOES IT READ EVEN AFTER??? */
  logprintf(MODULE, "waiting for badges to pass\n");
  while ( (nKey = recv(sockfd, recvBuff, sizeof(reader_badge_pass), 0)) >= 0) {
    // FIXME nKey == 0 --> restart socket & monitor

    fputs("\t  output:", stdout);
    for(int i = 0; i < nKey; i++) {
      if(!(i%20)) printf("\n\t");
      printf("%02x:", (unsigned char)recvBuff[i]);
    }
    puts("");
    
    logprintf(MODULE, "badge pass message read (length %d)\n", nKey);
    
    reverseEndian(recvBuff, nKey);
    char keys[5] = {0,0,0,0,0};
    memcpy(keys, recvBuff+68, 4);
    unsigned int key = ((unsigned char)keys[3] +  (unsigned char)keys[2]*0x100) +  0x10000*((unsigned char)keys[1] +  (unsigned char)keys[0]*0x100);
    //key = convertKey(key); // -- pas aujourd'hui bien oui à Annecy
    logprintf(MODULE, "badge key %010u (0x%x) passed\n", key, key);
      
    if(decide(key)) {
      // OPEN
      if ( (n = send(sockfd, reader_open_req1, sizeof(reader_open_req1), 0)) <  0) {
	printf("\n Write error\n");
	return 1;
      }
      logprintf(MODULE, "open REQ message 1 written (length %d)\n", n);

      if( (n = recv(sockfd, recvBuff, sizeof(reader_open_ans1), 0)) <= 0) {
	printf("\n Read error \n");      
      }
      logprintf(MODULE, "open ACK message 1 read (length %d)\n", n);
      recvBuff[n] = 0;
      /* fputs("\ttemplate: ", stdout);
	 for(int i = 0; i < sizeof(reader_open_ans1); i++) {
	 printf("%02x:", (unsigned char)reader_open_ans1[i]);
	 }
	 fputs("\n\t  output: ", stdout);
	 for(int i = 0; i < n; i++) {
	 printf("%02x:", (unsigned char)recvBuff[i]);
	 }
	 puts("");
      */
	
      if ( (n = send(sockfd, reader_open_req2, sizeof(reader_open_req2), 0)) <  0) {
	printf("\n Write error\n");
	return 1;
      }
      logprintf(MODULE, "open REQ message 2 written (length %d)\n", n);
	
      if( (n = recv(sockfd, recvBuff, sizeof(reader_open_ans2), 0)) < 0) {
	printf("\n Read error \n");
      }
      logprintf(MODULE, "open ACK message 2 read (length %d)\n", n);
      recvBuff[n] = 0;
      /* fputs("\ttemplate: ", stdout);
	 for(int i = 0; i < sizeof(reader_open_ans2); i++) {
	 printf("%02x:", (unsigned char)reader_open_ans2[i]);
	 }
	 fputs("\n\t  output: ", stdout);
	 for(int i = 0; i < n; i++) {
	 printf("%02x:", (unsigned char)recvBuff[i]);
	 }
	 puts("");*/
	
      logprintf(MODULE, "sleeping for %d seconds\n", OPEN_TIME);
      sleep(OPEN_TIME);

      // CLOSE
      if ( (n = send(sockfd, reader_close_req1, sizeof(reader_close_req1), 0)) <  0) {
	printf("\n Write error\n");
	return 1;
      }
      logprintf(MODULE, "close REQ message 1 written (length %d)\n", n);
	
      if( (n = recv(sockfd, recvBuff, sizeof(reader_close_ans1), 0)) < 0) {
	printf("\n Read error \n");      
      }
      logprintf(MODULE, "close ACK message 1 read (length %d)\n", n);
      recvBuff[n] = 0;
      /*	fputs("\ttemplate: ", stdout);
		for(int i = 0; i < sizeof(reader_close_ans1); i++) {
		printf("%02x:", (unsigned char)reader_close_ans1[i]);
		}
		fputs("\n\t  output: ", stdout);
		for(int i = 0; i < n; i++) {
		printf("%02x:", (unsigned char)recvBuff[i]);
		}
		puts("");*/
	
      if ( (n = send(sockfd, reader_close_req2, sizeof(reader_close_req2), 0)) <  0) {
	logprintf(MODULE, "\n Write error\n");
	return 1;
      }
      logprintf(MODULE, "close REQ message 2 written (length %d)\n", n);
	
      if( (n = recv(sockfd, recvBuff, sizeof(reader_close_ans2), 0)) < 0) {
	printf("\n Read error \n");
      }
      logprintf(MODULE, "close ACK message 2 read (length %d)\n", n);
      /*	recvBuff[n] = 0;
		fputs("\ttemplate: ", stdout);
		for(int i = 0; i < sizeof(reader_close_ans2); i++) {
		printf("%02x:", (unsigned char)reader_close_ans2[i]);
		}
		fputs("\n\t  output: ", stdout);
		for(int i = 0; i < n; i++) {
		printf("%02x:", (unsigned char)recvBuff[i]);
		}
		puts("");
      */
      //return 0;
      
    }
  }
  if(nKey < 0) {
    printf("\n Read error \n");
  } 

  return 0;
}


