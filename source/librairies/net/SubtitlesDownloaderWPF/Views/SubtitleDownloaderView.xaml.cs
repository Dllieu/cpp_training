using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using SubtitlesDownloaderWPF.Models;
using SubtitlesDownloaderWPF.SearchStrategies;
using SubtitlesDownloaderWPF.SearchStrategies.Bing;

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

        public SubtitleDownloaderView()
        {
            SearchResults = new ObservableCollection<SucceedResultModel>();
            SearchErrors = new ObservableCollection<ErrorResultModel>();

            // TODO : bingAccountKey in conf
            _searchStrategy = new BingSearchStrategy(Environment.GetCommandLineArgs()[1]);

            InitializeComponent();
        }

        /// <summary>
        /// GlobalPanel_OnDrop
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void GlobalPanel_OnDrop(object sender, DragEventArgs e)
        {
            if (e == null || e.Data == null)
                return;

            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files == null || !files.Any())
                return;

            SearTextBox.Text = files.First();
            try
            {
                await Task.Run(() =>
                {
                    var results = _searchStrategy.SearchSubtitles(files.First());
                    Dispatcher.Invoke(() =>
                    {
                        SearchResults.Clear();
                        results.ForEach(SearchResults.Add); 
                    });
                });
            }
            catch (Exception ex)
            {
                SearchErrors.Insert(0, new ErrorResultModel {Timestamp = DateTime.Now, ErrorMessage = ex.Message});
            }
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
    }
}
