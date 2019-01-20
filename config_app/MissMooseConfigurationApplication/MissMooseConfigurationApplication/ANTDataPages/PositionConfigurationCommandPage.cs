using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public class PositionConfigurationCommandPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x11;
        private UInt16 nodeId;
        private HardwareConfiguration nodeType;
        private Rotation nodeRotation;

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

        public HardwareConfiguration NodeType
        {
            get { return nodeType; }
            set { nodeType = value; OnPropertyChanged("NodeType"); }
        }

        public Rotation NodeRotation
        {
            get { return nodeRotation; }
            set { nodeRotation = value; OnPropertyChanged("NodeRotation"); }
        }

        public int xpos { get; set; }
        public int ypos { get; set; }
        public int xoffset { get; set; }
        public int yoffset { get; set; }

        #endregion

        #region Public Methods

        /* Encodes the current values of this page's data fields into the given txBuffer */
        public override void Encode(byte[] txBuffer)
        {
            txBuffer[0] = DataPageNumber;

            txBuffer[1] = BitManipulation.GetByte0(NodeId);
            txBuffer[2] = BitManipulation.GetByte1(NodeId);

            txBuffer[3] = (byte)NodeType;

            txBuffer[4] = (byte)NodeRotation.ToEnum();

            byte xpos_encoded = (byte)((xpos - 1) & 0x0F);
            byte ypos_encoded = (byte)((ypos - 1) & 0x0F);
            ypos_encoded <<= 4;

            txBuffer[5] = (byte)(xpos_encoded | ypos_encoded);
            txBuffer[6] = (byte)xoffset;
            txBuffer[7] = (byte)yoffset;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            BitManipulation.SetByte0(ref nodeId, rxBuffer[1]);
            BitManipulation.SetByte1(ref nodeId, (byte)(rxBuffer[2] & 0x01));

            NodeType = (HardwareConfiguration)(rxBuffer[3] & 0x03);

            NodeRotation.FromEnum((byte)(rxBuffer[4] & 0x07));
        }

        #endregion        
    }
}
