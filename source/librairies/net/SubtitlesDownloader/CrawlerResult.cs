namespace SubtitlesDownloader
{
    /// <summary>
    /// Result representing a possible website candidate for which we could download a specific subtitle
    /// </summary>
    public class CrawlerResult
    {
        public string Hostname { get; set; }
        public string Summary { get; set; }
    }
}
