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

struct RequestInputType {
  char inputcourse[50];
  char inputcategory[50];
};

struct CourseType {
  string coursecode;
  string credit;
  string professor;
  string days;
  string coursefullname;
};

string inputcode;
string category;
int serverCS_sockfd_udp;
struct sockaddr_in serverCS_sock_addr, serverM_sock_addr;
struct sockaddr_in new_sock_addr;
socklen_t new_sock_len;
map<string, CourseType> cstxt; // course code to CourseType

//Set socket
//Reused and Modified from Beej's Guide 6.3
void setsocketforServerM() {
  memset(&serverM_sock_addr, 0, sizeof(serverM_sock_addr));
  serverM_sock_addr.sin_family = AF_INET;
  serverM_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverM_sock_addr.sin_port = htons(serverM_UDP_Port);
}

//Reused and Modified from Beej's Guide 6.3
void setServerCSsocket() {
  //ServerCS UDP DATAGRAM
  serverCS_sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
  if (serverCS_sockfd_udp == -1){
    perror("Error on socket setting.");
    exit(-1);
  }

  memset(&serverCS_sock_addr, 0, sizeof(serverCS_sock_addr));
  serverCS_sock_addr.sin_family = AF_INET;
  serverCS_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverCS_sock_addr.sin_port = htons(serverCS_UDP_Port);

  if (bind(serverCS_sockfd_udp, (const struct sockaddr *)&serverCS_sock_addr, sizeof(serverCS_sock_addr)) == -1) {
    perror("Fail on binding");
    exit(0);
  }
}


//Read file same with ServerEE & ServerC
void readFileCS() {
  fstream infile("./cs.txt", ios::in);
  if (!infile.is_open()) {
    perror("Fail to open ee.txt");
    exit(0);
  }
  string line;
  //Parse the data by ","
  while (getline(infile, line)) {
    if (line.length() != 0){
      CourseType course;
      int idx = line.find(",");
      //cout << idx << endl;
      course.coursecode = line.substr(0, idx);
      //Skip ","
      line = line.substr(idx + 1);
      idx = line.find(",");
      //cout << idx << endl;
      course.credit = line.substr(0, idx);
      line = line.substr(idx + 1);
      idx = line.find(",");
      //cout << idx << endl;
      course.professor = line.substr(0, idx);
      line = line.substr(idx + 1);
      idx = line.find(",");
      //cout << idx << endl;
      course.days = line.substr(0, idx);
      line = line.substr(idx + 1);
      course.coursefullname = line;
      cstxt[course.coursecode] = course;
    } else {
      //perror("File is empty");
      break;
    }
  }
}

//Check course and Category
void checkCourseCategoryCS() {
  
  RequestInputType input;
  char checkresult[MAXBUFLENGTH];

  //Get multiple checking request from client
  while(true) {
    memset(&input, 0, sizeof(input));

    int recvsize = recvfrom(serverCS_sockfd_udp, &input, sizeof(input), 0, (struct sockaddr *)&new_sock_addr, &new_sock_len);
    if (recvsize < 0) {
      perror("Error on recving message.");
      exit(-1);
    }
    inputcode = input.inputcourse;
    category = input.inputcategory;
    cout << "The ServerCS received a request from the Main Server about the " << category << " of " << inputcode << "." << endl;

    //Use iterator and map.find() to memorize the location of the coursecode first appear in the map
    map<string, CourseType>::iterator itr;
    itr = cstxt.find(inputcode);
    if (itr == cstxt.end()) {
      strcpy(checkresult, "false");
      cout << "Didn't find the course: " << inputcode << endl;
    } else {
      string tmp;
      if (category == "Credit") {
        tmp = itr->second.credit;
      } else if (category == "Professor") {
        tmp = itr->second.professor;
      } else if (category == "Days") {
        tmp = itr->second.days;
      } else if (category == "CourseName") {
        tmp = itr->second.coursefullname;
      } else {
        cout << "Please enter correct category." << endl;
      }
      
      strcpy(checkresult, tmp.c_str());
      cout << "The course information has been found: The " << category << " of " << inputcode << " is " << checkresult << endl;
    }
    //Send back to main server
    int sendsize = sendto(serverCS_sockfd_udp, checkresult, sizeof(checkresult), 0, (const struct sockaddr *)&serverM_sock_addr, sizeof(serverM_sock_addr));
    if (sendsize < 0) {
      perror("Error on sending.");
      exit(-1);
    }
    cout << "The ServerCS finished sending the response to the Main Server." << endl;
  }
}


int main() {
  setsocketforServerM();
  setServerCSsocket();
  cout << "The ServerCS is up and running using UDP on port " << serverCS_UDP_Port << endl;
  readFileCS();
  checkCourseCategoryCS();
  return 0;
}
