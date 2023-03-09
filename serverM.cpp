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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <thread>

#define MAXBUFLENGTH 1024

//Hard coded portnumber
#define serverC_UDP_Port 21080
#define serverCS_UDP_Port 22080
#define serverEE_UDP_Port 23080
#define serverM_UDP_Port 24080
#define serverM_TCP_Port 25080
#define BACKLOG 5
#define localhost "127.0.0.1"

using namespace std;

//Use in Phase 1 & 2
struct LoginInputType {
  char unencryptedusername[50];
  char unencryptedpassword[50];
};

//Use in Phase 3 & 4
struct RequestInputType {
  char inputcourse[50];
  char inputcategory[50];
};

string inputusername;
string inputcoursecode;
string inputdepartment;
bool loginsuccess;
char liresult[MAXBUFLENGTH];
char ccresult[MAXBUFLENGTH];
char bufferforServerC[MAXBUFLENGTH];
char bufferforDepartment[MAXBUFLENGTH];
int client_sockfd, serverM_sockfd_tcp, serverM_sockfd_udp;
//Set
struct sockaddr_in serverM_tcp_addr, serverM_udp_addr, serverC_sock_addr, serverCS_sock_addr, serverEE_sock_addr;
//Phase 1 & 2 use
struct sockaddr_in tmp_addr;
socklen_t tmp_len;
//Phase 3 & 4 use
socklen_t serverCS_sock_len;
socklen_t serverEE_sock_len;
//Main use
struct sockaddr_in client_sock_addr;
socklen_t client_sock_len;

//Set the TCP Socket for serverM
//Reused and Modified from Beej's Guide 6.1
void setSocketTCP() {
  
  serverM_sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
  if (serverM_sockfd_tcp == -1) {
    perror("Error on set TCP socket.\n");
    exit(-1);
  }

  memset(&serverM_tcp_addr, 0, sizeof(serverM_tcp_addr));
  serverM_tcp_addr.sin_family = AF_INET;
  serverM_tcp_addr.sin_addr.s_addr = inet_addr(localhost);
  serverM_tcp_addr.sin_port = htons(serverM_TCP_Port);

  if (bind(serverM_sockfd_tcp, (struct sockaddr *)&serverM_tcp_addr, sizeof(struct sockaddr)) == -1) {
    perror("Error on binding the TCP socket.");
    exit(-1);
  }
  if (listen(serverM_sockfd_tcp, BACKLOG) == -1) {
    perror("Error on listen.\n");
    exit(-1);
  }

  client_sock_len = sizeof(client_sock_addr);
  client_sockfd = accept(serverM_sockfd_tcp, (struct sockaddr *)&client_sock_addr, &client_sock_len);
}

//Set the UDP Socket for serverM
//Reused and Modified from Beej's Guide 6.3
void setSocketUDP() {

  serverM_sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
  if (serverM_sockfd_udp == -1) {
    perror("Error on set UDP socket.\n");
    exit(-1);
  }
  
  memset(&serverM_udp_addr, 0, sizeof(serverM_udp_addr));
  serverM_udp_addr.sin_family = AF_INET;
  serverM_udp_addr.sin_addr.s_addr = inet_addr(localhost);
  serverM_udp_addr.sin_port = htons(serverM_UDP_Port);

  if (bind(serverM_sockfd_udp, (struct sockaddr *)&serverM_udp_addr, sizeof(serverM_udp_addr)) == -1) {
    perror("Error on binding the UDP socket.\n");
    exit(-1);
  }

}

//Set the UDP Socket for ServerC & CS & EE
//Reused and Modified from Beej's Guide 6.3
void setSocketForServerCCSEE() {
  memset(&serverC_sock_addr, 0, sizeof(serverC_sock_addr));
  serverC_sock_addr.sin_family = AF_INET;
  serverC_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverC_sock_addr.sin_port = htons(serverC_UDP_Port);

  memset(&serverCS_sock_addr, 0, sizeof(serverCS_sock_addr));
  serverCS_sock_addr.sin_family = AF_INET;
  serverCS_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverCS_sock_addr.sin_port = htons(serverCS_UDP_Port);

  memset(&serverEE_sock_addr, 0, sizeof(serverEE_sock_addr));
  serverEE_sock_addr.sin_family = AF_INET;
  serverEE_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverEE_sock_addr.sin_port = htons(serverEE_UDP_Port);
  
}


// Encryption of encrypted username and password
string encryptionOfINPUT(const string &inputdata) {
  string afterencryption(inputdata);
  for (int i = 0; i < inputdata.size(); ++i) {
    //Character lowercase
    if (islower(afterencryption[i]) != 0) {
      afterencryption[i] = 'a' + (inputdata[i] - 'a' + 4) % 26;
    } 
    //Character uppercase
    else if (isupper(afterencryption[i]) != 0) {
      afterencryption[i] = 'A' + (inputdata[i] - 'A' + 4) % 26;
    } 
    //Digit case
    else if (isdigit(afterencryption[i]) != 0) {
      afterencryption[i] = '0' + (inputdata[i] - '0' + 4) % 10;
    }
  }
  return afterencryption;
}


