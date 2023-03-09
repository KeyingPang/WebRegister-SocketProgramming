# Makefile 
#Use make all or make to generate executable files
#Use make clean to remove all the executable files
.PHONY: all
all: serverM serverC serverEE serverCS client

serverM : serverM.cpp 
	g++ -std=c++11 -o serverM serverM.cpp
serverC : serverC.cpp 
	g++ -std=c++11 -o serverC serverC.cpp
serverEE : serverEE.cpp 
	g++ -std=c++11 -o serverEE serverEE.cpp
serverCS : serverCS.cpp 
	g++ -std=c++11 -o serverCS serverCS.cpp
client : client.cpp 
	g++ -std=c++11 -o client client.cpp 

.PHONY: clean
clean:
	rm serverM serverC serverCS serverEE client