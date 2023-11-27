#include <string.h>
#include <iostream>
#include <assert.h>
#include <chrono>
#include <zmq.h>
#include <limits.h>
#include <unistd.h>

#define NR_TESTS 100
#define MAX_BUFFER (1024*1024)

using namespace std;

int server() {
  //  Socket to talk to clients
  void *context = zmq_ctx_new();
  assert (context);
  void *responder = zmq_socket(context, ZMQ_REP);
  assert (responder);
  assert(zmq_bind(responder, "tcp://*:5555") == 0);
  
  char* buffer = new char[MAX_BUFFER];
  int size = 0;
  int nrTests = 0;
  
  while(true) {
    int rc = zmq_recv(responder, &size, sizeof(int), 0);
    if(rc) {
      cout << "recv failed rc = " << rc << endl;
      perror ("oops: ");
      exit(1);
    }

    if(size < 0) {
      delete[] buffer;
      return 0;
    }

    assert(zmq_recv(responder, &nrTests, sizeof(int), 0) == 0);

    cout << "server: running test with size " << size << " and " << nrTests << " iterations" << endl;
    return 0;
    /*
    for(int i=0; i<nrTests; i++) {
      zmq_recv(responder, buffer, size, 0);
      zmq_send(responder, buffer, size, 0);
    }
    */
  }
}

int runTest(int size, int nrTests, void* requester) {
  auto buffer = new char[size];

  cout << "client: size = " << size << " nrTests = " << nrTests << endl;

  zmq_send(requester, &size, sizeof(int), 0);
  zmq_send(requester, &nrTests, sizeof(int), 0);

  auto begin = std::chrono::high_resolution_clock::now();
  for (int i=0; i<nrTests; i++) {
    zmq_send(requester, buffer, size, 0);
    zmq_recv(requester, buffer, size, 0);
  }
  auto end = std::chrono::high_resolution_clock::now();

  auto nanos = chrono::duration_cast<chrono::nanoseconds>(end-begin).count();
  double throughput = (((double) (nrTests * size)) / (1024.0 * 1024.0 * 1024.0)) / (nanos * 1000000000.0);

  //  cout << "total time: " << nanos << " ns" << endl;
  cout << "size: " << size << " roundtrip time: " << (nanos / nrTests) << " ns throughput " << throughput << " GB/s" << endl;

  delete[] buffer;
  return 0;
}

int client(char* host) {
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  char dest[HOST_NAME_MAX + 500];
  sprintf(dest, "tcp://%s:5555", host);
  cout << "Connecting to " << dest << endl;
  zmq_connect(requester, dest);

  runTest(1, NR_TESTS, requester);

  // send exit signal to the server
  int tmp = -1;
  zmq_send(requester, &tmp, sizeof(int), 0);

  zmq_close(requester);
  zmq_ctx_destroy(context);

  return 0;
}

int getmastername(char* mastername) {
  char* nodeList;
  if (! (nodeList = std::getenv("SLURM_NODELIST"))) {
      perror("SLURM nodelist");
      return EXIT_FAILURE;
  }

  // some magic to construct the mastername from the hostlist
  char* start = nodeList;
  strcpy(mastername, "node");
  int index = strlen("node");
  while(!isdigit(*start)) start++;
  while(isdigit(*start)) {
    mastername[index] = *start;
    start++;
    index++;
  }
  mastername[index+1] = 0;

  return 0;
}

int main(int argc, char* args[]) {

  char hostname[HOST_NAME_MAX];
  char mastername[HOST_NAME_MAX];

  if (gethostname(hostname, HOST_NAME_MAX)) {
    perror("gethostname");
    return EXIT_FAILURE;
  }

  if(argc > 1) {
    // running locally, for testing purposes
    cout << "local run" << endl;
    strcpy(mastername, hostname);

    int IamServer = atoi(args[0]);
    if(IamServer) {
      cout << "mastername = " << mastername << ", hostname = " << hostname << ": I am the server" << endl;
      server();
    } else {
      cout << "mastername = " << mastername << ", hostname = " << hostname << ": I am the client" << endl;
      client(mastername);
    }
  } else {
    if (getmastername(mastername)) {
      perror("getmastername");
      return EXIT_FAILURE;
    }  
  }

  if(!strcmp(mastername, hostname)) {
    cout << "mastername = " << mastername << ", hostname = " << hostname << ": I am the server" << endl;
    server();
  } else {
    cout << "mastername = " << mastername << ", hostname = " << hostname << ": I am the client" << endl;
    client(mastername);
  }

  return 0;
}
