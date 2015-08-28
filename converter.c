
// wg34 wg26

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *byte_to_binary(int x) {
    static char b[9];
    b[0] = '\0';
    for(int z = 128; z > 0; z >>= 1) {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    return b;
}

int main(void) {
  // 001925623 --> continue
  // 029 (8 bit) 25079 (16 bit)

// 0001925623 0x00 1d 61 f7
// --> 
// 2925079 0x 00 2c a2 17
  printf("%04x%04x\n", 29, 25079);
  printf("%08x\n", 1925623);
  int a = 0x2ca217;
  char s[8];
  sprintf(s, "%04x%04x", a / 100000, a % 100000);
  unsigned int b = (unsigned int)strtoul(s, NULL, 16);
  printf("%08x (%010u)\n", b, b);
  /*
  unsigned char key2[3] = {0x17, 0xa2, 0x2c};
  unsigned char key3[3] = {0x1d, 0x61, 0xf7};
  unsigned char key4[3] = {0x16, 0x51, 0x0b};
  unsigned int k0 = 0x002ca217;
  unsigned int k1 = 0x00000000;
  unsigned int cardID = (k0 & 0x1FFFFFE) >>1;

		k1 = k1 & 0x03;				// only need the 2 LSB of the codehigh
		k1 <<= 30;				// shift 2 LSB to MSB		
		k0 >>=1;
		unsigned int cardID = k1 | k0;

  printf("%u 0x%x\n", 0x002ca217, 0x002ca217);
  printf("%u 0x%x\n", 0x0017a22c, 0x0017a22c);
  printf("%u 0x%x\n", 0x001d61f7, 0x001d61f7);
  printf("%u 0x%x\n", cardID, cardID);
  
  for(int i = 0; i < sizeof(key)/sizeof(char); i++) {
    printf("%s ", byte_to_binary(key[i]));
  }
  printf("\n");

  for(int i = 0; i < sizeof(key2)/sizeof(char); i++) {
    printf("%s ", byte_to_binary(key2[i]));
  }
  printf("\n");

  for(int i = 0; i < sizeof(key3)/sizeof(char); i++) {
    printf("%s ", byte_to_binary(key3[i]));
  }
  printf("\n");

  for(int i = 0; i < sizeof(key4)/sizeof(char); i++) {
    printf("%s ", byte_to_binary(key4[i]));
  }
  printf("\n");
*/

  return 0;
}
