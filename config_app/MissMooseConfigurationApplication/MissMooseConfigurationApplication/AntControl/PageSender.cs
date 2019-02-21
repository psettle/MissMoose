﻿using System;
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
        private ANT_ChannelID channelId;

        #endregion

        #region Public Methods

        public PageSender(ANT_Channel channel, ANT_ChannelID channelId)
        {
            this.channel = channel;
            this.channelId = channelId;
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
                channel.sendExtBroadcastData(
                    channelId.deviceNumber, channelId.deviceTypeID, channelId.transmissionTypeID, 
                    txBuffer);
            }            
        }

        public bool SendAcknowledged(DataPage page, bool retry, byte maxTries=1)
        {
            if (page.DataPageLength != 8)
            {
                Console.WriteLine("Acknowledged messages must be 8 bytes long");
                return false;
            }
            else
            {
                byte[] txBuffer = new byte[8];
                page.Encode(txBuffer);

                ANT_ReferenceLibrary.MessagingReturnCode returnCode = ANT_ReferenceLibrary.MessagingReturnCode.Fail;
                byte retries = (byte) (retry ? maxTries : 1);

                while (returnCode != ANT_ReferenceLibrary.MessagingReturnCode.Pass && retries > 0)
                {
                    retries--;
                    returnCode = channel.sendExtAcknowledgedData(
                        channelId.deviceNumber, channelId.deviceTypeID, channelId.transmissionTypeID,
                        txBuffer, 500);
                }

                // If the message transmission has passed, return true.
                // Otherwise, return false.
                return returnCode == ANT_ReferenceLibrary.MessagingReturnCode.Pass ? true : false;
            }
        }

        #endregion
    }
}
