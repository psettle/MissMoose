using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ANT_Managed_Library;

namespace MissMooseConfigurationApplication
{
    public class PageSender
    {
        #region Private Members

        private ANT_Channel channel;

        #endregion

        #region Public Methods

        public PageSender(ANT_Channel channel)
        {
            this.channel = channel;
        }        

        public void SendBroadcast(DataPage page)
        {
            if (page.DataPageLength != 8)
            {
                Console.WriteLine("Broadcast messages must be 8 bytes long");
            }
            else
            {
                byte[] txBuffer = new byte[8];
                page.Encode(txBuffer);
                channel.sendBroadcastData(txBuffer);
            }            
        }

        public void SendAcknowledged(DataPage page, bool retry, byte maxRetries=1)
        {
            if (page.DataPageLength != 8)
            {
                Console.WriteLine("Acknowledged messages must be 8 bytes long");
            }
            else
            {
                byte[] txBuffer = new byte[8];
                page.Encode(txBuffer);

                if (retry)
                {
                    ANT_ReferenceLibrary.MessagingReturnCode returnCode = ANT_ReferenceLibrary.MessagingReturnCode.Fail;
                    byte retries = maxRetries;

                    while (returnCode != ANT_ReferenceLibrary.MessagingReturnCode.Pass && retries > 0)
                    {
                        returnCode = channel.sendAcknowledgedData(txBuffer, 500);
                    }
                }
                else
                {
                    channel.sendAcknowledgedData(txBuffer);
                }               
            }
        }

        #endregion
    }
}
