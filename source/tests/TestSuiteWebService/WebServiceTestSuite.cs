namespace TestSuiteCSharp
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using NUnit.Framework;

    [TestFixture]
    public static class WebServiceTestSuite
    {
        [Test]
        public static void WebService()
        {
            var client = new ServiceReference1.ServiceClient();
            var composite = new ServiceReference1.CompositeType { Name = "John" };

            var result = client.GetName(composite);
            Assert.AreEqual(string.Format("Hello {0}", composite.Name), result);
        }
    }
}
