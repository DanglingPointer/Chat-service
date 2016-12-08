using System;
using System.Windows.Documents;
using System.ComponentModel;
using System.Windows.Input;
using System.ServiceModel;
using ChatClient.ChatServiceReference;
using System.Windows.Media;

namespace ChatClient
{
    public class RelayCommand : ICommand
    {
        public event EventHandler CanExecuteChanged;
        public RelayCommand(Action<object> execute, Func<object, bool> canExecute)
        {
            m_execute = execute;
            m_canExec = canExecute;
        }
        public RelayCommand(Action<object> execute)
        {
            m_execute = execute;
            m_canExec = (o) => true;
        }
        public void Execute(object o)
        {
            m_execute(o);
        }
        public bool CanExecute(object o)
        {
            return m_canExec(o);
        }
        public void ExecuteChanged(object sender, EventArgs e)
        {
            CanExecuteChanged?.Invoke(sender, e);
        }
        Action<object> m_execute;
        Func<object, bool> m_canExec;
    }

    public class ChatViewModel : INotifyPropertyChanged, IChatServerCallback
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public ChatViewModel()
        {
            m_registered = false;
            Textpad = "Type username here and press 'Register'...";
            History = new FlowDocument();

            m_service = new ChatServerClient(new InstanceContext(this));
            OnSendPressed = new RelayCommand((o) => onSendPressedHandler(), (o) => m_registered);
            OnRegisterPressed = new RelayCommand((o) => onRegisterPressedHandler(), (o) => !m_registered);
            OnClose = new RelayCommand((o) => m_service.Unregister());
        }

        public FlowDocument History
        { get; }
        public string Textpad
        { get; set; }

        public RelayCommand OnSendPressed
        { get; }
        public RelayCommand OnRegisterPressed
        { get; }
        public RelayCommand OnClose
        { get; }

        void IChatServerCallback.DeliverMessage(Post msg)
        {
            Paragraph p = new Paragraph(new Run(msg.Username + ": " + msg.Content));
            p.FontSize = 10;
            p.FontFamily = new FontFamily("Arial");
            History?.Blocks.Add(p);
            propertyChanged(nameof(History));
        }

        void onRegisterPressedHandler()
        {
            try {
                if (Textpad == null) return;
                m_registered = m_service.Register(Textpad);
                Textpad = "";
                propertyChanged(nameof(Textpad));
                OnRegisterPressed.ExecuteChanged(null, null);
                OnSendPressed.ExecuteChanged(null, null);
            }
            catch (EndpointNotFoundException) {
                System.Environment.Exit(0);
            }
        }
        void onSendPressedHandler()
        {
            if (Textpad == null) return;
            m_service.PostMessage(Textpad);
            Textpad = "";
            propertyChanged(nameof(Textpad));
        }
        void propertyChanged(string propName)
        {
            PropertyChanged(this, new PropertyChangedEventArgs(propName));
        }

        bool m_registered;
        ChatServerClient m_service;
    }
}
