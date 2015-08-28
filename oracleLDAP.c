#include <stdio.h>
#include <sys/types.h>
#include <ldap.h>

#include "decider.h"

const char *ldap_server = "ldap://10.16.50.14";
const char *base_dn = "dc=chemin-neuf, dc=net";
const char *bind_dn = "cn=reader, dc=chemin-neuf, dc=net";
const char *passwd = "netforgod";

const char *ldap_group = "sequ-test";

extern pid_t myPID;

int decide(int key)  {
  fprintf(stderr, "[server auth %d] check access\n", myPID);

  LDAP* ld;
  if(ldap_initialize( &ld, ldap_server)) {
    perror( "ldap_initialize" );
    return 0;
  }
  int rc = ldap_simple_bind_s( ld, bind_dn, passwd);
  if( rc != LDAP_SUCCESS ) {
    fprintf(stderr, "[server auth %d] ldap_simple_bind_s: %s\n", myPID, ldap_err2string(rc));
    return 0;
  }
  printf( "Successful authentication\n");
  
  // GET TO GROUP
  
  
  // char *uid = "asisak";
  
  //char *filter;
  //sprintf(filter, "(&(objectClass=posixGroup)(memberOf=CN=%s))", uid);
  // iterate over filter --> 

  ldap_unbind(ld);

  // fprintf(stderr, "[server auth %d] access GRANTED\n", myPID);
  // return 1;
  
  fprintf(stderr, "[server auth %d] access REFUSED\n", myPID);
  return 0;
}
