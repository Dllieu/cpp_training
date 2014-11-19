using System.IO;
using System.Linq;
using System;
using System.Collections.Generic;
using Bing;

namespace SubtitlesDownloader
{
    /// <summary>
    /// Request google
    /// -> get top N best website
    /// -> check which website we whitelisted (they will come first) should be highlited in different color in the GUI
    /// -> if user click one of them, use regexp ? to find the link (if not white listed, it should open the page directly through a browser)
    /// TODO: thread safety
    /// </summary>
    public class ResultChoiceProcessor
    {
        public Action<string> OnErrorEvent;
        public Action<List<WebResult>, List<WebResult>> OnResultsEvent;

        public List<string> AcceptedFileExtension { get; set; }
        public int ResultPerRequest { get; set; }
        private HashSet<WhiteListCandidate> WhiteListCandidates { get; set; }
        private readonly BingRequestor _bingRequestor;

        /// <summary>
        /// TODO: init from conf
        /// </summary>
        public ResultChoiceProcessor(string bingAccountKey/*should take a conf to init whitelist stuff*/)
        {
            _bingRequestor = new BingRequestor(bingAccountKey);

            WhiteListCandidates = new HashSet<WhiteListCandidate>
            {
                new WhiteListCandidate {
                    Hostname = "opensubtitles.org",
                    DownloadLinkRegex = @"opensubtitles.org/.*/download/sub/.*"
                },
                new WhiteListCandidate {
                    Hostname = "yifysubtitles.com",
                    DownloadLinkRegex = @"yifysubtitles.com/subtitle/"
                },
            };

            AcceptedFileExtension = new List<string>
            {
                ".divx",
                ".avi",
                ".mp4",
                ".mkv"
            };
            ResultPerRequest = 10;
        }

        /// <summary>
        /// On Error
        /// </summary>
        /// <param name="error"></param>
        private void OnError(string error)
        {
            var handler = OnErrorEvent;
            if (handler != null)
                handler(error);
        }

        /// <summary>
        /// On Results
        /// </summary>
        /// <param name="whiteListResults"></param>
        /// <param name="otherResults"></param>
        private void OnResults(List<WebResult> whiteListResults, List<WebResult> otherResults)
        {
            var handler = OnResultsEvent;
            if (handler != null)
                handler(whiteListResults, otherResults);
        }

        /// <summary>
        /// Process Raw Results, make white list hostname appear first
        /// </summary>
        /// <param name="rawResults"></param>
        private void ProcessRawResults(IEnumerable<WebResult> rawResults)
        {
            if (rawResults == null)
            {
                OnResults(null, null);
                return;
            }

            var whiteListResults = new List<WebResult>(ResultPerRequest);
            var otherResults = new List<WebResult>(ResultPerRequest);
            foreach (var rawResult in rawResults)
            {
                if (WhiteListCandidates.Any(h => rawResult.DisplayUrl.IndexOf(h.Hostname, StringComparison.OrdinalIgnoreCase) >= 0))
                {
                    whiteListResults.Add(rawResult);
                    if (whiteListResults.Count >= ResultPerRequest)
                        break;
                    continue;
                }

                if (whiteListResults.Count + otherResults.Count < ResultPerRequest)
                    otherResults.Add(rawResult);
            }

            if (whiteListResults.Count + otherResults.Count > ResultPerRequest)
            {
                var numberOfItemsToRemove = otherResults.Count + whiteListResults.Count - ResultPerRequest;
                otherResults.RemoveRange(otherResults.Count - numberOfItemsToRemove, numberOfItemsToRemove);
            }
            OnResults(whiteListResults, otherResults);
        }

        /// <summary>
        /// Request Subtitle From File
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public void RequestSubtitleFromFile(string file)
        {
            if (file == null)
            {
                OnError(string.Format("Request subtitle from null file"));
                return;
            }

            var filenameExtension = Path.GetExtension(file);
            if (AcceptedFileExtension.All(s => string.Compare(s, filenameExtension, true) != 0))
            {
                OnError(string.Format("File format not handled: \"{0}\"", filenameExtension));
                return;
            }

            var filenameWithoutExtension = Path.GetFileNameWithoutExtension(file);
            try
            {
                ProcessRawResults(_bingRequestor.ExecuteQueryInWeb(string.Format("\"{0}\" srt english", filenameWithoutExtension)));
            }
            catch (Exception ex)
            {
                OnError(ex.Message);
            }
        }
    }
}
