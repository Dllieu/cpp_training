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
                    Console.WriteLine("Thread {0}", Thread.CurrentThread.ManagedThreadId);
                }));
                threads.Add(new Thread(delegate()
                {
                    Thread.Sleep(1000 * random.Next(1, 2));
                    Console.WriteLine("Thread {0}", Thread.CurrentThread.ManagedThreadId);
                }));
                threads[threads.Count - 1].Start();
            }

            tasks.ForEach(t => t.Wait());
            threads.ForEach(t => t.Join());
        }

        [Test]
        public static void LazyInstanciation()
        {
            // Initialize the integer to the managed thread id of the 
            // first thread that accesses the Value property.
            var number = new Lazy<int>(() => Thread.CurrentThread.ManagedThreadId);

            var t1 = new Thread(() => Assert.AreEqual(number.Value /*x*/, Thread.CurrentThread.ManagedThreadId /*x*/));
            t1.Start();

            var t2 = new Thread(() => Assert.AreNotEqual(number.Value /*x*/, Thread.CurrentThread.ManagedThreadId /*y*/));
            t2.Start();

            // Ensure that thread IDs are not recycled if the 
            // first thread completes before the last one starts.
            t1.Join();
            t2.Join();
        }
    }
}
