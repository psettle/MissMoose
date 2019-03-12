using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MissMooseConfigurationApplication.UIComponents;

namespace MissMooseConfigurationApplication
{
    public abstract class BaseErrorStatusPage : DataPage
    {
        #region Protected Members

        protected byte messageId;
        protected UInt16 nodeId;
        protected SensorType sensorType;
        protected Rotation sensorRotation = new Rotation(0);
        protected bool errorOccurring;

        #endregion

        #region Public Data Fields

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

        public SensorType SensorType
        {
            get { return sensorType; }
            set { sensorType = value; OnPropertyChanged("SensorType"); }
        }

        public Rotation SensorRotation
        {
            get { return sensorRotation; }
            set { sensorRotation = value; OnPropertyChanged("SensorRotation"); }
        }

        public bool ErrorOccurring
        {
            get { return errorOccurring; }
            set { errorOccurring = value; OnPropertyChanged("ErrorOccurring"); }
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

            txBuffer[4] = (byte)SensorType;

            txBuffer[5] = (byte)SensorRotation.ToEnum();

            BitManipulation.SetBit(ref txBuffer[6], 0, ErrorOccurring);

            txBuffer[7] = BitManipulation.ReservedOnes;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            MessageId = rxBuffer[1];

            BitManipulation.SetByte0(ref nodeId, rxBuffer[2]);
            BitManipulation.SetByte1(ref nodeId, (byte)(rxBuffer[3] & 0x01));

            SensorType = (SensorType)rxBuffer[4];

            SensorRotation.FromEnum((byte)(rxBuffer[5] & 0x07));

            ErrorOccurring = BitManipulation.GetBit(rxBuffer[6], 0);
        }

            #endregion
        }
}
