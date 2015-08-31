/* ========================================================================= */
/* oracleHardwire.c -- rfcontrol 'oracle' hard-wired implementation          */
/* ------------------------------------------------------------------------- */
/*   the keys listed in this file can enter                                  */
/* ========================================================================= */
#include "oracle.h"

const unsigned int keys[] = {
  0x2ca217, // yellow
  0x8ec877  // blue
};

/* ========================================================================= */
/* int decide(unsigned int key);                                             */
/* ------------------------------------------------------------------------- */
/* returns non-zero if 'key' shall pass / 0 if 'key' shall not pass          */
/* ========================================================================= */
int decide(unsigned int key) {
  for(int i = 0; i < sizeof(keys) / sizeof(int); i++) {
    if(keys[i] == key)
      return 1; // O.k.
  }
  // THOU SHALL NOT PASS!
  return 0;
}
