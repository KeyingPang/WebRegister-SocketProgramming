// Keying Pang
// USC ID: 8713254080
// ServerC  1 UDP Port Number: 21080
// ServerCS 1 UDP Port Number: 22080
// ServerEE 1 UDP Port Number: 23080
// ServerM 1 UDP Port Number: 24080; 1 TCP Port Number: 25080
// Client 1 TCP Dynamic Ports


#include <string> 
#include <cstring>
#include <sstream>
#include <iostream> 
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <thread>

using namespace std;

//Allow user to input username and password for three times
#define MAXATTEMPTTIMES 3

#define MAXBUFLENGTH 1024

//Hard coded portnumber
#define serverC_UDP_Port 21080
#define serverCS_UDP_Port 22080
#define serverEE_UDP_Port 23080
#define serverM_UDP_Port 24080
#define serverM_TCP_Port 25080
#define localhost "127.0.0.1"

//Define input and output message
struct LoginInputType {
  char unencryptedusername[50];
  char unencryptedpassword[50];
};


// define the course request/outputonse message format
struct RequestInputType {
  char inputcourse[50];
  char inputcategory[50];
};

// global information of a client
string username;
unsigned int assignedtcpport;
int client_sockfd_tcp;
bool loginsuccess;
struct sockaddr_in serverM_sock_addr;
struct sockaddr_in client_sock_addr;
socklen_t client_sock_len;

//Set socket from Beej's guide
//Reused and Modified from Beej's Guide 6.2
void setSocketTCP() {
  client_sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sockfd_tcp == -1) {
    perror("Error on creating socket");
    exit(-1);
  }
  if ((connect(client_sockfd_tcp, (struct sockaddr *)&serverM_sock_addr, sizeof(serverM_sock_addr))) == -1) {
    perror("Error on connecting!");
    exit(0);
  }  
}
//Reused and Modified from Beej's Guide 6.2
void setSocketServerM() {
  memset(&serverM_sock_addr, 0, sizeof(serverM_sock_addr));
  serverM_sock_addr.sin_family = AF_INET;
  serverM_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverM_sock_addr.sin_port = htons(serverM_TCP_Port);
}


// Phase 1 & 2: Login Part
//void LoginChecking(){}  if success then phase 3 & 4
void LoginChecking() {
  LoginInputType type_in;
  char databuffer[MAXBUFLENGTH];
  int numberofattempt = 0;
  loginsuccess = false;
  char result1[MAXBUFLENGTH];
  char* w1 = (char*)"faila"; //Username no match
  char* w2 = (char*)"failb"; //Username existed & Password no match
  char* w3 = (char*)"success"; //Username existed & Password match

  while (true) {
    if (++numberofattempt > (int)MAXATTEMPTTIMES) {
      cout << "\n" << "Authentication Failed for 3 attempts. Client will shut down." << endl;
      close(client_sockfd_tcp);
      exit(1);
    }
    memset(&type_in, 0, sizeof(type_in));
    memset(databuffer, 0, sizeof(databuffer));

    cout << "Please enter the username: " << endl;
    cin.getline(type_in.unencryptedusername, sizeof(type_in.unencryptedusername));
    cout << "Please enter the password: " << endl;
    cin.getline(type_in.unencryptedpassword, sizeof(type_in.unencryptedpassword));
    //Store Username
    username = type_in.unencryptedusername;
    //Send login request message to serverM
    int sendlen = send(client_sockfd_tcp, &type_in, sizeof(type_in), 0);
    if (sendlen == -1) {
      perror("Client error on sending username and password.");
      exit(-1);
    }
    cout << username << " sent an authentication request to the main server." << endl;
    //Receiver authentication outputonse from main server
    int recvsize = recv(client_sockfd_tcp, databuffer, sizeof(databuffer), 0);
    if (recvsize == -1) {
      perror("Client error on receiving the username and password result.");
      exit(-1);
    }
    strcpy(result1, databuffer);
    cout << username << " received the result of authentication using TCP over port " << assignedtcpport << ". ";

    if (strcmp(result1, w3) == 0) {
      cout << "Authentication is successful" << endl;
      loginsuccess = true;
      break;
    } else if (strcmp(result1, w2) == 0) {
      cout << "Authentication failed: Password does not match" << endl;
    } else if (strcmp(result1, w1) == 0) {
      cout << "Authentication failed: Username Does not exist" << endl;
    } else {
      cout << "Unknown response." << endl;
    }
    cout << "Attempts remaining: " << MAXATTEMPTTIMES - numberofattempt << endl;
  }
  //cout << "\n" << "Authentication Failed for 3 attempts. Client will shut down." << endl;
}

// Phase 3 & 4: Query Part
void checkCourses() {
  RequestInputType input;
  char databuffer[MAXBUFLENGTH];
  char result2[MAXBUFLENGTH];
  char* w4 = (char*)"false";
  char* w5 = (char*)"categoryfail";

  memset(&input, 0, sizeof(input));
  memset(databuffer, 0, sizeof(databuffer));

  cout << "Please enter the course code to query: " << endl;
  cin.getline(input.inputcourse, sizeof(input.inputcourse));
  cout << "Please enter the category(Credit / Professor / Days / CourseName): " << endl;
  cin.getline(input.inputcategory, sizeof(input.inputcategory));

  // send course request message to main server
  int sendsize = send(client_sockfd_tcp, &input, sizeof(input), 0);
  if (sendsize <= 0) {
    perror("Error on send message");
    exit(-1);
  }
  cout << "\n" << username << " sent a request to the main server." << endl;

  // recv from main server
  int recvsize = recv(client_sockfd_tcp, databuffer, sizeof(databuffer), 0);
  if (recvsize < 0) {
    perror("Error on receiving the response from department server.");
    exit(-1);
  }
  strcpy(result2, databuffer);

  cout << "\n" << "The client received the response from the Main server using TCP over port " << assignedtcpport << "." << endl;
  if (strcmp(result2, w4) == 0) {
    cout << "\n" << "Didn't find the course: " << input.inputcourse << endl;
  } else if (strcmp(result2, w5) == 0) {
    cout << "\n" << "Didn't find the category: " << input.inputcategory << endl;
  } else {
    cout << "The " << input.inputcategory << " of " << input.inputcourse << " is " << result2 << "." << endl;
  }
  cout << "\n" << "\n" << "\n" << endl;
  cout << "-----Start a new request-----" << endl;
  
}

int main() {
  //Creat socket and connect
  setSocketServerM();
  setSocketTCP();
  cout << "The client is up and running." << endl;

  signal(SIGPIPE, SIG_IGN);
  //Get TCP port number(Get dynamic assigned port number code reused from piazza)
  memset(&client_sock_addr, 0, sizeof(client_sock_addr));
  client_sock_len = sizeof(client_sock_addr);
  int getsock_check = getsockname(client_sockfd_tcp, (struct sockaddr *)&client_sock_addr, &client_sock_len);
  if (getsock_check == -1) {
    perror("Getsockname error!");
    exit(-1);
  }
  assignedtcpport = ntohs(client_sock_addr.sin_port);
  
  //Phase 1 & 2
  LoginChecking();

  //If phase 1 & 2 success
  while (loginsuccess) {
    //phase 3 & 4
    checkCourses();
  }
  return 0;
}