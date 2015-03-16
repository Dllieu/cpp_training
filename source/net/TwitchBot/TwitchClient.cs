using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Text.RegularExpressions;
using System.Windows.Controls;
using System.Windows.Documents;

namespace TwitchBot
{
    public class TwitchClient
    {
        #region ATTRIBUTES
        public string Login { get; set; }
        public string Password { get; set; }
        public string Channel { get; set; }

        public Regex MessageRegex { get; set; }
        #endregion

        public TwitchClient(string login, string password, string channel)
        {
            Login = login;
            Password = password;
            Channel = channel;

            MessageRegex = new Regex(string.Format(@"^\:([a-zA-Z0-9_]+)\!.*PRIVMSG \#{0} :(.*)$", Channel));
        }

        static private byte[] ToByte(string message)
        {
            return System.Text.Encoding.ASCII.GetBytes(string.Format("{0}\r\n", message));
        }

        private bool ProcessMessage(string line, TextBlock chatTextBox)
        {
            // :doeboybruh!doeboybruh@doeboybruh.tmi.twitch.tv PRIVMSG #calvinman17 :bla bla bla bla
            if (!line.Contains("PRIVMSG"))
                return false;

            var matchesInput = MessageRegex.Matches(line);
            if (matchesInput.Count < 1)
                return false;

            try
            {
                chatTextBox.Dispatcher.Invoke(() =>
                {
                    chatTextBox.Inlines.Add(new Bold(new Run(matchesInput[0].Groups[1].ToString())));
                    chatTextBox.Inlines.Add(string.Format(": {0}\r\n", matchesInput[0].Groups[2]));
                });
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
            //Console.WriteLine("{0}: {1}", matchesInput[0].Groups[1], matchesInput[0].Groups[2]);
            return true;
        }

        public void Connect(TextBlock chatTextBlock)
        {
            // recup liste des chats dispo a partir de http://twitchstatus.com/
            try
            {
                using (var client = new TcpClient("199.9.249.252", 80))
                using (var stream = client.GetStream())
                using (var reader = new StreamReader(stream))
                using (var writer = new StreamWriter(stream) { NewLine = "\r\n", AutoFlush = true })
                {
                    writer.WriteLine(string.Format("PASS {0}", Password));
                    writer.WriteLine(string.Format("NICK {0}", Login));
                    writer.WriteLine(string.Format("JOIN #{0}", Channel));

                    // 1 - receive list of all users
                    // 2 - receive status about the rights
                    // 3 - receive anything else
                    for (; ; )
                    {
                        Thread.Sleep(1);
                        var line = reader.ReadLine();
                        if (string.IsNullOrEmpty(line))
                            continue;

                        if (ProcessMessage(line, chatTextBlock))
                            continue;

                        if (line.StartsWith("PING"))
                        {
                            writer.WriteLine(line.Replace("PING", "PONG"));
                            continue;
                        }

                        Console.WriteLine(line);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }

            //if (socket_ == null)
            //    socket_ = new Socket(EndPoint.AddressFamily, SocketType.Stream, ProtocolType.Udp); // CHECK
            //else
            //    socket_.Disconnect(true);

            //socket_.Connect(EndPoint);
            //if (! socket_.Connected)
            //    return;

            ////HOST="199.9.253.199" ##This is the Twitch IRC ip, don't change it.
            ////PORT=6667 ##Same with this port, leave it be.
            ////NICK="KanthBot" ##This has to be your bots username.
            ////PASS="testpass1" ##Instead of a password, use this http://twitchapps.com/tmi/, since Twitch is soon updating to it.
            ////IDENT="KanthBot" ##Bot username again
            ////REALNAME="Kanthes Bot" ##This doesn't really matter.
            ////CHANNEL="#kanthes" ##This is the channel your bot will be working on.

            ////s.send("PASS %s\r\n" % PASS) ##Notice how I'm sending the password BEFORE the username!
            ////##Just sending the rest of the data now.
            ////s.send("NICK %s\r\n" % NICK)
            ////s.send("USER %s %s bla :%s\r\n" % (IDENT, HOST, REALNAME))
            ////##Connecting to the channel.
            ////s.send("JOIN %s\r\n" % CHANNEL)

            //socket_.Send(ToByte(string.Format("PASS {0}", Password)));
            //socket_.Send(ToByte(string.Format("NICK {0}", Login)));
            //socket_.Send(ToByte(string.Format("USER {0} {1} bla :{2}", Login, Host, Login)));
            //socket_.Send(ToByte(string.Format("JOIN #{0}", Channel)));

            //var buffer = new byte[1024];
            //socket_.Receive(buffer);
        }
    }
}
