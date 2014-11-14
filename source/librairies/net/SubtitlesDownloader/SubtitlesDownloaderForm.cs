using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using sdwrapper;

namespace SubtitlesDownloader
{
    public partial class SubtitlesDownloaderForm : Form
    {
        public SubtitlesDownloaderForm()
        {
            InitializeComponent();

            AllowDrop = true;
            DragDrop += new DragEventHandler(OnDragDropEvent);
            DragEnter += new DragEventHandler(OnDragEnterEvent);
            var b = new Scanner( 5 );
            b.scan("ee").ForEach(Console.WriteLine);
        }

        /// <summary>
        /// Modify cursor
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        static private void OnDragEnterEvent(object sender, DragEventArgs e)
        {
            if (e != null && e.Data.GetDataPresent(DataFormats.FileDrop))
                e.Effect = DragDropEffects.Move;
        }

        private void OnDragDropEvent(object sender, DragEventArgs e)
        {
            if (e != null)
                ;
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            foreach (string file in files) Console.WriteLine(file);
        }
    }
}
