using ANT_Managed_Library;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    class ChannelManager
    {
        // Slave channel to search for devices
        public ANT_Channel channel;

        // Device number of the connected master device
        public ushort masterDeviceNumber = 0;

        // Handles sending data pages to nodes being configured
        public PageSender pageSender;

        // ANT Device which opens slave channels to pair to node devices
        private ANT_Device device;
        
        private static readonly int channelNum = 0;

        // Properties for an ANT slave channel which can pair to node devices
        private static readonly byte rfFrequency = 66;          // 2466 MHz
        private static readonly byte deviceType = 1;            // search for node master device (nodes have device type 1)
        private static readonly byte transmissionType = 0;      // wildcard for search
        private static readonly ushort channelPeriod = 8192;    // same channel period as node master devices
        private static readonly ANT_ReferenceLibrary.ChannelType channelType = ANT_ReferenceLibrary.ChannelType.BASE_Slave_Receive_0x00;

        // Device number for node search ANT slave channel
        private static readonly ushort nodeSearchDeviceNumber = 0; // wildcard for search

        private bool deviceRunning = false;
        private bool reopenNodeSearchChannel = false;

        // Default wait time in milliseconds for ANT function calls
        private static readonly ushort waitTime = 500;

        public void StartConfigDevice()
        {
            if (!deviceRunning)
            {
                device = new ANT_Device();
                device.deviceResponse += DeviceResponse;

                // Check whether the connected ANT USB device supports the required capabilities
                ANT_DeviceCapabilities capabilities = device.getDeviceCapabilities(waitTime);
                if (!capabilities.SearchList)
                {
                    Console.WriteLine("Connected ANT USB device does not support exclusion search list");
                    ShutdownConfigDevice();
                }
                else if (!capabilities.ExtendedMessaging)
                {
                    Console.WriteLine("Connected ANT USB device does not support extended messages");
                    ShutdownConfigDevice();
                }
                else
                {
                    try
                    {
                        deviceRunning = true;
                        device.enableRxExtendedMessages(true);

                        // Get an unused ANT channel from the device
                        channel = device.getChannel(channelNum);
                        channel.channelResponse += new dChannelResponseHandler(ChannelResponse);

                        SetupAndOpenChannel(channel, nodeSearchDeviceNumber, true);

                        // Initialize a page sender with the channel
                        pageSender = new PageSender(channel);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("Can't start configuration slave channel: " + e.Message);
                        ShutdownConfigDevice();
                    }
                }
            }
        }

        public void ShutdownConfigDevice()
        {
            //If you need to manually disconnect from a ANTDevice call the shutdownDeviceInstance method,
            //It releases all the resources, the device, and nullifies the object reference
            ANT_Device.shutdownDeviceInstance(ref device);
            deviceRunning = false;
        }

        public void SetupAndOpenChannel(ANT_Channel channel, ushort deviceNumber, bool useExclusionList)
        {
            //We try-catch and forward exceptions to the calling function to handle and pass the errors to the user
            try
            {
                // Assign the channel with the given channel type
                // Use network 0, which has the public network key by default
                if (channel.assignChannel(channelType, 0, waitTime))
                {
                    Console.WriteLine("Ch assigned to " + channelType + " on net 0.");
                }
                else
                {
                    throw new Exception("Channel assignment operation failed.");
                }

                // Set the channel ID
                if (channel.setChannelID(deviceNumber, false, deviceType, transmissionType, waitTime))
                {
                    Console.WriteLine("Set channel ID to " + deviceNumber + ", " + deviceType + ", " + transmissionType);
                }
                else
                {
                    throw new Exception("Set channel ID operation failed.");
                }

                if (useExclusionList)
                {
                    if (masterDeviceNumber == 0)
                    {
                        // Configure exclusion list to be empty if we haven't yet paired to a node device
                        channel.includeExcludeList_Configure(0, true);
                    }
                    else
                    {
                        // Put the previously paired node's device number in the exclusion list so that we don't just re-pair to it immediately
                        channel.includeExcludeList_addChannel(masterDeviceNumber, 0, 0, 0);

                        // Configure exclusion list to include 1 channel
                        channel.includeExcludeList_Configure(1, true);
                    }
                }

                // Set the RF frequency for the channel
                if (channel.setChannelFreq(rfFrequency, waitTime))
                {
                    Console.WriteLine("RF frequency set to " + (rfFrequency + 2400));
                }

                // Set the channel period to match the rate at which the nodes broadcast
                if (channel.setChannelPeriod(channelPeriod, waitTime))
                {
                    Console.WriteLine("Message rate set to 4Hz");
                }

                // Open the channel
                if (channel.openChannel(waitTime))
                {
                    Console.WriteLine("Opened channel");
                }
                else
                {
                    throw new Exception("Channel open operation failed.");
                }
            }
            catch (Exception ex)
            {
                throw new Exception("Setup and open failed. " + ex.Message + Environment.NewLine);
            }
        }

        public void ReOpenChannel()
        {
            // Clear exclusion list
             channel.includeExcludeList_Configure(0, true);

            // Unassign the channel
            if (channel.unassignChannel(waitTime))
            {
                Console.WriteLine("Unassigned channel");
            }
            else
            {
                throw new Exception("Channel unassign operation failed.");
            }

            // Reassign and open the channel, excluding the current master device number from the search
            // so we don't just pair to it again
            SetupAndOpenChannel(channel, nodeSearchDeviceNumber, true);
        }

        public void CloseChannel()
        {
            if (channel.closeChannel(waitTime))
            {
                Console.WriteLine("Closed channel");
            }
            else
            {
                throw new Exception("Channel close operation failed.");
            }
        }

        /*
         * Handles the ANT device response
         */
        public void DeviceResponse(ANT_Response response)
        {
            switch ((ANT_ReferenceLibrary.ANTMessageID)response.responseID)
            {
                case ANT_ReferenceLibrary.ANTMessageID.RX_EXT_MESGS_ENABLE_0x66:
                    if (response.getChannelEventCode() == ANT_ReferenceLibrary.ANTEventID.INVALID_MESSAGE_0x28)
                    {
                        Console.WriteLine("Extended messages not supported in this ANT product");
                        break;
                    }
                    else if (response.getChannelEventCode() != ANT_ReferenceLibrary.ANTEventID.RESPONSE_NO_ERROR_0x00)
                    {
                        Console.WriteLine(String.Format("Error {0} configuring {1}", response.getChannelEventCode(), response.getMessageID()));
                        break;
                    }
                    Console.WriteLine("Extended messages enabled");
                    break;
                case ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40:
                    switch (response.getMessageID())
                    {
                        case ANT_ReferenceLibrary.ANTMessageID.CLOSE_CHANNEL_0x4C:

                            // Clear exclusion list
                            channel.includeExcludeList_Configure(0, true);

                            // Unassign the channel
                            if (channel.unassignChannel(waitTime))
                            {
                                Console.WriteLine("Unassigned channel");
                            }
                            else
                            {
                                throw new Exception("Channel unassign operation failed.");
                            }

                            // Reassign and open the channel, excluding the current master device number from the search
                            // so we don't just pair to it again
                            SetupAndOpenChannel(channel, nodeSearchDeviceNumber, true);

                            break;
                    }
                    break;
            }
        }

        private void ChannelResponse(ANT_Response response)
        {
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
            {
                switch (response.getChannelEventCode())
                {
                    case ANT_ReferenceLibrary.ANTEventID.EVENT_RX_SEARCH_TIMEOUT_0x01:
                        reopenNodeSearchChannel = true;
                        break;
                    case ANT_ReferenceLibrary.ANTEventID.EVENT_CHANNEL_CLOSED_0x07:
                        if (reopenNodeSearchChannel)
                        {
                            reopenNodeSearchChannel = false;
                            ReOpenChannel();
                        }
                        break;
                }
            }
        }
    }
}
