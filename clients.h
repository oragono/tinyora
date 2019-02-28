struct ClientInfo {
  char nick[NICKLEN];
  char user[USERLEN];
  char host[HOSTLEN];
  char realname[REALNAMELEN];
  char buffer[BUFFERLEN];
};
