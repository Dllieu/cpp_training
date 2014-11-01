using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Threading;
using NUnit.Framework;

namespace TestSuiteCSharp
{
    [TestFixture]
    class ThreadingTestSuite
    {
        [Test]
        public static void Threading()
        {
            var threads = new List<Thread>();
            var tasks = new List<Task>();
            var random = new Random();
            foreach (var i in Enumerable.Range(1, 7))
            {
                tasks.Add(Task.Run(delegate()
                {
                    Thread.Sleep(1000 * random.Next(1, 2));
                    Console.WriteLine("Thread {0}", i);
                }));
                threads.Add(new Thread(delegate()
                {
                    Thread.Sleep(1000 * random.Next(1, 2));
                    Console.WriteLine("Thread {0}", i);
                }));
                threads[threads.Count - 1].Start();
            }

            tasks.ForEach(t => t.Wait());
            threads.ForEach(t => t.Join());
        }
    }
}
