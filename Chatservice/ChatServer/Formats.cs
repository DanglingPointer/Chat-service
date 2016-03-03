using System;
using System.Runtime.Serialization;

namespace Chat.Formats
{
    /// <summary>
    /// Payload from a client to the server, immutable.
    /// Type: login, logout, msg, names, help
    /// Content: username for 'login', text for 'msg', None otherwse
    /// </summary>
    [DataContract]
    public struct Request
    {
        public Request(string type, string content)
        {
            Type = type;
            Content = content;
        }
        public Request(string type) :this(type, "None")
        { }
        [DataMember]
        public string Type
        { get; }
        [DataMember]
        public string Content
        { get; }
        public override string ToString()   // superfluous??
        {
            return string.Format("Request\nType: {0}\nContent: {1}", Type, Content);
        }
    }
    /// <summary>
    /// Payload from the server to a client, immutable.
    /// Sender: 'server' or username (if Type is 'message')
    /// Type: error, info, message, history
    /// </summary>
    [DataContract]
    public struct Response
    {
        public Response(string sender, string type, string content)
        {
            Sender = sender;
            Type = type;
            Content = content;
            TimeStamp = DateTime.Now.ToLongTimeString();
        }
        /// <summary> Can be used with all types except 'message' </summary>
        public Response(string type, string content) :this("server", type, content)
        { }
        [DataMember]
        public string Sender
        { get; }
        [DataMember]
        public string Type
        { get; }
        [DataMember]
        public string Content
        { get; }
        [DataMember]
        public string TimeStamp
        { get; }
        public override string ToString()
        {
            return string.Format("--Response--\nFrom: {2}\nType: {0}\nTime: {3}\nContent: {1}", 
                Type, Content, Sender, TimeStamp);
        }
    }
}
