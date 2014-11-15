using System;
using System.Collections.Generic;

namespace SubtitlesDownloader
{
    /// <summary>
    /// Request google
    /// -> get top N best website
    /// -> check which website we whitelisted (they will come first) should be highlited in different color in the GUI
    /// -> if user click one of them, use regexp ? to find the link (if not white listed, it should open the page directly through a browser)
    /// </summary>
    public class ResultChoiceProcessor
    {
        public HashSet<WhiteListCandidate> WhiteListCandidates { get; private set; }

        /// <summary>
        /// TODO: init from conf
        /// </summary>
        public ResultChoiceProcessor( /*should take a conf to init whitelist stuff*/)
        {
            // TODO: init from conf
            WhiteListCandidates = new HashSet<WhiteListCandidate>
            {
                new WhiteListCandidate {
                    Hostname = "opensubtitles",
                    DownloadLinkRegex = @"opensubtitles.org/.*/download/sub/.*"
                },
                new WhiteListCandidate {
                    Hostname = "yifysubtitles",
                    DownloadLinkRegex = @"yifysubtitles.com/subtitle/"
                },
            };
        }

        /// <summary>
        /// Result is never null
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        public List<CrawlerResult> GetTopResultFromFile(string filename)
        {
            var result = new List<CrawlerResult>();

            // faire la requete dans une autre classe, et a processer (parser) ici
            //return result;

            //http://www.codeproject.com/Articles/601329/Bing-it-on-Reactive-Extensions-Story-code-and-slid

            //https://onedrive.live.com/view.aspx?resid=9C9479871FBFA822!112&app=Word&authkey=!ANNnJQREB0kDC04
            //https://datamarket.azure.com/receipt/aef2d79f-74bf-4b63-8642-fa520b31491d?ctpa=False

            return result;
        }
    }
}
