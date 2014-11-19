using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using System.Windows.Media;
using Bing;

namespace SubtitlesDownloaderWPF
{
    /// <summary>
    /// Interaction logic for SubtitleDownloaderView.xaml
    /// </summary>
    public partial class SubtitleDownloaderView : Window
    {
        public ObservableCollection<SearchResult> SearchResults { get; private set; }
        private readonly ResultChoiceProcessor _resultChoiceProcessor;

        public SubtitleDownloaderView()
        {
            SearchResults = new ObservableCollection<SearchResult>();
            SearchResults.Add(new SearchResult {Title="Super Titre"});

            // TODO : bingAccountKe in conf
            _resultChoiceProcessor = new ResultChoiceProcessor(Environment.GetCommandLineArgs()[1]);
            _resultChoiceProcessor.OnErrorEvent += OnError;
            _resultChoiceProcessor.OnResultsEvent += OnResults;

            InitializeComponent();
        }

        /// <summary>
        /// On Error
        /// </summary>
        /// <param name="error"></param>
        private void OnError(string error)
        {
            //SearchErrors.Add(new DateTime.Now, error);
            //errorMenuStripLabel.Text = string.Format("{0} Errors", errorDataGridView.Rows.Count);
            //errorMenuStripLabel.Enabled = true;
            Console.WriteLine(error);
        }

        /// <summary>
        /// Add WebResult Row
        /// </summary>
        /// <param name="webResult"></param>
        /// <param name="isWhiteListed"></param>
        private void AddWebResultRow(WebResult webResult, bool isWhiteListed)
        {
            if (webResult != null)
                SearchResults.Add(new SearchResult
                {
                    IsWhiteListed = isWhiteListed,

                    Title = webResult.Title,
                    Provider = webResult.DisplayUrl,
                    DownloadLink = webResult.Url,
                    Description = webResult.Description,

                    Color = Colors.Cyan
                });
        }

        /// <summary>
        /// On Results
        /// </summary>
        /// <param name="whiteListResults"></param>
        /// <param name="otherResults"></param>
        private void OnResults(List<WebResult> whiteListResults, List<WebResult> otherResults)
        {
            SearchResults.Clear();

            whiteListResults.ForEach(s => AddWebResultRow(s, true));
            otherResults.ForEach(s => AddWebResultRow(s, false));
        }

        /// <summary>
        /// GlobalPanel_OnDrop
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void GlobalPanel_OnDrop(object sender, DragEventArgs e)
        {
            if (e == null)
                return;

            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files == null || !files.Any())
                return;

            //filenameTextBox.Text = files.First();
            _resultChoiceProcessor.RequestSubtitleFromFile(files.First());
            // show current file in textbox, then if its terminated by divx avi mp4, ask if it want to dowlaod subtitle from it and with which language
        }
    }
}
