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
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace TwitchBot
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private TwitchClient client_;

        public MainWindow()
        {
            InitializeComponent();

            client_ = new TwitchClient("NotABotz", "oauth:3qy8u4gjs1tufelk88vzzvovg3g1fwo", "gunnermaniac3");
            Task.Factory.StartNew(() => client_.Connect(ChatTextBlock));
        }
    }
}
