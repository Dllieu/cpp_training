using System;
using System.Collections.Generic;
using System.Net;
using Bing;

namespace SubtitlesDownloaderWPF.SearchStrategies.Bing
{
    public class BingRequestor : BingSearchContainer
    {
        public BingRequestor(string accountKey)
            : base(new Uri("https://api.datamarket.azure.com/Bing/Search/v1/Web"))
        {
            Credentials = new NetworkCredential(accountKey, accountKey);
        }

        /// <summary>
        /// Execute Query In Web (can throw)
        /// </summary>
        /// <param name="query"></param>
        /// <returns></returns>
        public IEnumerable<WebResult> ExecuteQueryInWeb(string query)
        {
            var searchQuery = Web(query, null, null, null, null, null, null, null);
            return searchQuery.Execute();
        }
    }
}
