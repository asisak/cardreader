#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define _BSD_SOURCE
#include <endian.h>

#include "reader.h"
#include "oracle.h"

#define SERVER_UDP_PORT 60000
#define SERVER_IP_ADDR "192.168.1.100"

#define TIMEOUT 30
#define OPEN_TIME 3

extern pid_t myPID;

int reverseEndian(char* buf, int n) {
  if(n%4) return 1;
  for (int i = 0; i != n; i +=4) {
    char i0 = buf[i], i1 = buf[i+1], i2 = buf[i+2], i3 = buf[i+3];
    buf[i] = i3;
    buf[i+1] = i2;
    buf[i+2] = i1;
    buf[i+3] = i0;
  }
  return 0;
}

int UDPSocketClient(void) {
  int sockfd = 0, n = 0, nKey = 0;
  char recvBuff[2048];
  char recvBuffKey[2048];
  struct sockaddr_in serv_addr; 

  memset(recvBuff, '0',sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("\n Error : Could not create socket \n");
    return 1;
  } 
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
  fprintf(stderr, "[server socket %d] socket open\n", myPID);

  // ENABLE MONITOR MODE
  if ( (n = write(sockfd, reader_monitor_req, sizeof(reader_monitor_req))) <  0) {
    printf("\n Write error\n");
    return 1;
  }
  fprintf(stderr, "[server socket %d] monitor REQ message written (length %d)\n", myPID, n);
  if( (n = read(sockfd, recvBuff, sizeof(reader_monitor_ans))) < 0) {
    printf("\n Read error \n");      
  }
  fprintf(stderr, "[server socket %d] monitor ACK message read (length %d)\n", myPID, n);
  recvBuff[n] = 0;
  reverseEndian(reader_monitor_ans, sizeof(reader_monitor_ans));
  reverseEndian(recvBuff, n);
  fputs("\ttemplate: ", stdout);
  for(int i = 0; i < sizeof(reader_monitor_ans); i++) {
    printf("%02x:", (unsigned char)reader_monitor_ans[i]);
  }
  fputs("\n\t  output: ", stdout);
  for(int i = 0; i < n; i++) {
    printf("%02x:", (unsigned char)recvBuff[i]);
  }
  puts("");

  // MONITORING CARD READER
  // FIXME: 30 seconds restart
  /* WHY DOES IT READ EVEN AFTER??? */
  fprintf(stderr, "[server socket %d] waiting for badges to pass\n", myPID);
  while ( (nKey = read(sockfd, recvBuffKey, sizeof(reader_badge_pass))) > 0) {
    fprintf(stderr, "[server socket %d] badge pass message read (length %d)\n", myPID, nKey);
    /*      
    // DUMP DATA READ
    fputs("\t  output: ", stdout);
    for(int i = 0; i < n; i++) {
    printf("%02x:", (unsigned char)recvBuff[i]);
    }
    puts("");
    */
    //unsigned int key = 2925079; // yellow
    //unsigned int key = 9357431; // blue
    //sprintf(keys, "%c%c%c", 0x17, 0xa2, 0x2c); // yellow
    //sprintf(keys, "%c%c%c", 0x77, 0xc8, 0x8e); // blue
    char keys[5];
    reverseEndian(recvBuffKey, nKey);
    memcpy(keys, recvBuffKey+68, 4);
    keys[5]=0;
    unsigned int a = ((unsigned char)keys[3] +  (unsigned char)keys[2]*0x100) +  0x10000*((unsigned char)keys[1] +  (unsigned char)keys[0]*0x100);
    char s[8];
    sprintf(s, "%04x%04x", a / 100000, a % 100000);
    unsigned int key = (unsigned int)strtoul(s, NULL, 16);
    fprintf(stderr, "[server socket %d] badge key %010u (0x%x) passed\n", myPID, key, key);
    /*
      fputs("\t  output:", stdout);
      for(int i = 0; i < n; i++) {
      if(!(i%20)) printf("\n\t");
      printf("%02x:", (unsigned char)recvBuffKey[i]);
      }
      puts("");
    */
      
    if(decide(key)) {
      // OPEN
      if ( (n = write(sockfd, reader_open_req1, sizeof(reader_open_req1))) <  0) {
	printf("\n Write error\n");
	return 1;
      }
      fprintf(stderr, "[server socket %d] open REQ message 1 written (length %d)\n", myPID, n);

      if( (n = read(sockfd, recvBuff, sizeof(reader_open_ans1))) < 0) {
	printf("\n Read error \n");      
      }
      fprintf(stderr, "[server socket %d] open ACK message 1 read (length %d)\n", myPID, n);
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
	
      if ( (n = write(sockfd, reader_open_req2, sizeof(reader_open_req2))) <  0) {
	printf("\n Write error\n");
	return 1;
      }
      fprintf(stderr, "[server socket %d] open REQ message 2 written (length %d)\n", myPID, n);
	
      if( (n = read(sockfd, recvBuff, sizeof(reader_open_ans2))) < 0) {
	printf("\n Read error \n");
      }
      fprintf(stderr, "[server socket %d] open ACK message 2 read (length %d)\n", myPID, n);
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
	
      fprintf(stderr, "[server socket %d] sleeping for %d seconds\n", myPID, OPEN_TIME);
      sleep(OPEN_TIME);

      // CLOSE
      if ( (n = write(sockfd, reader_close_req1, sizeof(reader_close_req1))) <  0) {
	printf("\n Write error\n");
	return 1;
      }
      fprintf(stderr, "[server socket %d] close REQ message 1 written (length %d)\n", myPID, n);
	
      if( (n = read(sockfd, recvBuff, sizeof(reader_close_ans1))) < 0) {
	printf("\n Read error \n");      
      }
      fprintf(stderr, "[server socket %d] close ACK message 1 read (length %d)\n", myPID, n);
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
	
      if ( (n = write(sockfd, reader_close_req2, sizeof(reader_close_req2))) <  0) {
	printf("\n Write error\n");
	return 1;
      }
      fprintf(stderr, "[server socket %d] close REQ message 2 written (length %d)\n", myPID, n);
	
      if( (n = read(sockfd, recvBuff, sizeof(reader_close_ans2))) < 0) {
	printf("\n Read error \n");
      }
      fprintf(stderr, "[server socket %d] close ACK message 2 read (length %d)\n", myPID, n);
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
      return 0;
    }
  }
  if(nKey < 0) {
    printf("\n Read error \n");
  } 

  return 0;
}


