using NUnit.Framework;

namespace TestSuiteCSharp
{
    [TestFixture]
    class CoroutineTestSuite
    {
        public static System.Collections.IEnumerable Power(int number, int exponent)
        {
            var result = 1;
            for (var i = 0; i < exponent; i++)
            {
                result = result * number;
                yield return result;
            }
        }

        [Test]
        public static void BasicYieldUsage()
        {
            var result = 1;
            foreach (int i in Power(2, 8))
                Assert.AreEqual((result *= 2), i);
        }
    }
}
