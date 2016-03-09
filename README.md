# Chat-service

Simple chat service with a server and any number of clients. Any messages sent by a client are visible for all the other clients. Two implementations of client are available: a console version and a GUI version. 

The compiled files `ChatServer.exe` and `ChatClient.exe` make use of .NET version 4.5.2. `ChatServer.exe` takes a port number as a command line parameter, otherwise uses the default port number `1234`. `ChatClient.exe` takes IPv4-address and a port number as command line parameters, otherwise uses `localhost` and `1234`. `ChatConsoleClient.exe` does not take any command line parameters.

The GUI-version of client looks like this:

![Messenger](https://github.com/DanglingPointer/Chat-service/blob/master/Messenger.jpg)

Since the work was doe as part of a course at NTNU a UML class diagram and two sequence diagrams were created:

![Class diagram](https://github.com/DanglingPointer/Chat-service/blob/master/ClassDiagram.png)

![Login scenario](https://github.com/DanglingPointer/Chat-service/blob/master/Login_scenario.png)

![Message scenario](https://github.com/DanglingPointer/Chat-service/blob/master/Message_scenario.png)
