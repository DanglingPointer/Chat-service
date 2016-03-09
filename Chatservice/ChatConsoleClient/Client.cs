using System;
using System.Net;
using System.Net.Sockets;
using System.Runtime.Serialization;
using Chat.Formats;

namespace Chat.Client
{
    public class ChatClient
    {
        /// <summary>
        /// Connects to the server, but doesn't read any data yet
        /// </summary>
        public ChatClient(string serverAddr, int serverPort)
        {
            m_client = new TcpClient();
            IPEndPoint endpt = new IPEndPoint(IPAddress.Parse(serverAddr), serverPort);
            m_client.Connect(endpt);    // might throw SocketException
            m_parser = new JsonParser(m_client.GetStream());
        }
        public event Action<string>     MessageReceived;
        public event Action<string>     ErrorReceived;
        public event Action<string>     InfoReceived;
        public event Action             ConnectionLost;
        /// <summary>
        /// Starts checking incoming messages, blocks.
        /// Proceeds each response asynchronously (fires the events)
        /// </summary>
        public void Run()
        {
            var stream = m_client.GetStream();
            try
            {
                while (true)
                {
                    if (stream.DataAvailable == true)
                    {
                        try
                        {
                            Response resp;
                            lock (m_mutex)
                            {
                                resp = m_parser.ExtractResponse();
                            }
                            ProceedResponse(resp);
                        }
                        catch (SerializationException)
                        { }
                    }
                }
            }
            catch
            {
                ConnectionLost.BeginInvoke(null, null);
                m_client.Close();
            }
        }
        // -------- Methods for sending different types of requests to the server ------------------
        public bool SendMessage(string text)
        {
            return SendRequest("msg", text);
        }
        public bool LogIn(string username)
        {
            return SendRequest("login", username);
        }
        public bool RequestNames()
        {
            return SendRequest("names", "None");
        }
        public bool RequestHelp()
        {
            return SendRequest("help", "None");
        }
        public void LogOut()
        {
            SendRequest("logout", "None");
            m_client.Close();
        }
        //------------------------------------------------------------------------------------------
        private bool SendRequest(string type, string content)
        {
            try
            {
                var request = new Request(type, content);
                lock (m_mutex)
                {
                    m_parser.PutRequest(request);
                }
                return true;
            }
            catch (SerializationException)
            {
                return false;
            }
            catch
            {
                ConnectionLost.BeginInvoke(null, null);
                m_client.Close();
                return false;
            }
        }
        private void ProceedResponse(Response resp)
        {
            switch (resp.Type)
            {
                case "error":
                    ErrorReceived.BeginInvoke(resp.TimeStamp + "\n" + resp.Content, null, null);
                    break;
                case "message":
                    MessageReceived.BeginInvoke(resp.TimeStamp + " " + resp.Sender + " wrote:\n" + resp.Content,
                        null, null);
                    break;
                case "info":
                    InfoReceived.BeginInvoke(resp.Content, null, null);
                    break;
                case "history":
                    string[] log = m_parser.SplitJsonObjects(resp.Content);
                    foreach(string jsonmsg in log)
                    {
                        Response msg = m_parser.ConvertToResponse(jsonmsg);
                        ProceedResponse(msg);
                    }
                    break;
            }
        }
        TcpClient       m_client;
        JsonParser      m_parser;
        object m_mutex = new object();
    }
}
