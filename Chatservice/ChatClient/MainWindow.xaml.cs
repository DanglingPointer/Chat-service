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

        }
        private void OnTextPadKeyPressed(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                Paragraph p = new Paragraph(new Run(textpad.Text));
                p.FontSize = 9;
                p.FontFamily = new FontFamily("Arial");
                chattext.Blocks.Add(p);
                textpad.Text = "";
            }
        }
        private void OnSendClick(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("you clicked send!");
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
