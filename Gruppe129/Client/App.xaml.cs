using System.Windows;

namespace Chat
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public StartupEventArgs CmdParams;
        private void OnAppStartUp(object sender, StartupEventArgs e)
        {
            CmdParams = e;
        }
    }
}
