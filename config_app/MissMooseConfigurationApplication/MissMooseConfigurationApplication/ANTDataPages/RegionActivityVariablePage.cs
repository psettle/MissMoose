using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    #region Public Types

    public enum RegionStatus : byte
    {
        NoDetection,
        ProbableDetection,
        DefiniteDetection,
    }

    #endregion

    public class RegionActivityVariablePage : DataPage
    {
        #region Private Members

        private static readonly byte dataPageNumber = 0x23;
        private byte messageId;
        private byte xCoordinate;
        private byte yCoordinate;
        private float activityVariable;
        private RegionStatus regionStatus;

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

        public byte XCoordinate
        {
            get { return xCoordinate; }
            set { xCoordinate = value; OnPropertyChanged("XCoordinate"); }
        }

        public byte YCoordinate
        {
            get { return yCoordinate; }
            set { yCoordinate = value; OnPropertyChanged("YCoordinate"); }
        }

        public float ActivityVariable
        {
            get { return activityVariable; }
            set { activityVariable = value; OnPropertyChanged("ActivityVariable"); }
        }

        public RegionStatus RegionStatus
        {
            get { return regionStatus; }
            set { regionStatus = value; OnPropertyChanged("RegionStatus"); }
        }

        #endregion

        #region Public Methods

        /* Encodes the current values of this page's data fields into the given txBuffer */
        public override void Encode(byte[] txBuffer)
        {
            txBuffer[0] = DataPageNumber;

            txBuffer[1] = MessageId;

            txBuffer[2] |= (byte)(XCoordinate & 0x0F);
            txBuffer[2] |= (byte)((YCoordinate & 0x0F) << 4);

            byte[] activityVariableBytes = BitConverter.GetBytes(ActivityVariable);
            txBuffer[3] = activityVariableBytes[0];
            txBuffer[4] = activityVariableBytes[1];
            txBuffer[5] = activityVariableBytes[2];
            txBuffer[6] = activityVariableBytes[3];

            txBuffer[7] = (byte)RegionStatus;
        }

        /* Decodes the given rxBuffer into this page's data fields */
        public override void Decode(byte[] rxBuffer)
        {
            MessageId = rxBuffer[1];

            XCoordinate = (byte)(rxBuffer[2] & 0x0F);
            YCoordinate = (byte)((rxBuffer[2] & 0xF0) >> 4);

            ActivityVariable = BitConverter.ToSingle(rxBuffer, 3);

            RegionStatus = (RegionStatus)rxBuffer[7];
        }

        #endregion
    }
}
