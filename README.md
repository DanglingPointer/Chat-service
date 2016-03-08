# Chat-service

Program consist of a console application implementing the server and two versions of client (a console app and a GUI-version). 
Compiled executables (except console client) for .NET4.5.2 (Any CPU), can be found in `Chat-servise.zip`. `ChatServer.exe` takes port number as a command line parameters, otherwise uses default port number `1234`. `ChatClient.exe` takes IPv4-address and port number as command line parameters, otherwise uses `localhost` and `1234`. `ChatConsoleClient.exe` does not take any command line parameters.

The GUI-version of Client looks like this:

![Messenger](https://github.com/DanglingPointer/Chat-service/blob/master/Messenger.jpg)

The application works fine when using localhost for both client and server, but gives socket error 10060 when using an extern host. The reason is probably some Windows or Firewall settings.

![Socket error](https://github.com/DanglingPointer/Chat-service/blob/master/SocketError.jpg)
