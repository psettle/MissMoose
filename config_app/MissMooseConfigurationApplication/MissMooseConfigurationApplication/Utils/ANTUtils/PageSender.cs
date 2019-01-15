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

        private bool expectingAck = false;
        private byte retryCount = 0;
        private byte maxRetries;
        private byte[] acknowledgedMessage;

        #endregion

        #region Public Methods

        public PageSender(ANT_Channel channel)
        {
            this.channel = channel;

            this.channel.channelResponse += new dChannelResponseHandler(channelResponse);
        }

        private void channelResponse(ANT_Response response)
        {
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
            {
                switch (response.getChannelEventCode())
                {
                    case ANT_ReferenceLibrary.ANTEventID.EVENT_TRANSFER_TX_COMPLETED_0x05:
                        retryCount = 0;
                        expectingAck = false;
                        break;
                    case ANT_ReferenceLibrary.ANTEventID.EVENT_TRANSFER_TX_FAILED_0x06:
                        if (expectingAck)
                        {
                            if (retryCount < maxRetries)
                            {
                                channel.sendAcknowledgedData(acknowledgedMessage);
                                retryCount++;
                            }
                            else
                            {
                                expectingAck = false;
                            }
                        }
                        break;
                }
            }
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
                    // Save this message buffer to send again if the ack fails
                    acknowledgedMessage = txBuffer;

                    this.maxRetries = maxRetries;
                    retryCount = 0;
                }

                channel.sendAcknowledgedData(txBuffer);
            }
        }

        #endregion
    }
}
