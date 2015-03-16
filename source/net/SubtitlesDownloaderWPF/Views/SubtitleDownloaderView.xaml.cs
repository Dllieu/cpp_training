using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using SubtitlesDownloaderWPF.Models;
using SubtitlesDownloaderWPF.SearchStrategies;
using SubtitlesDownloaderWPF.SearchStrategies.Bing;

// todo : presenter comme github (presenter juste url avec une arrow si click il y a les details)
namespace SubtitlesDownloaderWPF.Views
{
    /// <summary>
    /// Interaction logic for SubtitleDownloaderView.xaml
    /// </summary>
    public partial class SubtitleDownloaderView : Window
    {
        public ObservableCollection<SucceedResultModel> SearchResults { get; private set; }
        public ObservableCollection<ErrorResultModel> SearchErrors { get; private set; }
        private readonly ISearchStrategy _searchStrategy;
        private readonly int _timeout;

        public SubtitleDownloaderView()
        {
            SearchResults = new ObservableCollection<SucceedResultModel>();
            SearchErrors = new ObservableCollection<ErrorResultModel>();

            // TODO : bingAccountKey in conf
            _searchStrategy = new BingSearchStrategy(Environment.GetCommandLineArgs()[1]);
            _timeout = 10000;

            InitializeComponent();
        }

        /// <summary>
        /// CallFunctorOrTimeout
        /// return true if we could call the fucntor within timeout timeline
        /// </summary>
        /// <param name="functor"></param>
        /// <param name="timeout"></param>
        /// <returns></returns>
        static private async Task<bool> CallFunctorOrTimeout(Action<CancellationToken> functor, int timeout)
        {
            var tokenSource = new CancellationTokenSource();
            var token = tokenSource.Token;
            var task = new Task(() => functor(token));
            task.Start();

            if (await Task.WhenAny(task, Task.Delay(timeout, token)) != task)
                return false; // timeout

            // We re-await the task so that any exceptions/cancellation is rethrown.
            await task;
            return true;
        }

        /// <summary>
        /// SearchSubtitle
        /// </summary>
        /// <param name="file"></param>
        private async void SearchSubtitle(string file)
        {
            SearTextBox.Text = file;
            try
            {
                var result = await CallFunctorOrTimeout(token =>
                    {
                        var results = _searchStrategy.SearchSubtitle(file);
                        Dispatcher.Invoke(() =>
                        {
                            SearchResults.Clear();
                            results.ForEach(SearchResults.Add);
                        });
                    }, _timeout);

                if (!result)
                    throw new Exception(string.Format("Search subtitle timeout {0}ms for file '{1}'", _timeout, file));
            }
            catch (Exception ex)
            {
                SearchErrors.Insert(0, new ErrorResultModel { Timestamp = DateTime.Now, ErrorMessage = ex.Message });
            }
        }

        /// <summary>
        /// GlobalPanel_OnDrop
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void GlobalPanel_OnDrop(object sender, DragEventArgs e)
        {
            if (e == null || e.Data == null)
                return;

            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files == null || !files.Any())
                return;

            SearchSubtitle(files.First());
        }

        /// <summary>
        /// OpenFile
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OpenFile(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFileDialog();

            var result = dialog.ShowDialog();
            if (result.HasValue && result.Value)
                SearchSubtitle(dialog.FileName);
        }

        /// <summary>
        /// ErrorsContextMenu_Click
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ErrorsContextMenu_Click(object sender, RoutedEventArgs e)
        {
            SearchErrors.Clear();
        }

        /// <summary>
        /// ErrorsContextMenu_Click
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ShowOrHideErrorsResult_Click(object sender, RoutedEventArgs e)
        {
            SearchErrorRow.Height = SearchErrorRow.Height.IsStar ? new GridLength(0) : new GridLength(1, GridUnitType.Star);
            SearchErrorsListView.Visibility = SearchErrorsListView.IsVisible ? Visibility.Collapsed : Visibility.Visible;
        }

        /// <summary>
        /// ShowOrhideMainStatusBar_Click
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ShowOrhideMainStatusBar_Click(object sender, RoutedEventArgs e)
        {
            MainStatusBar.Visibility = MainStatusBar.IsVisible ? Visibility.Collapsed : Visibility.Visible;
        }

        /// <summary>
        /// ExitApplication
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ExitApplication(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        /// <summary>
        /// DoubleClickSearchResultListView
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void DoubleClickSearchResultListView(object sender, MouseButtonEventArgs e)
        {
            if (SearchResultListView.SelectedIndex == -1)
                return;

            var selectedItem = (SucceedResultModel) SearchResultListView.SelectedItem;
            if (selectedItem == null)
                return;

            try
            {
                if (! await CallFunctorOrTimeout(token => _searchStrategy.DownloadSubtitle(selectedItem), _timeout))
                    throw new Exception(string.Format("Download subtitle timeout {0}ms on '{1}'", _timeout, selectedItem.DownloadLink));
            }
            catch (Exception ex)
            {
                SearchErrors.Insert(0, new ErrorResultModel { Timestamp = DateTime.Now, ErrorMessage = ex.Message });
            }
        }
    }
}
