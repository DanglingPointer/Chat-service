using System;
using System.IO;
using System.Collections.Generic;
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
        { get; private set; }
        [DataMember]
        public string content // Content
        { get; private set; }
        public override string ToString()   // superfluous??
        {
            return string.Format("--Request--\nType: {0}\nContent: {1}", request, content);
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
        { get; private set; }
        [DataMember]
        public string response // Type
        { get; private set; }
        [DataMember]
        public string content // Content
        { get; private set; }
        [DataMember]
        public string timestamp // TimeStamp
        { get; private set; }
        public override string ToString()   // superfluous??
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
            m_tcpwriter.Write(serializedObj);   // WriteLine() ??
        }
        public void PutResponse(Response obj)
        {
            m_memstream.SetLength(0);
            m_respSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;

            string serializedObj = m_memreader.ReadToEnd();
            m_tcpwriter.Write(serializedObj);   // WriteLine() ??
        }
        public Request ExtractRequest()
        {
            string serializedObj = ReadJsonObject();  // m_tcpreader.ReadLine() ??
            m_memstream.SetLength(0);
            m_memwriter.Write(serializedObj);
            m_memstream.Position = 0;
            return (Request)m_reqSer.ReadObject(m_memstream);
        }
        public Response ExtractResponse()
        {
            string serializedObj = ReadJsonObject();  // m_tcpreader.ReadLine() ??
            m_memstream.SetLength(0);
            m_memwriter.Write(serializedObj);
            m_memstream.Position = 0;
            return (Response)m_respSer.ReadObject(m_memstream);
        }
        public string ConvertToJson(Request obj)
        {
            m_memstream.SetLength(0);
            m_reqSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;
            return m_memreader.ReadToEnd();
        }
        public string ConvertToJson(Response obj)
        {
            m_memstream.SetLength(0);
            m_respSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;
            return m_memreader.ReadToEnd();
        }
        /// <summary> Converts a string consisting of one json-request </summary>
        public Request ConvertToRequest(string jsonString)
        {
            m_memstream.SetLength(0);
            m_memwriter.Write(jsonString);
            m_memstream.Position = 0;
            return (Request)m_reqSer.ReadObject(m_memstream);
        }
        /// <summary> Converts a string consisting of one json-response </summary>
        public Response ConvertToResponse(string jsonString)
        {
            m_memstream.SetLength(0);
            m_memwriter.Write(jsonString);
            m_memstream.Position = 0;
            return (Response)m_respSer.ReadObject(m_memstream);
        }
        /// <summary> Extracts first json-string from the tcp stream, invalid
        /// json-syntacs or its absence causes blocking </summary>
        public string ReadJsonObject()
        {
            int opnCount = 0;
            int clsCount = 0;
            string temp = "";
            int symbol;
            do
            {
                symbol = m_tcpreader.Read();
                if (symbol == 123) // '{'
                    ++opnCount;
                else if (symbol == 125) // '}'
                    ++clsCount;
                if (opnCount != 0) temp += symbol;
            } while (opnCount != clsCount || opnCount == 0);
            return temp;
        }
        /// <summary> Extracts all Json-objects (if any) from a string </summary>
        public string[] SplitJsonObjects(string s)
        {
            int opnCount = 0;
            int clsCount = 0;
            var strs = new List<string>();
            string temp="";

            foreach(char symbol in s)
            {
                if (symbol == 123) // '{'
                    ++opnCount;
                else if (symbol == 125) // '}'
                    ++clsCount;
                if (opnCount != 0) temp += symbol;
                if (opnCount == clsCount && opnCount != 0)
                {
                    strs.Add(string.Copy(temp));
                    temp = "";
                }
            }
            return strs.ToArray();
        }
        StreamReader m_tcpreader;
        StreamWriter m_tcpwriter;

        MemoryStream m_memstream;
        StreamReader m_memreader;
        StreamWriter m_memwriter;

        DataContractJsonSerializer m_reqSer;
        DataContractJsonSerializer m_respSer;
    }
}
