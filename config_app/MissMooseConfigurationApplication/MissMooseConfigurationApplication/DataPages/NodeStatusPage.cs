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
        private ConfigurationStatus configStatus = ConfigurationStatus.Unconfigured;

        #endregion

        #region Public Data Fields

        public override byte DataPageNumber
        {
            get { return dataPageNumber; }
        }

        public ConfigurationStatus ConfigStatus
        {
            get { return configStatus; }
            set { configStatus = value; OnPropertyChanged("ConfigStatus"); }
        }
        #endregion

        #region Public Methods

        /* Encodes the current values of this page's data fields into the given txBuffer */
        public override void Encode(byte[] txBuffer)
        {
            txBuffer[0] = DataPageNumber;

            txBuffer[1] = BitManipulation.ReservedZeros;
            txBuffer[1] = (byte)ConfigStatus;

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
            ConfigStatus = (ConfigurationStatus)(BitManipulation.GetBit(rxBuffer[1], 0) ? 1 : 0);
        }

        #endregion

        #region Types

        public enum ConfigurationStatus : byte
        {
            Unconfigured = 0,
            Configured = 1,
        }

        #endregion
    }
}
