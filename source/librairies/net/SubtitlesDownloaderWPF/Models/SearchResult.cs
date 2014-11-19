using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SubtitlesDownloaderWPF
{
    public class SearchResult
    {
        public string Title { get; set; }
        public string Provider { get; set; }
        public string DownloadLink { get; set; }
        public string Description { get; set; }

        public bool IsWhiteListed { get; set; }
    }
}
