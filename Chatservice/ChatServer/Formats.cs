using System;
using System.IO;
using System.Net.Sockets;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json; // add System.ServiceModel.Web and
                                         // System.Runtime.Serialization in References

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
            request = type;
            this.content = content;
        }
        public Request(string type) :this(type, "None")
        { }
        [DataMember]
        public string request // Type
        { get; }
        [DataMember]
        public string content // Content
        { get; }
        public override string ToString()   // superfluous??
        {
            return string.Format("Request\nType: {0}\nContent: {1}", request, content);
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
            this.sender = sender;
            response = type;
            this.content = content;
            timestamp = DateTime.Now.ToLongTimeString();
        }
        /// <summary> Can be used with all types except 'message' </summary>
        public Response(string type, string content) :this("server", type, content)
        { }
        [DataMember]
        public string sender // Sender
        { get; }
        [DataMember]
        public string response // Type
        { get; }
        [DataMember]
        public string content // Content
        { get; }
        [DataMember]
        public string timestamp // TimeStamp
        { get; }
        public override string ToString()
        {
            return string.Format("--Response--\nFrom: {2}\nType: {0}\nTime: {3}\nContent: {1}", 
                response, content, sender, timestamp);
        }
    }
    public class JsonParser
    {
        public JsonParser(NetworkStream stream)
        {
            m_tcpreader = new StreamReader(stream);
            m_tcpwriter = new StreamWriter(stream);
            m_tcpwriter.AutoFlush = true;

            m_memstream = new MemoryStream();
            m_memreader = new StreamReader(m_memstream);
            m_memwriter = new StreamWriter(m_memstream);
            m_memwriter.AutoFlush = true;

            m_reqSer    = new DataContractJsonSerializer(typeof(Request));
            m_respSer   = new DataContractJsonSerializer(typeof(Response));
        }
        public void PutRequest(Request obj)
        {
            m_memstream.SetLength(0);
            m_reqSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;

            string serializedObj = m_memreader.ReadToEnd();
            m_tcpwriter.WriteLine(serializedObj);   // Write() ??
        }
        public void PutResponse(Response obj)
        {
            m_memstream.SetLength(0);
            m_respSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;

            string serializedObj = m_memreader.ReadToEnd();
            m_tcpwriter.WriteLine(serializedObj);   // Write() ??
        }
        public Request ExtractRequest()
        {
            string serializedObj = m_tcpreader.ReadLine();  //Read() ??
            m_memstream.SetLength(0);
            m_memwriter.Write(serializedObj);
            return (Request)m_reqSer.ReadObject(m_memstream);
        }
        public Response ExtractResponse()
        {
            string serializedObj = m_tcpreader.ReadLine();  //Read() ??
            m_memstream.SetLength(0);
            m_memwriter.Write(serializedObj);
            return (Response)m_respSer.ReadObject(m_memstream);
        }

        StreamReader m_tcpreader;
        StreamWriter m_tcpwriter;

        MemoryStream m_memstream;
        StreamReader m_memreader;
        StreamWriter m_memwriter;

        DataContractJsonSerializer m_reqSer;
        DataContractJsonSerializer m_respSer;
    }
    //internal class RequestJsonParser : JsonParser<Request>
    //{
    //    public RequestJsonParser(NetworkStream stream) :base(stream)
    //    { }
    //    public override Request ExtractObject()
    //    {
    //        return new Request();
    //    }
    //    public override void SendObject(Request obj)
    //    {
    //        m_memstream.SetLength(0);
    //        m_ser.WriteObject(m_memstream, obj);
    //        m_memstream.Position = 0;
    //
    //        string serializedObj = m_memreader.ReadToEnd();
    //        m_tcpwriter.Write(serializedObj);
    //    }
    //}
    //internal class ResponseJsonParser : JsonParser<Response>
    //{
    //    public ResponseJsonParser(NetworkStream stream) : base(stream)
    //    { }
    //    public override Response ExtractObject()
    //    {
    //        return new Response();
    //    }
    //    public override void SendObject(Response obj)
    //    {
    //        string serializedObj = string.Format("{'timestamp':{0},'sender':{1},'response':{2},'content':{3}}",
    //            obj.TimeStamp, obj.Sender, obj.Type, obj.Content);
    //        m_tcpwriter.Write(serializedObj);
    //    }
    //}
}
