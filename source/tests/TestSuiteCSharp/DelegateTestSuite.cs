using System;
using NUnit.Framework;

namespace TestSuiteCSharp
{
    [TestFixture]
    class DelegateTestSuite
    {
        delegate void CustomDelegate(string s);

        public static void ExampleImplemDelegate(string s)
        {
            Console.WriteLine("ExampleImplemDelegate {0}", s);
        }

        [Test]
        public static void Delegate()
        {
            var c1 = new CustomDelegate(ExampleImplemDelegate);
            var c2 = new CustomDelegate(s => Console.WriteLine("realDelegate {0}", s));
            var c12 = c1 + c2;
            var c1Bis = c12 - c2;

            const string str = "rofl";
            c1(str);
            c2(str);
            c12(str);
            c1Bis(str);
        }
    }
}
