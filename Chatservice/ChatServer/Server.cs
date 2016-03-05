using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.IO;
using System.Threading.Tasks;
using Chat.Formats;

namespace Chat.Server
{
    internal class ClientHandler
    {
        public event Action<string, Request> IncomingRequest;
        public event Action<string> ConnectionLost;
        public ClientHandler(Socket sock, long clientId, 
            Action<string, Request> reqHandler, 
            Action<string> discHandler)
        {
            m_socket = sock;
            m_stream = new NetworkStream(sock);
            m_parser = new JsonParser(m_stream);
            m_username = clientId.ToString();
            m_nameSet = false;
            IncomingRequest += reqHandler;
            ConnectionLost += discHandler;
        }
        public string Username
        {
            get { return m_username; }
            set
            {
                if (m_nameSet == false) m_username = value;
                else throw new InvalidOperationException("Username cannot be changed");
            }
        }
        public JsonParser Parser
        {
            get { return m_parser; }
        }
        public void Disconnect()
        {
            m_stream.Close();
            ConnectionLost(m_username);
        }
        public void SendResponse(Response resp)
        {
            lock (m_streamMutex)
            {
                m_parser.PutResponse(resp);
            }
        }
        /// <summary> Starts checking incoming requests, executes in parallell </summary>
        public void Start()
        {
            try
            {
                Task.Run(() =>
                {
                    while (true)
                    {
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
                Console.WriteLine("Connection lost with client {0}:\nDisconnecting client {0}",
                    m_username);
                Disconnect();
            }
            catch (SerializationException)
            {
                Console.WriteLine("Cannot deserialize data from client {0}\nDisconnecting client {0}",
                    m_username);
                Disconnect();
            }
        }
        Socket          m_socket;
        NetworkStream   m_stream;
        string          m_username;
        bool            m_nameSet;
        JsonParser      m_parser;
        object          m_streamMutex = new object();
    }
    public class TCPServer
    {
        public TCPServer(int port)
        {
            IPAddress ipAddress = Dns.GetHostEntry("localhost").AddressList[0];
            m_listener = new TcpListener(/*IPAddress.Any*/ipAddress, port);
            m_log = "";
            m_clients = new Dictionary<string, ClientHandler>();
        }
        public void Run()
        {
            m_listener.Start();
            long nextManagerId = -1;
            while (true)
            {
                Socket s = m_listener.AcceptSocket(); // blocks
                var ch = new ClientHandler(s, nextManagerId, ServeRequest, EraseClient);
                SendToAll += ch.SendResponse;
                lock (m_clientlistMutex)
                {
                    m_clients[nextManagerId.ToString()] = ch;
                }
                ch.Start();
                --nextManagerId;
            }
        }
        public async Task RunAsync()  // might be useful in bigger apps
        {
            await Task.Run(() => Run());
        }
        /// <summary> Called by ClientManager when a request is received </summary>
        private void ServeRequest(string user, Request req)
        {
            Response response;
            try
            {
                switch (req.request)
                {
                    case "login":
                        string newname = req.content;
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
                        response = new Response("history", m_log);
                        m_clients[user].SendResponse(response);
                        break;
                    case "logout":
                        m_clients[user].Disconnect();
                        break;
                    case "msg":
                        if (!IsNameValid(user))
                            throw new ProtocolViolationException();
                        response = new Response(user, "message", req.content);
                        SendToAll(response);
                        m_log += m_clients[user].Parser.ConvertToJson(response);
                        break;
                    case "help":
                        if (!IsNameValid(user))
                            throw new ProtocolViolationException();
                        response = new Response("info", "Request types: login, logout, msg, names, help");
                        m_clients[user].SendResponse(response);
                        break;
                    case "names":
                        if (!IsNameValid(user))
                            throw new ProtocolViolationException();
                        string names = "";
                        foreach (string name in m_clients.Keys)
                        {
                            names += (name + " ");
                        }
                        response = new Response("info", names);
                        m_clients[user].SendResponse(response);
                        break;
                    default:
                        throw new InvalidDataException();
                }
            }
            catch (ProtocolViolationException)
            {
                response = new Response("error", "ERROR: Login required");
                m_clients[user].SendResponse(response);
            }
            catch (InvalidDataException)
            {
                response = new Response("error", "ERROR: Invalid request");
                m_clients[user].SendResponse(response);
            }
            catch(Exception e)
            {   // IOException and serialization fail on server's side
                Console.WriteLine(e.Message);
                m_clients[user].Disconnect();
            }
        }
        /// <summary> Called by ClientManager when disconnecting </summary>
        private void EraseClient(string user)
        {
            lock (m_clientlistMutex)
            {
                m_clients.Remove(user);
            }
        }
        private bool IsNameValid(string name)
        {
            foreach(char letter in name)
            {
                if (letter < 48 || letter > 122 || (letter > 57 && letter < 65)
                    || (letter > 90 && letter < 97))
                    return false;
                return true;
            }
            return true;
        }
        private event Action<Response> SendToAll;

        TcpListener     m_listener;
        string          m_log;
        IDictionary<string, ClientHandler> m_clients;
        object m_clientlistMutex = new object();
    }
}