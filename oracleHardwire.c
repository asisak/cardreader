#include "decider.h"

const int keys[] = {
  0x2ca217, // yellow
  0x8ec877  // blue
};

int decide(int key) {
  for(int i = 0; i < sizeof(keys) / sizeof(int); i++) {
    if(keys[i] == key)
      return 1;
  }
  return 0;
}
