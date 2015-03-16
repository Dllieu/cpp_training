using System.IO;
using System.Linq;
using System;
using System.Collections.Generic;
using System.Net;
using System.Text.RegularExpressions;
using Bing;
using SubtitlesDownloaderWPF.Models;

namespace SubtitlesDownloaderWPF.SearchStrategies.Bing
{
    /// <summary>
    /// Request google
    /// -> get top N best website
    /// -> check which website we whitelisted (they will come first) should be highlited in different color in the GUI
    /// -> if user click one of them, use regexp ? to find the link (if not white listed, it should open the page directly through a browser)
    /// </summary>
    public class BingSearchStrategy : ISearchStrategy
    {
        private List<string> AcceptedFileExtension { get; set; }
        private int ResultPerRequest { get; set; }
        private HashSet<BingWhiteListCandidate> WhiteListCandidates { get; set; }
        private readonly BingRequestor _bingRequestor;

        /// <summary>
        /// TODO: init from conf
        /// </summary>
        public BingSearchStrategy(string bingAccountKey/*should take a conf to init whitelist stuff*/)
        {
            _bingRequestor = new BingRequestor(bingAccountKey);

            WhiteListCandidates = new HashSet<BingWhiteListCandidate>
            {
                new BingWhiteListCandidate {
                    Hostname = "opensubtitles.org",
                    DownloadLinkRegex = "\"(http://dl.opensubtitles.org/.*?/download/sub/.*?)\""
                },
                //new BingWhiteListCandidate {
                //    Hostname = "subscene.com",
                //    DownloadLinkRegex = "\"(http://subscene.com/subtitle/download.*?)\""
                //},
            };

            AcceptedFileExtension = new List<string>
            {
                ".divx",
                ".avi",
                ".mp4",
                ".mkv"
            };

            // TODO : Not taken in account by bing
            ResultPerRequest = 10;
        }

        /// <summary>
        /// BingResultToModelResult
        /// </summary>
        /// <param name="webResult"></param>
        /// <param name="isWhiteListed"></param>
        /// <returns></returns>
        static private SucceedResultModel BingResultToModelResult(WebResult webResult, bool isWhiteListed)
        {
            // Quick and dirty
            string provider;
            try
            {
                provider = new Uri(webResult.Url).Host;
            }
            catch (Exception)
            {
                provider = webResult.DisplayUrl;
            }

            return new SucceedResultModel
            {
                IsWhiteListed = isWhiteListed,

                Title = webResult.Title,
                Provider = provider,
                DownloadLink = webResult.Url,
                Description = webResult.Description,
            };
        }

        /// <summary>
        /// Process Raw Results, make white list hostname appear first
        /// </summary>
        /// <param name="rawResults"></param>
        private List<SucceedResultModel> ProcessRawResults(IEnumerable<WebResult> rawResults)
        {
            if (rawResults == null)
                throw new Exception("Null result from bing");

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

            var result = new List<SucceedResultModel>();
            whiteListResults.ForEach(r => result.Add(BingResultToModelResult(r, true)));
            otherResults.ForEach(r => result.Add(BingResultToModelResult(r, false)));
            return result;
        }

        /// <summary>
        /// Search Subtitle From File
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public List<SucceedResultModel> SearchSubtitle(string file)
        {
            if (file == null)
                throw new Exception("Request subtitle from null file");

            var filenameExtension = Path.GetExtension(file);
            if (AcceptedFileExtension.All(s => string.Compare(s, filenameExtension, true) != 0))
                throw new Exception(string.Format("Format not handled: \"{0}\" ({1})", filenameExtension, file));

            try
            {
                return ProcessRawResults(_bingRequestor.ExecuteQueryInWeb(string.Format("\"{0}\" srt english", Path.GetFileNameWithoutExtension(file))));
            }
            catch (Exception ex)
            {
                throw new Exception(string.Format("Unexpected exception while requesting bing: \"{0}\"", ex));
            }
        }

        /// <summary>
        /// DownloadSubtitle
        /// </summary>
        /// <param name="succeedResult"></param>
        public void DownloadSubtitle(SucceedResultModel succeedResult)
        {
            if (succeedResult == null)
                throw new Exception("Download subtitle from a null search result");

            var whiteListCandidate = WhiteListCandidates.FirstOrDefault(h => succeedResult.Provider.IndexOf(h.Hostname, StringComparison.OrdinalIgnoreCase) >= 0);
            if (whiteListCandidate == null)
            {
                System.Diagnostics.Process.Start(succeedResult.DownloadLink);
                return;
            }

            try
            {
                using (var client = new WebClient())
                {
                    var htmlCode = client.DownloadString(succeedResult.DownloadLink);
                    var match = Regex.Match(htmlCode, whiteListCandidate.DownloadLinkRegex);
                    var realDownloadLink = match.Groups[1].Value;

                    // Just open a browser with the download link
                    System.Diagnostics.Process.Start(realDownloadLink);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                throw;
            }
        }
    }
}
