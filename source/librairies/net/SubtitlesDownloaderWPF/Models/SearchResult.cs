using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace SubtitlesDownloaderWPF
{
    public class SearchResult
    {
        public bool IsWhiteListed { get; set; }

        public string Title { get; set; }
        public string Provider { get; set; }
        public string DownloadLink { get; set; }
        public string Description { get; set; }
    }
}
