#define CONSOLE_LOG
using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.IO;
using System.Threading.Tasks;
using System.Threading;
using Chat.Formats;
using System.Diagnostics;   // for [Conditional] attribute

namespace Chat.Server
{
    /// <summary>
    /// Maintains connection with one client
    /// </summary>
    internal class ClientHandler
    {
        public event Action<string>             ConnectionLost;
        public event Action<string, Request>    IncomingRequest;
        public event Action<string>             WriteLog;
        /// <summary>
        /// Creates a new ClientHandler that uses a given connection socket.
        /// Does not set any event handlers
        /// </summary>
        public ClientHandler(Socket sock, long clientId)
        {
            m_socket = sock;
            m_stream = new NetworkStream(sock);
            m_parser = new JsonParser(m_stream);
            m_username = clientId.ToString();
            m_nameSet = false;
            m_logSignature = "Message from ClientHandler [" + m_username + "]\n";
        }
        /// <summary>
        /// Username is set to clientId number initially and can be changed only once
        /// </summary>
        public string Username
        {
            get { return m_username; }
            set
            {
                if (m_nameSet == false)
                {
                    m_username = value;
                    m_logSignature = "Message from ClientHandler [" + m_username + "]\n";
                }
                else
                    throw new InvalidOperationException("Username cannot be changed");
            }
        }
        public JsonParser Parser
        {
            get { return m_parser; }
        }
        /// <summary>
        /// Closes the tcp connection and triggers ConnectionLost event
        /// </summary>
        public void Disconnect()
        {
            m_stream.Close();
            ConnectionLost(m_username);
        }
        /// <summary>
        /// Serializes to JSON and sends a response to the client
        /// </summary>
        public void SendResponse(Response resp)
        {
            lock (m_streamMutex)
            {
                m_parser.PutResponse(resp);
            }
        }
        /// <summary> 
        /// Starts checking incoming requests, executes in parallell (does not block).
        /// Triggers IncomingRequest event when anything received
        /// </summary>
        public void Start()
        {
            try
            {
                WriteLog(m_logSignature + string.Format("ClientHandler for client {0} started", Username));
                Task.Run(() =>
                {
                    while (true)
                    {
                        Thread.Sleep(30);
                        if (m_stream.DataAvailable == true)
                        {
                            lock (m_streamMutex)
                            {
                                Request req = m_parser.ExtractRequest();
                                IncomingRequest(m_username, req);
                            }
                        }
                    }
                });
            }
            catch (IOException)
            {
                WriteLog(m_logSignature + string.Format("Connection lost with client {0}:\nDisconnecting client {0}",
                    m_username));
                Disconnect();
            }
            catch (SerializationException)
            {
                WriteLog(m_logSignature + string.Format("Cannot deserialize data from client {0}\nDisconnecting client {0}",
                    m_username));
                Disconnect();
            }
        }

