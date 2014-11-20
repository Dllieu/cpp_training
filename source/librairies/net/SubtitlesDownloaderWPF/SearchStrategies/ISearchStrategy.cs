using System.Collections.Generic;
using SubtitlesDownloaderWPF.Models;

namespace SubtitlesDownloaderWPF.SearchStrategies
{
    public interface ISearchStrategy
    {
        /// <summary>
        /// SearchSubtitles : throw in case of error
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        List<SucceedResultModel> SearchSubtitles(string filename);
    }
}
