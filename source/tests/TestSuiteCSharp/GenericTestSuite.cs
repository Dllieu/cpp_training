using NUnit.Framework;

namespace TestSuiteCSharp
{
    [TestFixture]
    class GenericTestSuite
    {
        public class Pair<T1, T2>
        {
            public T1 First { get; set; }
            public T2 Second { get; set; }
        }

        public static void InitPair(out Pair<int, string> pair)
        {
            pair = new Pair<int, string> { First = 65, Second = "Hello" };
        }

        [Test]
        public static void Container()
        {
            Pair<int, string> pair;
            InitPair(out pair);
            Assert.AreEqual(pair.Second, "Hello");
        }
    }
}
