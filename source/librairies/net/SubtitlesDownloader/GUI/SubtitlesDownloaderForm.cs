using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using Bing;
using sdwrapper;

namespace SubtitlesDownloader
{
    public partial class SubtitlesDownloaderForm : Form
    {
        private readonly ResultChoiceProcessor _resultChoiceProcessor;

        public SubtitlesDownloaderForm(string bingAccountKey)
        {
            InitializeComponent();

            errorPanel.Visible = false;

            AllowDrop = true;
            DragDrop += new DragEventHandler(OnDragDropEvent);
            DragEnter += new DragEventHandler(OnDragEnterEvent);

            _resultChoiceProcessor = new ResultChoiceProcessor(bingAccountKey);
            _resultChoiceProcessor.OnErrorEvent += OnError;
            _resultChoiceProcessor.OnResultsEvent += OnResults;
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

        /// <summary>
        /// On Drag Drop Event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDragDropEvent(object sender, DragEventArgs e)
        {
            if (e == null)
                return;

            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files == null || ! files.Any())
                return;

            filenameTextBox.Text = files.First();
            _resultChoiceProcessor.RequestSubtitleFromFile(files.First());
            // show current file in textbox, then if its terminated by divx avi mp4, ask if it want to dowlaod subtitle from it and with which language
        }

        /// <summary>
        /// On Error
        /// </summary>
        /// <param name="error"></param>
        private void OnError(string error)
        {
            errorDataGridView.Rows.Insert(0, DateTime.Now, error);
            errorMenuStripLabel.Text = string.Format("{0} Errors", errorDataGridView.Rows.Count);
            errorMenuStripLabel.Enabled = true;
        }

        /// <summary>
        /// Add WebResult Row
        /// </summary>
        /// <param name="webResult"></param>
        private void AddWebResultRow(WebResult webResult)
        {
            if (webResult != null)
                resultFromSearchGridView.Rows.Add(webResult.Title, webResult.DisplayUrl, webResult.Description);
        }

        /// <summary>
        /// On Results
        /// </summary>
        /// <param name="whiteListResults"></param>
        /// <param name="otherResults"></param>
        private void OnResults(List<WebResult> whiteListResults, List<WebResult> otherResults)
        {
            resultFromSearchGridView.Rows.Clear();

            whiteListResults.ForEach(AddWebResultRow);
            otherResults.ForEach(AddWebResultRow);
        }

        #region MENU_EVENT
        private void DisplayErrorPanel(object sender, EventArgs e)
        {
            errorPanel.Visible = displayErrorToolStripMenuItem.Checked ^= true;
        }

        private void statusBarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            statusStrip.Visible = statusBarToolStripMenuItem.Checked ^= true;
        }
        #endregion
    }
}
