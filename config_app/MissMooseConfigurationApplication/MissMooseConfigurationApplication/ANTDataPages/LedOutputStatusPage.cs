using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MissMooseConfigurationApplication.UIComponents;

namespace MissMooseConfigurationApplication
{
    public class LedOutputStatusPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x24;
        private byte messageId;
        private UInt16 nodeId;
        private LedFunction ledFunction;
        private LedColour ledColour;

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

        public UInt16 NodeId
        {
            get { return nodeId; }
            set { nodeId = value; OnPropertyChanged("NodeId"); }
        }

        public LedFunction LedFunction
        {
            get { return ledFunction; }
            set { ledFunction = value; OnPropertyChanged("LedFunction"); }
        }

        public LedColour LedColour
        {
            get { return ledColour; }
            set { ledColour = value; OnPropertyChanged("LedColour"); }
        }

        #endregion

        #region Public Methods

        /* Encodes the current values of this page's data fields into the given txBuffer */
        public override void Encode(byte[] txBuffer)
        {
            txBuffer[0] = DataPageNumber;

            txBuffer[1] = MessageId;

            txBuffer[2] = BitManipulation.GetByte0(NodeId);
            txBuffer[3] = BitManipulation.GetByte1(NodeId);

            txBuffer[4] = (byte)LedFunction;

            txBuffer[5] = (byte)LedColour;

            txBuffer[6] = BitManipulation.ReservedOnes;
            txBuffer[7] = BitManipulation.ReservedOnes;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            MessageId = rxBuffer[1];

            BitManipulation.SetByte0(ref nodeId, rxBuffer[2]);
            BitManipulation.SetByte1(ref nodeId, (byte)(rxBuffer[3] & 0x01));

            LedFunction = (LedFunction)(rxBuffer[4] & 0x03);

            LedColour = (LedColour)(rxBuffer[5] & 0x01);
        }

            #endregion
        }
}
