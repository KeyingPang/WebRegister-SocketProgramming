// Keying Pang
// USC ID: 8713254080
// ServerC  1 UDP Port Number: 21080
// ServerCS 1 UDP Port Number: 22080
// ServerEE 1 UDP Port Number: 23080
// ServerM 1 UDP Port Number: 24080; 1 TCP Port Number: 25080
// Client 1 TCP Dynamic Ports


#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <map>

#define MAXBUFLENGTH 1024

//Hard coded portnumber
#define serverC_UDP_Port 21080
#define serverCS_UDP_Port 22080
#define serverEE_UDP_Port 23080
#define serverM_UDP_Port 24080
#define serverM_TCP_Port 25080
#define localhost "127.0.0.1"

using namespace std;

struct LoginInputType {
  char unencryptedUsername[50];
  char unencryptedPassword[50];
};

int serverC_sockfd_udp;
string username, password;
struct sockaddr_in serverC_sock_addr, serverM_sock_addr;
struct sockaddr_in newC_sock_addr;
socklen_t newC_sock_size;
map<string, string> credtxt;


//Set the UDP Socket of serverC
//Reused and Modified from Beej's Guide 6.3
void setserverCUDP() {
  serverC_sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
  if (serverC_sockfd_udp == -1) {
    perror("Error on set UDP socket.\n");
    exit(-1);
  }
  
  memset(&serverC_sock_addr, 0, sizeof(serverC_sock_addr));
  serverC_sock_addr.sin_family = AF_INET;
  serverC_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverC_sock_addr.sin_port = htons(serverC_UDP_Port);

  if (bind(serverC_sockfd_udp, (const struct sockaddr *)&serverC_sock_addr, sizeof(serverC_sock_addr)) == -1) {
    perror("Error on binding the UDP socket.\n");
    exit(-1);
  }
}

//Set the UDP Socket for ServerM
//Reused and Modified from Beej's Guide 6.3
void setsocketforServerM() {
  memset(&serverM_sock_addr, 0, sizeof(serverM_sock_addr));
  serverM_sock_addr.sin_family = AF_INET;
  serverM_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverM_sock_addr.sin_port = htons(serverM_UDP_Port);
}

void ReadFile() {
  fstream infile("./cred.txt", ios::in);
  if (!infile.is_open()){
    perror("Fail to open file cred.txt");
    exit(-1);
  }
  string line, username, password;
  while (getline(infile, line)) {
    if (line.length() != 0) {
    int idx = line.find(",");
    //cout << idx << endl;
    username = line.substr(0, idx);
    //Parse the data by ","
    password = line.substr(idx + 1);
    credtxt[username] = password;
    }
  }
}

void checkForLoginData() {
  LoginInputType type_in;
  char returnresult[MAXBUFLENGTH] = {0};
  while(true) {
    //Clear all the buffer
    memset(&type_in, 0, sizeof(type_in));

    int recvsize = recvfrom(serverC_sockfd_udp, &type_in, sizeof(type_in), 0, (struct sockaddr *)&newC_sock_addr, &newC_sock_size);
    if (recvsize <= 0) {
      perror("ServerC receive error");
      exit(-1);
    }
    char* w3 = (char*)"success";
    cout << "The ServerC received an authentication request from the Main Server.\n";
    username = type_in.unencryptedUsername;
    password = type_in.unencryptedPassword;

    //Use iterator and map.find() to memorize the location of the username first appear in the map
    map<string, string>::iterator itr;
    itr = credtxt.find(username);
    if (itr == credtxt.end()) {
      //Username no match
      strcpy(returnresult, "faila");
    } else if (credtxt[username] != password) { 
      //Username existed & Password no match
      strcpy(returnresult, "failb");
    } else { 
      //Username existed & Password match
      strcpy(returnresult, "success");
    }
    int sendsize = sendto(serverC_sockfd_udp, returnresult, sizeof(returnresult), 0, (const struct sockaddr *)&serverM_sock_addr, sizeof(serverM_sock_addr));
    if (sendsize == -1) {
      perror("Fail to send");
      exit(-1);
    }
    
    cout << "The ServerC finished sending the response to the Main Server." << endl;
  }
}

int main() {

  setserverCUDP();
  setsocketforServerM();
  cout << "The ServerC is up and running using UDP on port " << serverC_UDP_Port << endl;
  ReadFile();
  checkForLoginData();
  return 0;
  
}
