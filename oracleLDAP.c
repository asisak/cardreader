#define LDAP_DEPRECATED 1
#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "logger.h"
#include "oracle.h"

#define MODULE _rfcontrol_module_ldap
const char *_rfcontrol_module_ldap = "LDAP auth";

const char *ldap_server = "ldap://10.16.50.14";
const char *base_dn = "dc=chemin-neuf, dc=net";
const char *bind_dn = "cn=reader, dc=chemin-neuf, dc=net";
const char *passwd = "netforgod";

const char *groups_base="cn=groups,dc=chemin-neuf,dc=net";
const char *key_base="cn=users,dc=chemin-neuf,dc=net";

// CONFIG FILE
const char *ldap_group = "secu-test";

int decide(unsigned int key)  {
  LDAP *ld;

  logprintf(MODULE, "check access\n");
  if(ldap_initialize(&ld, ldap_server)) {
    perror( "ldap_initialize" );
    return 0;
  }
  int rc = ldap_simple_bind_s(ld, bind_dn, passwd);
  if( rc != LDAP_SUCCESS ) {
    logprintf(MODULE, "ldap_simple_bind_s: %s\n", ldap_err2string(rc));
    return 0;
  }
  logprintf(MODULE, "LDAP authentication successful\n");
  
  LDAPMessage *msg, *entry;
  char key_filter[512];
  char uid[512];
  char *attrs[2];
  attrs[0]=malloc(sizeof(char)*1024);
  attrs[1]=NULL;

  sprintf(key_filter, "(x121Address=%010u)", key);
  if (ldap_search_s(ld, key_base, LDAP_SCOPE_SUBTREE, key_filter, NULL, 0, &msg) != LDAP_SUCCESS) {
    logprintf(MODULE, "LDAP authentication error: %s", ldap_err2string(rc));
  }
  free(attrs[0]);
  int num_entries_returned = ldap_count_entries(ld, msg);
  logprintf(MODULE, "%d LDAP entries returned for RFid %u (%x)\n",  num_entries_returned, key, key);
  if(num_entries_returned == 1) {
    for(entry = ldap_first_entry(ld, msg); entry != NULL; entry = ldap_next_entry(ld, entry)) {
      char *dn = NULL;
      if((dn = ldap_get_dn(ld, entry)) != NULL) {
	// printf("\treturned dn: %s\n", dn);
	strcpy(uid, dn);
	ldap_memfree(dn);
      }      
    }
  }

  char group_filter[512];
  sprintf(group_filter, "(member=%s)", uid);
  char groups_base2[512];
  sprintf(groups_base2, "cn=%s,%s", ldap_group, groups_base);
  logprintf(MODULE, "searching for %s in group %s\n", group_filter, groups_base2);
  if(ldap_search_s(ld, groups_base2, LDAP_SCOPE_SUBTREE, group_filter, NULL, 0, &msg) != LDAP_SUCCESS) {
    logprintf(MODULE, "LDAP search error: %s", ldap_err2string(rc));
  } else {
    num_entries_returned = ldap_count_entries(ld, msg);
    ldap_unbind(ld);
    
    if(num_entries_returned == 1) {
      logprintf(MODULE, "'Speak, friend, and enter!' [access GRANTED]\n");
      return 1;
    }
  }
  logprintf(MODULE, "'You shall not pass!' [access REFUSED]\n");
  return 0;
}
