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

// define the course request/outputonse message format
struct RequestInputType {
  char inputcourse[50];
  char inputcategory[50];
};

// define course structure
struct CourseType {
  string coursecode;
  string credit;
  string professor;
  string days;
  string coursefullname;
};

string inputcode;
string category;
int serverEE_sockfd_udp;
struct sockaddr_in serverEE_sock_addr, serverM_sock_addr;
struct sockaddr_in new_sock_addr;
socklen_t new_sock_len;
map<string, CourseType> eetxt; // course code to CourseType


//Set socket
//Reused and Modified from Beej's Guide 6.3
void setsocketforServerM() {
  memset(&serverM_sock_addr, 0, sizeof(serverM_sock_addr));
  serverM_sock_addr.sin_family = AF_INET;
  serverM_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverM_sock_addr.sin_port = htons(serverM_UDP_Port);
}

//Reused and Modified from Beej's Guide 6.3
void setServerEEsocket() {
  serverEE_sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
  if (serverEE_sockfd_udp == -1) {
    perror("Error on socket setting.");
    exit(-1);
  }
  
  memset(&serverEE_sock_addr, 0, sizeof(serverEE_sock_addr));
  serverEE_sock_addr.sin_family = AF_INET;
  serverEE_sock_addr.sin_addr.s_addr = inet_addr(localhost);
  serverEE_sock_addr.sin_port = htons(serverEE_UDP_Port);
  
  if (bind(serverEE_sockfd_udp, (const struct sockaddr *)&serverEE_sock_addr, sizeof(serverEE_sock_addr)) == -1) {
    perror("Error on binding the socket");
    exit(0);
  }
}


//Read file same with ServerCS & ServerC
void readFileEE() {
  //Read file
  fstream infile("./ee.txt", ios::in);
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
      eetxt[course.coursecode] = course;
    } else {
      //perror("File is empty");
      break;
    }
  }
}

//Check course and Category
void checkCourseCategoryEE() {
  
  RequestInputType input;
  char checkresult[MAXBUFLENGTH];
  while(true) {
    memset(&input, 0, sizeof(input));
    
    //Receive the check course request from main server
    int recvsize = recvfrom(serverEE_sockfd_udp, &input, sizeof(input), 0, (struct sockaddr *)&new_sock_addr, &new_sock_len);
    if(recvsize < 0) {
      perror("Error on receive.");
      exit(-1);
    }
    inputcode = input.inputcourse;
    category = input.inputcategory;
    cout << "The ServerEE received a request from the Main Server about the " << category << " of " << inputcode << "." << endl;
    
    //Use iterator and map.find() to memorize the location of the coursecode first appear in the map
    map<string, CourseType>::iterator itr;
    itr = eetxt.find(inputcode);
    if (itr == eetxt.end()) {
      strcpy(checkresult, "false");
      cout << "Didn't find the course: " << inputcode << endl;
    } else {
      string temp;
      if (category == "Credit") {
        temp = itr->second.credit;
      } else if (category == "Professor") {
        temp = itr->second.professor;
      } else if (category == "Days") {
        temp = itr->second.days;
      } else if (category == "CourseName") {
        temp =itr->second.coursefullname;
      } else {
        cout << "Please enter correct category." << endl;
        //break;
      }
      strcpy(checkresult, temp.c_str());
      cout << "The course information has been found: The " << category << " of " << inputcode << " is " << checkresult << endl;
    }
    //Send the result back to main server
    int sendsize = sendto(serverEE_sockfd_udp, checkresult, sizeof(checkresult), 0, (const struct sockaddr *)&serverM_sock_addr, sizeof(serverM_sock_addr));
    if (sendsize < 0) {
      perror("Error on sending.");
      exit(-1);
    }
    cout << "The ServerEE finished sending the response to the Main Server." << endl; 
  }
  
}


int main() {
  setsocketforServerM();
  setServerEEsocket();
  cout << "The ServerEE is up and running using UDP on port " << serverEE_UDP_Port << endl;
  readFileEE();
  checkCourseCategoryEE();
  return 0;
}
