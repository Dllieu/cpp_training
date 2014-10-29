using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MediaWatcher
{
    public class FileWatcher
    {
        private readonly FileSystemWatcher _watcher;

        public FileWatcher()
        {
            _watcher = new FileSystemWatcher
            {
                Path = string.Format(@"C:\Users\{0}\Music", Environment.UserName)
            };


            _watcher.EnableRaisingEvents = true;
        }

        private void OnUpdate()
        {
        }
    }
}
