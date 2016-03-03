using System;
using System.Net;
using System.Net.Sockets;
using Chat.Formats;
using System.Runtime.Serialization.Json; // add System.ServiceModel.Web and
                                         // System.Runtime.Serialization in References
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;

namespace Chat.Server
{
    class ClientHandler
    {
        public event Action<string, Request> IncomingRequest;
        public event Action<string> ConnectionLost;
        public ClientHandler(Socket sock, long clientId, 
            Action<string, Request> reqHandler, 
            Action<string> discHandler)
        {
            m_socket = sock;
            m_stream = new NetworkStream(sock);
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
        public void Disconnect()
        {
            m_stream.Close();
            ConnectionLost(m_username);
        }
        public void SendResponse(Response resp)
        {
            lock (m_streamMutex)
            {
                m_respSer.WriteObject(m_stream, resp);
            }
        }
        /// <summary> Starts checking incoming requests </summary>
        public void AsyncStart()
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
                                Request req = (Request)m_reqSer.ReadObject(m_stream);
                                IncomingRequest(m_username, req);
                            }
                        }
                    }
                });
            }
            catch (IOException e)
            {
                Console.WriteLine("Exception thrown by client {0}:\n\n{1}", m_username, e.ToString());
                Disconnect();
            }
            catch (SocketException e)
            {
                Console.WriteLine("Exception thrown by client {0}:\n\n{1}", m_username, e.ToString());
                Disconnect();
            }
        }
        Socket          m_socket;
        NetworkStream   m_stream;
        string          m_username;
        bool            m_nameSet;
        object          m_streamMutex = new object();
        DataContractJsonSerializer m_respSer = new DataContractJsonSerializer(typeof(Response));
        DataContractJsonSerializer m_reqSer = new DataContractJsonSerializer(typeof(Request));
    }
    class TCPServer
    {
        public event Action<Response> SendToAll;
        public TCPServer(int port)
        {
            IPAddress ipAddress = Dns.GetHostEntry("localhost").AddressList[0];
            m_listener = new TcpListener(/*IPAddress.Any*/ipAddress, port);
            m_log = new MemoryStream();
            m_logWriter = new StreamWriter(m_log);
            m_logWriter.WriteLine("Server IP-address: {0}", ipAddress);
            m_clients = new Dictionary<string, ClientHandler>();
        }
        public void Start()
        {
            m_listener.Start();
            long nextManagerId = -1;
            while (true)
            {
                Socket s = m_listener.AcceptSocket(); // blocks
                var ch = new ClientHandler(s, nextManagerId, ServeRequest, EraseClient);
                SendToAll += ch.SendResponse;
                lock (m_listMutex)
                {
                    m_clients[nextManagerId.ToString()] = ch;
                }
                ch.AsyncStart();
                --nextManagerId;
            }
        }
        public async Task StartAsync()  // might be useful in bigger apps
        {
            await Task.Run(() => Start());
        }
        /// <summary> Called by ClientManager when a request is received </summary>
        private void ServeRequest(string user, Request req)
        {
            Response? response = null;
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
                                client.Username = newname;  // might throw an exception
                                lock (m_listMutex)
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
                            m_clients[user].SendResponse((Response)response);
                            return;
                        }
                        response = new Response("history", LogToString());
                        m_clients[user].SendResponse((Response)response);
                        break;
                    case "logout":
                        m_clients[user].Disconnect();
                        break;
                    case "msg":
                        if (!IsNameValid(user))
                            throw new ProtocolViolationException();
                        response = new Response(user, "message", req.Content);
                        SendToAll((Response)response);
                        break;
                    case "help":
                        if (!IsNameValid(user))
                            throw new ProtocolViolationException();
                        response = new Response("info", "Request types:\nlogin\nlogout\nmsg\nnames\nhelp");
                        m_clients[user].SendResponse((Response)response);
                        break;
                    case "names":
                        if (!IsNameValid(user))
                            throw new ProtocolViolationException();
                        response = new Response("info", "Request types:\nlogin\nlogout\nmsg\nnames\nhelp");
                        m_clients[user].SendResponse((Response)response);
                        break;
                    default:
                        throw new InvalidDataException();
                }
            }
            catch (ProtocolViolationException)
            {
                response = new Response("error", "ERROR: Login required");
                m_clients[user].SendResponse((Response)response);
            }
            catch (InvalidDataException)
            {
                response = new Response("error", "ERROR: Invalid request");
                m_clients[user].SendResponse((Response)response);
            }
            catch(Exception e)
            {
                Console.WriteLine(e.ToString());
                m_clients[user].Disconnect();
            }
            finally
            {
                if (response != null)
                    m_logWriter.WriteLine(response.ToString());
            }
        }
        /// <summary> Called by ClientManager when disconnecting </summary>
        private void EraseClient(string user)
        {
            lock (m_listMutex)
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
        private string LogToString()
        {
            long prevPos = m_log.Position;
            m_log.Position = 0;
            var sr = new StreamReader(m_log);
            string content = sr.ReadToEnd();
            m_log.Position = prevPos;
            return content;
        }
        TcpListener     m_listener;
        MemoryStream    m_log;
        StreamWriter    m_logWriter;
        Dictionary<string, ClientHandler> m_clients;
        object m_listMutex = new object();
    }
}