#define LDAP_DEPRECATED 1
#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "oracle.h"

const char *ldap_server = "ldap://10.16.50.14";
const char *base_dn = "dc=chemin-neuf, dc=net";
const char *bind_dn = "cn=reader, dc=chemin-neuf, dc=net";
const char *passwd = "netforgod";

const char *groups_base="cn=groups,dc=chemin-neuf,dc=net";
const char *key_base="cn=users,dc=chemin-neuf,dc=net";

// CONFIG FILE
const char *ldap_group = "secu-test";

extern pid_t myPID;

int decide(int key)  {
  LDAP *ld;

  fprintf(stderr, "[server auth %d] check access\n", myPID);

  if(ldap_initialize(&ld, ldap_server)) {
    perror( "ldap_initialize" );
    return 0;
  }
  int rc = ldap_simple_bind_s(ld, bind_dn, passwd);
  if( rc != LDAP_SUCCESS ) {
    fprintf(stderr, "[server auth %d] ldap_simple_bind_s: %s\n", myPID, ldap_err2string(rc));
    return 0;
  }
  fprintf(stderr, "[server auth %d] LDAP authentication successful\n", myPID);
  
  LDAPMessage *msg, *entry;
  char *key_filter = malloc(2048);
  char uid[2048];
  char *attrs[2];
  attrs[0]=malloc(sizeof(char)*1024);
  attrs[1]=NULL;
  const char *keyString = "0001925623";

  sprintf(key_filter, "(x121Address=%s)", keyString); // FIXME
  if (ldap_search_s(ld, key_base, LDAP_SCOPE_SUBTREE, key_filter, NULL, 0, &msg) != LDAP_SUCCESS) {
     fprintf(stderr, "[server auth %d] LDAP authentication error: ", myPID);
    ldap_perror(ld, "ldap_search_s");
  }
  free(attrs[0]);
  fprintf(stderr, "[server auth %d] %d LDAP entries returned for RFid %s\n",  myPID, ldap_count_entries(ld, msg), keyString);
  for(entry = ldap_first_entry(ld, msg); entry != NULL; entry = ldap_next_entry(ld, entry)) {
    char *dn = NULL;
    if((dn = ldap_get_dn(ld, entry)) != NULL) {
      printf("\treturned dn: %s\n", dn);
      strcpy(uid, dn);
      // DOES dn belong to GROUP?
      ldap_memfree(dn);
    }      
  }
  free(key_filter);

  // GO FOR THE GROUP
  char *group_filter = malloc(2048);
  sprintf(group_filter, "(member=%s)", uid);
  char groups_base2[512];
  sprintf(groups_base2, "cn=%s,%s", ldap_group, groups_base);
  fprintf(stderr, "[server auth %d] searching for %s in group %s", myPID, group_filter, groups_base2);
  if (ldap_search_s(ld, groups_base2, LDAP_SCOPE_SUBTREE, group_filter, NULL, 0, &msg) != LDAP_SUCCESS) {
    ldap_perror( ld, "ldap_search_s");
  } else {
    fprintf(stderr, "\n");
  }
  int num_entries_returned = ldap_count_entries(ld, msg);
  fprintf(stderr, "\t%d entries returned\n", ldap_count_entries(ld, msg));
  free(group_filter);
  ldap_unbind(ld);

  if(num_entries_returned == 1) {
    fprintf(stderr, "[server auth %d] 'Speak, friend, and enter!' [access GRANTED]\n", myPID);
    return 1;
  }
  fprintf(stderr, "[server auth %d] 'You shall not pass!' [access REFUSED]\n", myPID);
  return 0;
}
