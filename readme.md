# EE450-Socket-Project
Student Name: Keying Pang
Student ID: 8713254080

# What I have done
Client Server: 

 - Phase 1A: The Client Server will store the unencrypted username and unencrypted password in the struct LoginInputType. The Client Server will send this unencrypted message to Main Server over TCP using a dynamically assigned port number instead of a hard-coded port number.
 - Phase 2B: The Client Server received the authentication result. The Client Server will have maximum three attempts and the server will automatically shut down if all three attempts failed.
 - Phase 3A: The Client Server will store the course code and the category in the struct RequestInputType. The Client Server will send the data to Main Server over TCP using a dynamically assigned port number instead of a hard-coded port number.
 - Phase 4B: The Client Server received the course check result and print out on-screen message.

Main Server:

 - Phase 1B: The Main Server will store the received message from the Client Server over TCP, offset each character and digit by 4 and send the encrypted username and password to the Credential Server over UDP.
 - Phase 2A: The Main Server received the authentication result from the Credential Server over UDP.
 - Phase 2B: The Main Server sends the authentication result to the Client Server over TCP.
 - Phase 3A: The Main Server received and buffered the course code and category sent from the Client and check whether this data is sent to EE Server or CS server.
 - Phase 3B: The Main Server sent the data to the corresponding  Department Server.
 - Phase 4B: The Main Server received the course check result from the Department Server and sent back to the Client Server over TCP.

Credential Server:

 - Phase 2A: The Credential Server checked the encrypted username and password and sent the result back to the Main Server over UDP. "faila" means the username do not exist. "failb" means the username is correct but the password does not match. "success" means the authentication passed.

Department Server:
 - Phase 3B: The corresponding Department Server received the course code and category over UDP and check its buffered information to obtain the result.
 - Phase 4B: The Department Server sent the result to the Main Server over UDP.


## Code Files

 1. client.cpp
	- Define LoginInputType to store the unencrypted username and password. Define RequestInputType to store the request course code and category.
	- Create TCP socket with the main server
	- Get dynamically assigned TCP port number and stored it in assignedtcpport.
	- Send authentication request and receive the authentication result from the main server.
	- If the authentication success, send the check course code and category message and get the result from the main server.

 2. serverM.cpp
	 - Create UDP socket with the credential server
	 - Create TCP socket with client server.
	 - Receive the authentication request from the client over TCP and send the request to the credential server over UDP.
	 - Receive the authentication result from the credential server over UDP and send the result to the client over TCP.
	 - Create UDP socket with the corresponding department server.
	 - Receive the course code and category from the client over TCP and send the data to the corresponding department server over UDP.
	 - Receive the result from the department server over UDP and send the result back to the client over TCP

 3. serverC.cpp
	 - Create UDP socket with the main server
	 - Read file cred.txt
	 - Receive the login request from the main server over UDP and check whether the request matches with file cred.txt
	 - Send the login result to the main server over UDP.

 4. serverEE/CS.cpp
	 - Create UDP socket with the main server
	 - Read file ee.txt/cs.txt
	 - Receive the course check request to the main server over UDP and check whether the course code and category match with file ee.txt/cs.txt.
	 - Send the course check result to the main server over UDP.

## Messages Exchange Format

 1. Login Request Message
- Define LoginInputType to store the unencrypted username and unencrypted password entered from the client server. Set the maximum size of the two chars to 50.
 2. Course Check Message
- Define RequestInputType to store the input course code and category entered from the client server. Set the maximum size of the two chars to 50.
 4. Credential File
 - Use getline() to read in the file line by line, use find() to find the place where the ',' appeared, and use substr() to parse the message and pass the store them.
 - Use map<string, string> to map the encrypted username to the encrypted password read from cred.txt and stored in the credential server.
 6. Department File 
 - Define CourseType to store the coursecode, credit, professor, days and course name read from ee.txt or cs.txt. 
- Use getline() to read in the file line by line, use find() to find the place where the ',' appeared, and use substr() to parse the message and pass the store them.
 - Then use map<string, CourseType> to map the data of each category to the course code and stored the map in the department server.
 6. Databuffer
  - To send and receive data from each server. I used buffers in form of char buffername[MAXBUFLENGTH] (buffername may differ for each code file) to store each message exchanged between servers and clients then I copied the data from the databuffer to the corresponding output.
 7. Login Result Message
 - Defined different types of output to represent the result of each login authentication result.
 "faila" ->Authenticationn failed: Username Does not exist
 "failb" -> Authentication failed: Password does not match
 "success" ->Authentication is successful
 8. Check Course Result Message
 - Defined different types of output to represent the result of each check course result. 
If the input course code is wrong, copy "false" to the send buffer and send it to the main server.
Else, copy the corresponding category of the input course code to the send buffer and send it to the main server.

## Idiosyncrasy

 If the client server is close, the credential server and department servers will remain the same, but the main server will also close.

## Reused Code & Learning Material

 1. Beej's Guide to Network Programming
 - Set sockets for TCP and UDP
 - Send and receive data between  client and server
 - Client-Server setup 
 - Functions formats of socket programming: accept( ), bind( ), connect( ), listen( ), recv( ), recvfrom( ), send ( ).....
 2. [Structures in C++](https://www.geeksforgeeks.org/structures-in-cpp/)( Learning material)
 - Defined data types used to store groups of items
 - The [syntax](https://www.javatpoint.com/cpp-structs) of structures in C++.
 - [Differences between struct and class](https://stackoverflow.com/questions/54585/when-should-you-use-a-class-vs-a-struct-in-c) and on which condition we should use class instead of struct or vice versa.
 
 3. [Map in C++](https://www.geeksforgeeks.org/map-associative-containers-the-c-standard-template-library-stl/)( Learning material)
 - Store the data in a mapped function. Map the key value to the mapped value.
 - How to [construct a map and access elements](https://www.udacity.com/blog/2020/03/c-maps-explained.html) in a map.
 
 4. [Find in Maps and Strings](https://www.oreilly.com/library/view/c-cookbook/0596007612/ch04s10.html)( Learning material)
- Use [map.find()](https://www.geeksforgeeks.org/map-find-function-in-c-stl/) function to return the position where the key is present in the map.
- Use string [iterator](https://www.geeksforgeeks.org/iterate-over-characters-of-a-string-in-c/) in C++  to search the corresponding element.
- Use [string.find()](https://www.geeksforgeeks.org/string-find-in-cpp/) to search the element.
- [Other follow-up questions](https://stackoverflow.com/questions/3061966/why-does-stringfind-return-size-type-and-not-an-iterator) 

## Port Number Table
|Process  |Dynamic Ports  |Static Ports     |
|--       |--             |--               |
|serverC  |-  |1 UDP, 21080                 |
|serverCS |-  |1 UDP, 22080                 |
|serverEE |-  |1 UDP, 23080                 |
|serverM  |-  |1 UDP, 24080 ; 1 TCP, 25080  | 
|Client   |1 TCP |< Dynamic Port assignment>|



