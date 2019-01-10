using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public class NetworkConfigurationCommandPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x10;
        private UInt16 nodeId;
        private UInt16 networkId;

        #endregion

        #region Public Data Fields

        public override byte DataPageNumber
        {
            get { return dataPageNumber; }
        }

        public UInt16 NodeId
        {
            get { return nodeId; }
            set { nodeId = value; OnPropertyChanged("NodeId"); }
        }

        public UInt16 NetworkId
        {
            get { return networkId; }
            set { networkId = value; OnPropertyChanged("NetworkId"); }
        }

        #endregion

        #region Public Methods

        /* Encodes the current values of this page's data fields into the given txBuffer */
        public override void Encode(byte[] txBuffer)
        {
            txBuffer[0] = DataPageNumber;

            txBuffer[1] = BitManipulation.GetByte0(NodeId);
            txBuffer[2] = BitManipulation.ReservedZeros;
            txBuffer[2] = BitManipulation.GetByte1(NodeId);

            txBuffer[3] = BitManipulation.GetByte0(NetworkId);
            txBuffer[4] = BitManipulation.GetByte1(NetworkId);

            txBuffer[5] = BitManipulation.ReservedOnes;
            txBuffer[6] = BitManipulation.ReservedOnes;
            txBuffer[7] = BitManipulation.ReservedOnes;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            BitManipulation.SetByte0(ref nodeId, rxBuffer[1]);
            BitManipulation.SetByte1(ref nodeId, (byte)(rxBuffer[2] & 0x01));

            BitManipulation.SetByte0(ref networkId, rxBuffer[3]);
            BitManipulation.SetByte1(ref networkId, rxBuffer[4]);
        }

        #endregion
    }
}
