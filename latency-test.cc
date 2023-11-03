#include <string.h>
#include <iostream>
#include <assert.h>
#include <chrono>
#include <zmq.h>
#include <limits.h>
#include <unistd.h>

#define NR_TESTS 100000

using namespace std;

int server() {
  //  Socket to talk to clients
  void *context = zmq_ctx_new ();
  void *responder = zmq_socket (context, ZMQ_REP);
  int rc = zmq_bind (responder, "tcp://*:5555");
  assert (rc == 0);
  
  char buffer [1];
  buffer[0] = 0;
  
  for(int i=0; i<NR_TESTS; i++) {
    zmq_recv (responder, buffer, 1, 0);
    zmq_send (responder, buffer, 1, 0);
  }
  return 0;
}

int client(char* host) {
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  char dest[500];
  sprintf(dest, "tcp://%s:5555", host);
  cout << "Connecting to " << dest << endl;
  zmq_connect (requester, dest);

  char buffer [1];
  buffer[0] = 0;

  auto begin = std::chrono::high_resolution_clock::now();
  for (int i=0; i<NR_TESTS; i++) {
    zmq_send (requester, buffer, 1, 0);
    zmq_recv (requester, buffer, 1, 0);
  }
  auto end = std::chrono::high_resolution_clock::now();

  zmq_close (requester);
  zmq_ctx_destroy (context);

  auto nanos = chrono::duration_cast<chrono::nanoseconds>(end-begin).count();
  //  cout << "total time: " << nanos << " ns" << endl;
  cout << "roundtrip time: " << (nanos / NR_TESTS) << " ns" << endl;

  return 0;
}

int main(int argc, char* arg[]) {

  char hostname[HOST_NAME_MAX];
  char masterName[HOST_NAME_MAX];

  if (gethostname(hostname, HOST_NAME_MAX)) {
    perror("gethostname");
    return EXIT_FAILURE;
  }

  char* nodeList;
  if (! (nodeList = std::getenv("SLURM_NODELIST"))) {
      perror("SLURM nodelist");
      return EXIT_FAILURE;
  }

  // some magic to construct the masterName from the hostlist
  char* start = nodeList;
  strcpy(masterName, "node");
  int index = strlen("node");
  while(!isdigit(*start)) start++;
  while(isdigit(*start)) {
    masterName[index] = *start;
    start++;
    index++;
  }
  masterName[index+1] = 0;

  if(!strcmp(masterName, hostname)) {
    cout << "masterName = " << masterName << ", hostname = " << hostname << ": I am the server" << endl;
    server();
  } else {
    cout << "masterName = " << masterName << ", hostname = " << hostname << ": I am the client" << endl;
    client(masterName);
  }

  return 0;
}