        Socket          m_socket;
        NetworkStream   m_stream;
        string          m_username;
        bool            m_nameSet;
        JsonParser      m_parser;
        string          m_logSignature;
        object          m_streamMutex = new object();
    }
    public class TCPServer
    {
        /// <summary>
        /// Creates a new server that can listen on a given port
        /// </summary>
        public TCPServer(int port)
        {
            m_listener = new TcpListener(IPAddress.Any, port);
            m_msglog = "";
            m_clients = new Dictionary<string, ClientHandler>();

            var host = Dns.GetHostEntry(Dns.GetHostName());
            string localIP="";
            foreach (IPAddress ip in host.AddressList)
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                    localIP = ip.ToString();
            Print(string.Format("Server initialized at ip-address {0} and port {1}", localIP , port));
        }
        /// <summary>
        /// Starts listening to new connections on the given port
        /// </summary>
        public void Run()
        {
            m_listener.Start();
            Print("Listening...");
            long nextManagerId = -1;
            while (true)
            {
                Socket s = m_listener.AcceptSocket(); // blocks
                Print("Client accepted");
                var ch = new ClientHandler(s, nextManagerId);
                ch.IncomingRequest += ServeRequest;
                ch.ConnectionLost += EraseClient;
                ch.WriteLog += Print;
                SendToAll += ch.SendResponse;
                lock (m_clientlistMutex)
                {
                    m_clients[nextManagerId.ToString()] = ch;
                }
                ch.Start();
                --nextManagerId;
            }
        }
        /// <summary>
        /// Starts listening asynchronously to new connections on the given port
        /// </summary>
        public async Task RunAsync()  // might be useful in bigger apps
        {
            await Task.Run(() => Run());
        }
        /// <summary> 
        /// Called by ClientManager when a request is received 
        /// </summary>
        private void ServeRequest(string user, Request req)
        {
            Print("Serving request");
            Response response;
            try
            {
                switch (req.Type)
                {
                    case "login":
                        string newname = req.Content;
                        try
                        {
                            if (!IsNameValid(user) && IsNameValid(newname) && !m_clients.ContainsKey(newname))
                            {
                                var client = m_clients[user];
                                client.Username = newname;  // might throw InvalidOperationException
                                lock (m_clientlistMutex)
                                {
                                    m_clients.Remove(user);
                                    m_clients[newname] = client;
                                }
                                Print(string.Format("Client login: {0}", newname));
                            }
                            else
                                throw new InvalidOperationException();
                        }
                        catch (InvalidOperationException)
                        {
                            response = new Response("error", "ERROR: Invalid username");
                            m_clients[user].SendResponse(response);
                            break;
                        }
                        response = new Response("history", m_msglog);
                        Print(string.Format("History sent to client {0}", newname));
                        m_clients[newname].SendResponse(response);
                        break;
                    case "logout":
                        m_clients[user].Disconnect();
                        Print(string.Format("Client logout: {0}", user));
                        break;
                    case "msg":
                        if (!IsNameValid(user))
                            throw new ProtocolViolationException();
                        response = new Response(user, "message", req.Content);
                        SendToAll(response);
                        m_msglog += m_clients[user].Parser.ConvertToJson(response);
                        Print(string.Format("Message from client {0} sent to all", user));
                        break;
                    case "help":
                        response = new Response("info", "Request types: login, logout, msg, names, help");
                        m_clients[user].SendResponse(response);
                        Print(string.Format("Info sent to client {0}", user));
                        break;
                    case "names":
                        string names = "";
                        object clienlist = m_clients;
                        var volatileclientlist = (IDictionary<string, ClientHandler> )Thread.VolatileRead(ref clienlist);
                        foreach (string name in volatileclientlist.Keys)
                        {
                            if (IsNameValid(name))
                                names += (name + "\n");
                        }
                        response = new Response("info", names);
                        m_clients[user].SendResponse(response);
                        Print(string.Format("All names sent to client {0}", user));
                        break;
                    default:
                        throw new InvalidDataException();
                }
            }
            catch (ProtocolViolationException)
            {
                response = new Response("error", "ERROR: Login required");
                m_clients[user].SendResponse(response);
                Print(string.Format("'ERROR: Login required' sent to client {0}", user));
            }
            catch (InvalidDataException)
            {
                response = new Response("error", "ERROR: Invalid request");
                m_clients[user].SendResponse(response);
                Print(string.Format("'ERROR: Invalid request' sent to client {0}", user));
            }
            catch(Exception e)
            {   // IOException and serialization fail on server's side
                Console.WriteLine(e.Message);
                m_clients[user].Disconnect();
            }
        }
        /// <summary> 
        /// Called by ClientManager when disconnecting 
        /// </summary>
        private void EraseClient(string user)
        {
            SendToAll -= m_clients[user].SendResponse;
            lock (m_clientlistMutex)
            {
                m_clients.Remove(user);
            }
        }
        private bool IsNameValid(string name)
        {
            if (name.Length == 0)
                return false;
            foreach(char letter in name)
            {
                if (letter < 48 || letter > 122 || (letter > 57 && letter < 65)
                    || (letter > 90 && letter < 97))
                    return false;
                return true;
            }
            return true;
        }
        //[Conditional("WRITE_LOG")] // doesn't work with events
        private void Print(string s)
        {
        #if CONSOLE_LOG
            Console.WriteLine("--{0}--\n" + s, DateTime.Now.ToLongTimeString());
        #endif
        }
        private event Action<Response> SendToAll;

        TcpListener     m_listener;
        string          m_msglog;
        IDictionary<string, ClientHandler> m_clients;
        object m_clientlistMutex = new object();
    }
}