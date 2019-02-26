using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public class ErrorStatusAckPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x27;
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

            txBuffer[2] = BitManipulation.ReservedOnes;
            txBuffer[3] = BitManipulation.ReservedOnes;
            txBuffer[4] = BitManipulation.ReservedOnes;
            txBuffer[5] = BitManipulation.ReservedOnes;
            txBuffer[6] = BitManipulation.ReservedOnes;
            txBuffer[7] = BitManipulation.ReservedOnes;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            MessageId = rxBuffer[1];
        }

        #endregion        
    }
}
