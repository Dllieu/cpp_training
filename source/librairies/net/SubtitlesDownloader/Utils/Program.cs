using System;
using System.Diagnostics;
using System.Linq;
using System.Windows.Forms;

namespace SubtitlesDownloader
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            // TODO : pass Bing account key by conf
            Debug.Assert(args.Any());

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new SubtitlesDownloaderForm(args[0]));
        }
    }
}
