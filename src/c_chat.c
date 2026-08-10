#include "../include/c_chat.h"

user_t *create_user(const char *username, const char *ip) {
  if (!username || !ip)
    return NULL;

  user_t *user = malloc(sizeof(user_t));

  if (!user)
    return NULL;

  strcpy(user->username, username);
  strcpy(user->ip, ip);

  return user;
}

void delete_user(user_t *user) {
  if (user)
    free(user);
}