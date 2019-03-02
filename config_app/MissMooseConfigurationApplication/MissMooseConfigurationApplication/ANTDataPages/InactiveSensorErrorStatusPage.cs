using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public class InactiveSensorErrorStatusPage : BaseErrorStatusPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x26;

        #endregion

        #region Public Data Fields

        public override byte DataPageNumber
        {
            get { return dataPageNumber; }
        }

        #endregion
    }
}
