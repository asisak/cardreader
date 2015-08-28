// 0001925623 0x001d61f7
// --> 
// 2925079 0x002ca217

// wg34 wg26

#include <stdio.h>
#include <string.h>



const char *byte_to_binary(int x) {
    static char b[9];
    b[0] = '\0';
    for(int z = 128; z > 0; z >>= 1) {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    return b;
}

int main(void) {
  unsigned char key[3] = {0x2c, 0xa2, 0x17};
  unsigned char key2[3] = {0x1d, 0x61, 0xf7};
  
  printf("%d\n", 0x002ca217);
  printf("%d\n", 0x001d61f7);
  
  for(int i = 0; i < sizeof(key)/sizeof(char); i++) {
    printf("%s ", byte_to_binary(key[i]));
  }
  printf("\n");

  for(int i = 0; i < sizeof(key2)/sizeof(char); i++) {
    printf("%s ", byte_to_binary(key2[i]));
  }
  printf("\n");

  return 0;
}
