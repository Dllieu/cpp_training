namespace TestSuiteCSharp
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Threading;
    using NUnit.Framework;

    [TestFixture]
    public class FrameworkTestSuite
    {
        delegate void CustomDelegate(string s);

        public static void exampleImplemDelegate(string s)
        {
            Console.WriteLine("exampleImplemDelegate {0}", s);
        }

        [Test]
        public void Delegate()
        {
            CustomDelegate c1, c2, c12, c1bis;

            c1 = new CustomDelegate( exampleImplemDelegate );
            c2 = new CustomDelegate(delegate(string s) { Console.WriteLine("realDelegate {0}", s); });

            c12 = c1 + c2;
            c1bis = c12 - c2;

            string str = "rofl";
            c1(str);
            c2(str);
            c12(str);
            c1bis(str);
        }

        [Test]
        public void WebService()
        {
            ServiceReference1.ServiceClient client = new ServiceReference1.ServiceClient();
            ServiceReference1.CompositeType composite = new ServiceReference1.CompositeType();

            composite.Name = "John";
            string result = client.GetName(composite);
            Assert.AreEqual(string.Format("Hello {0}", composite.Name), result);
        }

        [Test]
        public void Threading()
        {
            List<Thread> threads = new List<Thread>();
            List<Task> tasks = new List<Task>();
            Random random = new Random();
            foreach (int i in Enumerable.Range(1, 7))
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
                threads[ threads.Count - 1 ].Start();
            }

            tasks.ForEach(t => t.Wait());
            threads.ForEach(t => t.Join());
        }

        public class Pair<T1, T2>
        {
            public T1 First { get; set; }
            public T2 Second { get; set; }
        }

        void initPair(ref Pair<int, string> pair)
        {
            pair = new Pair<int, string>();
            pair.First = 65;
            pair.Second = "hello";
        }

        [Test]
        public void Container()
        {
            Pair<int, string> pair = null;
            initPair(ref pair);
            Assert.AreEqual(pair.Second, "hello");
        }
    }
}
