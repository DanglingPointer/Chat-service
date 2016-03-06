using System;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Chat.Client;

namespace Chat
{
    class Program
    {
        static bool alive = true;
        static void Main(string[] args)
        {
            try
            {
                var client = new ChatClient(args[0], Convert.ToInt32(args[1]));
                client.ConnectionLost += PrintAbortMsg;
                client.ErrorReceived += PrintError;
                client.InfoReceived += PrintInfo;
                client.MessageReceived += PrintMessage;

                Task.Run(() => client.Run());
                Console.WriteLine("Commands:\nl - login\nm - send message\nn - request names\nh - help\n");
                while (alive)
                {
                    string command = Console.ReadLine();
                    switch (command)
                    {
                        case "l":
                            Console.WriteLine("Enter username: ");
                            string username = Console.ReadLine();
                            client.LogIn(username);
                            break;
                        case "m":
                            Console.WriteLine("Enter message: ");
                            string msg = Console.ReadLine();
                            client.SendMessage(msg);
                            break;
                        case "n":
                            client.RequestNames();
                            break;
                        case "h":
                            client.RequestHelp();
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
                Console.ReadKey();
            }
        }
        static void PrintMessage(string msg)
        {
            Console.WriteLine("--MESSAGE RECEIVED--\n" + msg);
        }
        static void PrintError(string errmsg)
        {
            Console.WriteLine("--ERROR RECEIVED--\n" + errmsg);
        }
        static void PrintInfo(string info)
        {
            Console.WriteLine("--INFO RECEIVED--\n" + info);
        }
        static void PrintAbortMsg()
        {
            alive = false;
            Console.WriteLine("We are finished!");
        }
    }
}
