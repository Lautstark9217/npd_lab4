#include "server.h"
int main(int argc, char *argv[]) {
    Server server;
    serverClose(&server);
    serverStart(&server);
    return 0;
}