using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public class NodeStatusPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x01;
        private UInt16 nodeId;
        private UInt16 networkId;
        private NodeType nodeType;
        private bool isGatewayNode;

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

        public NodeType NodeType
        {
            get { return nodeType; }
            set { nodeType = value; OnPropertyChanged("NodeType"); }
        }

        public bool IsGatewayNode
        {
            get { return isGatewayNode; }
            set { isGatewayNode = value; OnPropertyChanged("IsGatewayNode"); }
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

            txBuffer[5] &= 0xFC;
            txBuffer[5] |= (byte)NodeType;

            txBuffer[5] &= 0x7F;
            txBuffer[5] |= (byte)((IsGatewayNode ? 1 : 0) << 7);

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

            NodeType = (NodeType)(rxBuffer[5] & 0x03);

            IsGatewayNode = (rxBuffer[5] & 0x80) != 0;
        }

        #endregion        
    }
}
