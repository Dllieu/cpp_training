namespace SubtitlesDownloaderWPF.Models
{
    public class SucceedResultModel
    {
        public bool IsWhiteListed { get; set; }

        public string Title { get; set; }
        public string Provider { get; set; }
        public string DownloadLink { get; set; }
        public string Description { get; set; }
    }
}
