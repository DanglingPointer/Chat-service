using System;
using System.Net.Sockets;
using System.Threading.Tasks;
using Chat.Client;

namespace Chat
{
    /// <summary>
    /// Simple console program for testing client logic
    /// </summary>
    class ClientProgram
    {
        static bool alive = true;
        static void Main(string[] args)
        {
            try
            {
                Console.Write("Enter IPv4-address: ");
                string ipv4 = Console.ReadLine();
                Console.Write("Enter port number: ");
                int port = Convert.ToInt32(Console.ReadLine());
                var client = new ChatClient(ipv4, port);
                client.ConnectionLost += PrintAbortMsg;
                client.ErrorReceived += PrintError;
                client.InfoReceived += PrintInfo;
                client.MessageReceived += PrintMessage;

                Task.Run(() => client.Run());
                Console.WriteLine("Commands:\nl - login\nm - send message\nn - request names\nh - help\nlo - logout");
                while (alive)
                {
                    bool b;
                    string command = Console.ReadLine();
                    switch (command)
                    {
                        case "l":
                            Console.Write("Enter username: ");
                            string username = Console.ReadLine();
                            b = client.LogIn(username);
                            Console.WriteLine("Operation successfull: {0}", b);
                            break;
                        case "m":
                            Console.Write("Enter message: ");
                            string msg = Console.ReadLine();
                            b = client.SendMessage(msg);
                            Console.WriteLine("Operation successfull: {0}", b);
                            break;
                        case "n":
                            b = client.RequestNames();
                            Console.WriteLine("Operation successfull: {0}", b);
                            break;
                        case "h":
                            b = client.RequestHelp();
                            Console.WriteLine("Operation successfull: {0}", b);
                            break;
                        case "lo":
                            client.LogOut();
                            break;
                    }
                }
            }
            catch (SocketException e)
            {
                Console.WriteLine("Oops!.. Unable to connect to server.\n"
                    + e.Message);
            }
            catch(Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            finally
            {
                Console.Read();
            }
        }
        static void PrintMessage(string msg)
        {
            Console.WriteLine("\n--MESSAGE RECEIVED--\n" + msg);
        }
        static void PrintError(string errmsg)
        {
            Console.WriteLine("\n--ERROR RECEIVED--\n" + errmsg);
        }
        static void PrintInfo(string info)
        {
            Console.WriteLine("\n--INFO RECEIVED--\n" + info);
        }
        static void PrintAbortMsg()
        {
            alive = false;
            Console.WriteLine("\nWe are finished!");
        }
    }
}
