using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MissMooseConfigurationApplication.UIComponents;

namespace MissMooseConfigurationApplication
{
    public class PirMonitoringPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x22;
        private byte messageId;
        private UInt16 nodeId;
        private Rotation sensorRotation = new Rotation(0);
        private bool detection;

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

        public Rotation SensorRotation
        {
            get { return sensorRotation; }
            set { sensorRotation = value; OnPropertyChanged("SensorRotation"); }
        }

        public bool Detection
        {
            get { return detection; }
            set { detection = value; OnPropertyChanged("Detection"); }
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

            txBuffer[4] = (byte)SensorRotation.ToEnum();

            BitManipulation.SetBit(ref txBuffer[5], 0, detection);

            txBuffer[6] = BitManipulation.ReservedZeros;
            txBuffer[7] = BitManipulation.ReservedZeros;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            MessageId = rxBuffer[1];

            BitManipulation.SetByte0(ref nodeId, rxBuffer[2]);
            BitManipulation.SetByte1(ref nodeId, (byte)(rxBuffer[3] & 0x01));

            SensorRotation.FromEnum((byte)(rxBuffer[4] & 0x07));

            Detection = BitManipulation.GetBit(rxBuffer[5], 0);
        }

        #endregion        
    }
}
