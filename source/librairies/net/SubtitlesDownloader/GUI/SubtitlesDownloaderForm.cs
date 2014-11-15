using System;
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

        static private void OnDragDropEvent(object sender, DragEventArgs e)
        {
            if (e == null)
                return;

            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            foreach (var file in files)
            {
                var f = new ResultChoiceProcessor();
                f.GetTopResultFromFile(file);
                break;
            }

            
            // show current file in textbox, then if its terminated by divx avi mp4, ask if it want to dowlaod subtitle from it and with which language
        }
    }
}
