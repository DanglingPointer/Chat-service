﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using Microsoft.VisualBasic;
using System.Windows.Media;
using System.Windows.Markup;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Chat.Formats;
using Chat.Client;

namespace Chat
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            string ipaddr = "127.0.0.1";
            int port = 1234;

            StartupEventArgs e = ((App)Application.Current).CmdParams;            
            if (e.Args.Length == 2)
            {
                ipaddr = e.Args[0];
                port = Convert.ToInt32(e.Args[1]);
            }
            else if (e.Args.Length != 0)
            {
                MessageBox.Show("Program started with invalid parameters", "Error");
                Environment.Exit(-1);
            }
            try
            {
                m_client = new ChatClient(ipaddr, port);
                //m_client.ConnectionLost += ShowAbortMsg;
                m_client.ErrorReceived += OnErrorReceived;
                m_client.InfoReceived += OnInfoReceived;
                m_client.MessageReceived += OnMessageReceived;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
                Environment.Exit(-2);
            }
        }
        private void OnTextPadKeyPressed(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                var msg = textpad.Text;
                textpad.Text = "";
                m_client.SendMessage(msg);
            }
        }
        /// <summary>
        /// Login button pressed
        /// </summary>
        private void OnLoginClick(object sender, RoutedEventArgs e)
        {
            string name = Interaction.InputBox("Enter username:", "Login");
            m_client.LogIn(name);
        }
        /// <summary>
        /// Info button pressed
        /// </summary>
        private void OnInfoClick(object sender, RoutedEventArgs e)
        {
            m_client.RequestHelp();
        }
        /// <summary>
        /// Names button pressed
        /// </summary>
        private void OnNamesClick(object sender, RoutedEventArgs e)
        {
            m_client.RequestNames();
        }
        /// <summary>
        /// Window has been closed
        /// </summary>
        private void OnWindowClosed(object sender, EventArgs e)
        {
            m_client.LogOut();
        }
        private void OnErrorReceived(string errmsg)
        {
            MessageBox.Show(errmsg, "Error");
        }
        private void OnInfoReceived(string infomsg)
        {
            MessageBox.Show(infomsg, "Information");
        }
        private void OnMessageReceived(string msg)
        {
            Paragraph p = new Paragraph(new Run("-- "+DateTime.Now.ToLongTimeString() +  " --\n" + msg));
            p.FontSize = 9;
            p.FontFamily = new FontFamily("Arial");
            chattext.Blocks.Add(p);
        }
        private void OnWindowLoaded(object sender, RoutedEventArgs e)
        {
            Dispatcher.CurrentDispatcher.BeginInvoke(DispatcherPriority.ApplicationIdle,
                (Action)(() =>
                {
                    m_client.Run();
                }));
            OnLoginClick(null, null);
        }
        ChatClient m_client;
    }
}
