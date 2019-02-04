using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public class MonitoringDataAckPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x20;
        private byte messageId;

        #endregion

        #region Public Data Fields

        public override byte DataPageNumber
        {
            get { return dataPageNumber; }
        }

        public byte MessageId
        {
            get { return messageId; }
            set { messageId = value; OnPropertyChanged("MessageId"); }
        }

        #endregion

        #region Public Methods

        /* Encodes the current values of this page's data fields into the given txBuffer */
        public override void Encode(byte[] txBuffer)
        {
            txBuffer[0] = DataPageNumber;

            txBuffer[1] = MessageId;

            txBuffer[2] = BitManipulation.ReservedZeros;
            txBuffer[3] = BitManipulation.ReservedZeros;
            txBuffer[4] = BitManipulation.ReservedZeros;
            txBuffer[5] = BitManipulation.ReservedZeros;
            txBuffer[6] = BitManipulation.ReservedZeros;
            txBuffer[7] = BitManipulation.ReservedZeros;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            MessageId = rxBuffer[1];
        }

        #endregion        
    }
}
