using System;
using System.Net.Sockets;
using Chat.Server;

namespace Chat
{
    class ServerProgram
    {
        static void Main(string[] args)
        {
            try
            {
                int port = 1234;
                if (args.Length == 1)
                    port = Convert.ToInt32(args[0]);
                var server = new TCPServer(port);
                server.Run();
            }
            catch (IndexOutOfRangeException)
            {
                Console.WriteLine("ERROR: no port number provided\n");
            }
            catch(ArgumentOutOfRangeException)
            {
                Console.WriteLine("ERROR: invalid port number provided\n");
            }
            catch (SocketException)
            {
                Console.WriteLine("ERROR: port number already in use\n");
            }
        }
    }
}
