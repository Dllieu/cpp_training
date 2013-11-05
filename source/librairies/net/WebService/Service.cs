using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;

namespace WebService
{
    // NOTE: You can use the "Rename" command on the "Refactor" menu to change the class name "Service1" in both code and config file together.
    public class Service : IService
    {
        public string Price(string id)
        {
            return string.Format("You requested: Price on id '{0}'", id);
        }

        public string GetName(CompositeType composite)
        {
            if (composite == null)
                throw new ArgumentNullException("composite");

            return string.Format("Hello {0}", composite.Name);
        }
    }
}
