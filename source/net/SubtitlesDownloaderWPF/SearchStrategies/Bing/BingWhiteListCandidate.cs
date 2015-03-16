namespace SubtitlesDownloaderWPF.SearchStrategies.Bing
{
    /// <summary>
    /// Websites in white list means that if we query google, we will try to find them in the result, which mean they are the more appropriate results
    /// Dictionary is composed by the hostname, and a regexp to find the download link once we are on the page
    /// </summary>
    public class BingWhiteListCandidate
    {
        public string Hostname { get; set; }
        public string DownloadLinkRegex { get; set; }
    }
}