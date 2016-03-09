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
            Type = type;
            Content = content;
        }
        public Request(string type) :this(type, "")
        { }
        [DataMember]
        public string Type  
        { get; private set; }
        [DataMember]
        public string Content  
        { get; private set; }
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
        { get; private set; }
        [DataMember]
        public string Type  
        { get; private set; }
        [DataMember]
        public string Content   
        { get; private set; }
        [DataMember]
        public string TimeStamp 
        { get; private set; }
    }
    public class JsonParser
    {
        /// <summary>
        /// Constructs a new parser that reads/writes from/to an established TCP stream
        /// </summary>
        public JsonParser(NetworkStream tcpstream)
        {
            m_tcpreader = new StreamReader(tcpstream);
            m_tcpwriter = new StreamWriter(tcpstream);
            m_tcpwriter.AutoFlush = true;

            m_memstream = new MemoryStream();
            m_memreader = new StreamReader(m_memstream);
            m_memwriter = new StreamWriter(m_memstream);
            m_memwriter.AutoFlush = true;

            m_reqSer    = new DataContractJsonSerializer(typeof(Request));
            m_respSer   = new DataContractJsonSerializer(typeof(Response));
        }
        /// <summary>
        /// Serializes a request to JSON and sends to the tcp stream
        /// </summary>
        public void PutRequest(Request obj)
        {
            m_memstream.SetLength(0);
            m_reqSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;

            string serializedObj = m_memreader.ReadToEnd();
            m_tcpwriter.Write(serializedObj);
        }
        /// <summary>
        /// Serializes a response to JSON and sends to the tcp stream
        /// </summary>
        public void PutResponse(Response obj)
        {
            m_memstream.SetLength(0);
            m_respSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;

            string serializedObj = m_memreader.ReadToEnd();
            m_tcpwriter.Write(serializedObj);
        }
        /// <summary>
        /// Reads first request from the TCP stream and deserializes it.
        /// No JSON objects in the stream causes blocking
        /// </summary>
        public Request ExtractRequest()
        {
            string serializedObj = ReadJsonObject();
            m_memstream.SetLength(0);
            m_memwriter.Write(serializedObj);
            m_memstream.Position = 0;
            return (Request)m_reqSer.ReadObject(m_memstream);
        }
        /// <summary>
        /// Reads first response from the TCP stream and deserializes it.
        /// No JSON objects in the stream causes blocking
        /// </summary>
        public Response ExtractResponse()
        {
            string serializedObj = ReadJsonObject();
            m_memstream.SetLength(0);
            m_memwriter.Write(serializedObj);
            m_memstream.Position = 0;
            return (Response)m_respSer.ReadObject(m_memstream);
        }
        /// <summary>
        /// Obtains JSON representation of a Request object
        /// </summary>
        public string ConvertToJson(Request obj)
        {
            m_memstream.SetLength(0);
            m_reqSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;
            return m_memreader.ReadToEnd();
        }
        /// <summary>
        /// Obtains JSON representation of a Response object
        /// </summary>
        public string ConvertToJson(Response obj)
        {
            m_memstream.SetLength(0);
            m_respSer.WriteObject(m_memstream, obj);
            m_memstream.Position = 0;
            return m_memreader.ReadToEnd();
        }
        /// <summary> 
        /// Deserializes a string consisting of one JSON-request 
        /// </summary>
        public Request ConvertToRequest(string jsonString)
        {
            m_memstream.SetLength(0);
            m_memwriter.Write(jsonString);
            m_memstream.Position = 0;
            return (Request)m_reqSer.ReadObject(m_memstream);
        }
        /// <summary> 
        /// Deserializes a string consisting of one JSON-response 
        /// </summary>
        public Response ConvertToResponse(string jsonString)
        {
            m_memstream.SetLength(0);
            m_memwriter.Write(jsonString);
            m_memstream.Position = 0;
            return (Response)m_respSer.ReadObject(m_memstream);
        }
        /// <summary> 
        /// Extracts all JSON-objects (if any) from a string 
        /// </summary>
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
                    opnCount = clsCount = 0;
                }
            }
            return strs.ToArray();
        }
        /// <summary> 
        /// Extracts first json-string from the tcp stream, invalid syntacs or its absence causes blocking 
        /// </summary>
        protected string ReadJsonObject()
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
                if (opnCount != 0) temp += (char)symbol;
            } while (opnCount != clsCount || opnCount == 0);
            return temp;
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
