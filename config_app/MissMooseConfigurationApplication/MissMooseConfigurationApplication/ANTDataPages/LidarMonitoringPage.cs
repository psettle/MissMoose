using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public enum LidarRegion
    {
        None,

        Region1,
        Region2,

        Count
    }

    public class LidarMonitoringPage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x21;
        private byte messageId;
        private UInt16 nodeId;
        private Rotation sensorRotation = new Rotation(0);
        private UInt16 distance;
        private LidarRegion region;

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

        public UInt16 Distance
        {
            get { return distance; }
            set { distance = value; OnPropertyChanged("Distance"); }
        }

        public LidarRegion Region
        {
            get { return region; }
            set { region = value; OnPropertyChanged("Region"); }
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

            txBuffer[5] = BitManipulation.GetByte0(Distance);
            txBuffer[6] = BitManipulation.GetByte1(Distance);

            txBuffer[7] = (byte)Region;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            MessageId = rxBuffer[1];

            BitManipulation.SetByte0(ref nodeId, rxBuffer[2]);
            BitManipulation.SetByte1(ref nodeId, (byte)(rxBuffer[3] & 0x01));

            SensorRotation.FromEnum((byte)(rxBuffer[4] & 0x07));

            BitManipulation.SetByte0(ref distance, rxBuffer[5]);
            BitManipulation.SetByte1(ref distance, rxBuffer[6]);

            Region = (LidarRegion)rxBuffer[7];
        }

        #endregion        
    }
}
