using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public class PageParser
    {
        private Dictionary<byte, Type> definedDataPages;

        public PageParser()
        {
            definedDataPages = new Dictionary<byte, Type>();
        }

        // Adds the given data page to the list of parsable data pages
        public void AddDataPage(DataPage dataPage)
        {
            definedDataPages[dataPage.DataPageNumber] = dataPage.GetType();
        }

        public dynamic Parse(byte[] rxBuffer)
        {
            dynamic dataPage = null;

            // If the payload has a valid data page number, decode it
            if (definedDataPages.TryGetValue(rxBuffer[0], out Type pageType))
            {
                dataPage = Activator.CreateInstance(pageType);
                dataPage.Decode(rxBuffer);
            }

            return dataPage;
        }
    }
}
