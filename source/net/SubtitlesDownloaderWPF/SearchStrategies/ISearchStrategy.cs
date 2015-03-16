using System.Collections.Generic;
using SubtitlesDownloaderWPF.Models;

namespace SubtitlesDownloaderWPF.SearchStrategies
{
    public interface ISearchStrategy
    {
        /// <summary>
        /// SearchSubtitle : throw in case of error
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        List<SucceedResultModel> SearchSubtitle(string filename);
        void DownloadSubtitle(SucceedResultModel succeedResult);
    }
}
