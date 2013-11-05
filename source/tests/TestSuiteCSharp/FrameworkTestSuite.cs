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
            Random random = new Random();
            foreach (int i in Enumerable.Range(1, 7))
            {
                threads.Add(new Thread(delegate()
                                       {
                                            Thread.Sleep(1000 * random.Next(1, 2));
                                            Console.WriteLine("Thread {0}", i);
                                       }));
                threads[i].Start();
            }

            foreach (int i in Enumerable.Range(1, 7))
                threads[i].Join();
        }
    }
}
