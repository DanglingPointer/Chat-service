using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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

namespace ChatClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            //string ipaddr = Interaction.InputBox("Enter server's IPv4-address:", "Establishing connection...", "127.0.0.1");
            //string portnum = Interaction.InputBox("Enter server's port number:", "Establishing connection...", "1234");
            string ipaddr;
            int port;
            StartupEventArgs e = ((App)Application.Current).CmdParams;

            if (e.Args.Length == 0)
            {
                ipaddr = "127.0.0.1";
                port = 1234;
            }
            else if (e.Args.Length == 2)
            {
                ipaddr = e.Args[0];
                port = Convert.ToInt32(e.Args[1]);
            }
            else
            {
                MessageBox.Show("Wrong parameters");
                Environment.Exit(-1);
            }
        }
        private void OnTextPadKeyPressed(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                Paragraph p = new Paragraph(new Run("--"+DateTime.Now.ToLongTimeString() +  "--\n" + textpad.Text));
                p.FontSize = 9;
                p.FontFamily = new FontFamily("Arial");
                chattext.Blocks.Add(p);
                textpad.Text = "";
            }
        }
        private void OnClosedClick(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
        private void OnInfoClick(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("you clicked info!");
        }
        private void OnNamesClick(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("you clicked names!");
        }
        private void OnWindowClosed(object sender, EventArgs e)
        {
            // Disconnect (logout)
        }
    }
}
