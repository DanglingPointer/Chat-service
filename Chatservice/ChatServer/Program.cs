using System;
using System.Net.Sockets;
using Chat.Server;

namespace Chat
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                var server = new TCPServer(Convert.ToInt32(args[0]));
                server.Start();
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