//Phase 1 & 2
void checkLoginSuccess(int tmp_sock_fd) {
  LoginInputType type_in;
  char* w3 = (char*)"success";
  loginsuccess = false;
  //int temp = 0; //Login less or equal 3 times
  while (true) {
    memset(bufferforServerC, 0, sizeof(bufferforServerC));
    memset(&type_in, 0, sizeof(type_in));

    int recvsize = recv(tmp_sock_fd, &type_in, sizeof(type_in), 0);
    if (recvsize == -1) {
      perror("Error on receiving");
      continue;
      //break;
    }

    memset(&inputusername, 0,sizeof(inputusername));
    inputusername = type_in.unencryptedusername;
    cout << "The main server received the authentication for " << inputusername << " using TCP over port " << serverM_TCP_Port << endl;

    
    string encryptedinputusername = encryptionOfINPUT(type_in.unencryptedusername);
    string encryptedpassword = encryptionOfINPUT(type_in.unencryptedpassword);
    memset(&type_in, 0, sizeof(type_in));
    strcpy(type_in.unencryptedusername, encryptedinputusername.c_str());
    strcpy(type_in.unencryptedpassword, encryptedpassword.c_str());

    // Send to serverC
    int sendsize = sendto(serverM_sockfd_udp, &type_in, sizeof(type_in), 0,
          (const sockaddr *)&serverC_sock_addr, sizeof(serverC_sock_addr));
    if (sendsize == -1) {
      perror("Error on sending Phase 1 & 2");
      continue;
      //exit(-1);
      //break;
    }
    cout << "The main server sent an authentication request to serverC." << endl;
    
    // Receive authentication result from serverC
    int recvsize1 = recvfrom(serverM_sockfd_udp, bufferforServerC, sizeof(bufferforServerC), 0, (struct sockaddr *)&tmp_addr, &tmp_len);
    if (recvsize1 == -1) {
      perror("Error on receiving Phase 1 & 2");
      exit(-1);
    }
    cout << "The main server received the result of the authentication request from ServerC "
              << "using UDP over port " << serverM_UDP_Port << "\n";
    strcpy(liresult, bufferforServerC);

    int sendsize1 = send(tmp_sock_fd, &liresult, sizeof(liresult), MSG_NOSIGNAL);
    if (sendsize1 == -1) {
      perror("Error on sending login decision.");
      continue;
    }
    cout << "The main server sent the authentication result to the client." << endl;
    if (strcmp(liresult, w3) == 0) {
      loginsuccess = true;
      break;
      //return loginsuccess;
    }
  }
  //loginsuccess = false;
  //return loginsuccess;

}


//Phase 3 & 4
void checkCourseResult(int tmp_sock_fd) {
  RequestInputType input;
  
  while (true) {
    memset(&input, 0, sizeof(input));
    //Receive the coursecode and category from the client
    int recvsize = recv(tmp_sock_fd, &input, sizeof(input), 0);
    if (recvsize == -1) {
      perror("Error in receiving the input");
      break;
    }
    string checkcoursecode = input.inputcourse;
    string checkcategory = input.inputcategory;
    cout << "The main server received from " << inputusername << " to query course " 
              << checkcoursecode << " about " << checkcategory << " using TCP over port "
              << serverM_TCP_Port << "." << endl;

    //Get the result
    memset(bufferforDepartment, 0, sizeof(bufferforDepartment));

    //Decide which department to send the message to
    if (checkcoursecode.find("CS") != string::npos) {
      inputdepartment = "CS";

      //Send the data to the department server
      int sendlen = sendto(serverM_sockfd_udp, &input, sizeof(input), 0, (const struct sockaddr *)&serverCS_sock_addr, sizeof(serverCS_sock_addr));
      if (sendlen == -1) {
        perror("Error on sending Phase 3 & 4");
        exit(-1);
      }
      cout << "The main server sent a request to server" << inputdepartment << endl;

      //Receive the result from the department server
      int recvlen = recvfrom(serverM_sockfd_udp, bufferforDepartment, sizeof(bufferforDepartment), 0, (struct sockaddr *)&serverCS_sock_addr, &serverCS_sock_len);
      if (recvlen < 0) {
        perror("Error on recving Phase 3 & 4");
        exit(-1);
      }
      cout << "The main server received the response from server" << inputdepartment
                << " using UDP over port " << serverM_UDP_Port << endl;
      strcpy(ccresult, bufferforDepartment);
    } else if (checkcoursecode.find("EE") != string::npos){
      inputdepartment = "EE";
      
      //Send the data to the department server
      int sendlen = sendto(serverM_sockfd_udp, &input, sizeof(input), 0, (const struct sockaddr *)&serverEE_sock_addr, sizeof(serverEE_sock_addr));
      if (sendlen == -1) {
        perror("Error on sending Phase 3 & 4");
        exit(-1);
      }
      cout << "The main server sent a request to server" << inputdepartment << endl;

      //Receive the result from the department server
      int recvlen = recvfrom(serverM_sockfd_udp, bufferforDepartment, sizeof(bufferforDepartment), 0, (struct sockaddr *)&serverEE_sock_addr, &serverEE_sock_len);
      if (recvlen < 0) {
        perror("Error on recving Phase 3 & 4");
        exit(-1);
      }
      cout << "The main server received the response from server" << inputdepartment
                << " using UDP over port " << serverM_UDP_Port << endl;
      strcpy(ccresult, bufferforDepartment);
    } else {
      strcpy(ccresult, "false");
      cout << "Input coursecode error." << endl;
      //break;
    }

    //Send the result to the client
    int sendsize = send(tmp_sock_fd, ccresult, sizeof(ccresult), 0);
    if (sendsize  == -1) {
      perror("Error sending the output");
      exit(-1);
    }
    cout << "The main server sent the query information to the client." << endl;
  }
}

int main() {
  cout << "The main server is up and running" << endl;
  setSocketTCP();
  setSocketUDP();
  setSocketForServerCCSEE();
  //cout << "The main server is up and running" << endl;
  checkLoginSuccess(client_sockfd);
  //loginsuccess
  if (loginsuccess) {
    checkCourseResult(client_sockfd);
  } 
  return 0;
}
