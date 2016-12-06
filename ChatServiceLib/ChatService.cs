using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.ServiceModel;

namespace ChatServiceLib
{
    [DataContract]
    public class Post
    {
        [DataMember]
        public string Username
        { get; set; }
        [DataMember]
        public string Content
        { get; set; }
    }

    [ServiceContract(SessionMode = SessionMode.Required, 
        CallbackContract = typeof(IClientCallback))]
    public interface IChatServer
    {
        [OperationContract]
        bool Register(string name);

        [OperationContract(IsOneWay = true)]
        void PostMessage(string msg);

        [OperationContract(IsOneWay = true)]
        void Unregister();
    }

    public interface IClientCallback
    {
        [OperationContract(IsOneWay = true)]
        void DeliverMessage(Post msg);
    }

    [ServiceBehavior(
        ConcurrencyMode = ConcurrencyMode.Single,
        InstanceContextMode = InstanceContextMode.Single)]
    public class SimpleChatServer : IChatServer
    {
        public SimpleChatServer()
        {
            m_users = new Dictionary<IClientCallback, string>();
            m_log = new List<Post>();
        }

        public bool Register(string name)
        {
            if (m_users.Values.Contains(name))
                return false;
            IClientCallback user = OperationContext.Current.GetCallbackChannel<IClientCallback>();
            m_users.Add(user, name);
            Console.WriteLine("User {0} registerd", name);
            return true;
        }

        public void Unregister()
        {
            IClientCallback user = OperationContext.Current.GetCallbackChannel<IClientCallback>();
            if (!m_users.ContainsKey(user))
                return;
            string name = m_users[user];
            m_users.Remove(user);
            Console.WriteLine("User {0} removed", name);
        }

        public void PostMessage(string text)
        {
            IClientCallback user = OperationContext.Current.GetCallbackChannel<IClientCallback>();
            var post = new Post { Content = text, Username = m_users[user] };
            m_log.Add(post);
            foreach (IClientCallback client in m_users.Keys)
                client.DeliverMessage(post);
            Console.WriteLine("User {0} wrote: {1}", post.Username, post.Content);
        }
        
        IDictionary<IClientCallback, string>   m_users;
        IList<Post> m_log;
    }
}
