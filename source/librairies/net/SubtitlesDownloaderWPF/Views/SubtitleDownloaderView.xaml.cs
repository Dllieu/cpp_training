using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
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
        /// SearchSubtitle
        /// </summary>
        /// <param name="file"></param>
        private async void SearchSubtitle(string file)
        {
            SearTextBox.Text = file;
            try
            {
                await Task.Run(() =>
                {
                    var results = _searchStrategy.SearchSubtitle(file);
                    Dispatcher.Invoke(() =>
                    {
                        SearchResults.Clear();
                        results.ForEach(SearchResults.Add);
                    });
                });
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
                // http://stackoverflow.com/questions/4238345/asynchronously-wait-for-taskt-to-complete-with-timeout
                await Task.Run(() =>
                {
                    _searchStrategy.DownloadSubtitle(selectedItem);
                    //Dispatcher.Invoke(() =>
                    //{
                        
                    //});
                });
            }
            catch (Exception ex)
            {
                SearchErrors.Insert(0, new ErrorResultModel { Timestamp = DateTime.Now, ErrorMessage = ex.Message });
            }
        }
    }
}
