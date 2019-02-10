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
        // Slave channel to scan for devices
        public ANT_Channel channel;

        // Channels which act as data buffers for sending to nodes
        // Dictionary keys are node device numbers
        private Dictionary<ushort, ANT_Channel> responseChannels
            = new Dictionary<ushort, ANT_Channel>();

        // Keeps track of the next channel to try to assign
        private byte nextChannelNum = 1;
        private static byte minChannelNum = 1;
        private static byte maxChannelNum = 7;

        // ANT Device which opens slave channel to scan for node devices
        private ANT_Device device;

        // Properties for ANT slave channel to scan for node devices
        private static readonly int channelNum = 0;             // Must be 0 for continuous scan
        private static readonly byte rfFrequency = 66;          // 2466 MHz
        private static readonly byte deviceType = 1;            // search for node master device (nodes have device type 1)
        private static readonly byte transmissionType = 0;      // wildcard for search
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

                if (!capabilities.ExtendedMessaging)
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

                        SetupAndOpenChannel(channel);
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

        public void SetupAndOpenChannel(ANT_Channel channel)
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
                if (channel.setChannelID(nodeSearchDeviceNumber, false, deviceType, transmissionType, waitTime))
                {
                    Console.WriteLine("Set channel ID to " + nodeSearchDeviceNumber + ", " + deviceType + ", " + transmissionType);
                }
                else
                {
                    throw new Exception("Set channel ID operation failed.");
                }

                // Set the RF frequency for the channel
                if (channel.setChannelFreq(rfFrequency, waitTime))
                {
                    Console.WriteLine("RF frequency set to " + (rfFrequency + 2400));
                }

                // Open the channel
                if (device.openRxScanMode(waitTime))
                {
                    Console.WriteLine("Opened channel in scan mode");
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
            // Unassign the channel
            if (channel.unassignChannel(waitTime))
            {
                Console.WriteLine("Unassigned channel");
            }
            else
            {
                throw new Exception("Channel unassign operation failed.");
            }

            // Reassign and open the channel
            SetupAndOpenChannel(channel);
        }

        public void CloseChannel()
        {
            ANT_ChannelStatus status = channel.requestStatus(waitTime);

            // Only attempt to close the channel if it is currently open (searching or tracking)
            if (status.BasicStatus == ANT_ReferenceLibrary.BasicChannelStatusCode.SEARCHING_0x2
                || status.BasicStatus == ANT_ReferenceLibrary.BasicChannelStatusCode.TRACKING_0x3)
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
        }

        /*
         * Provides a channel to communicate with the node with the given device number
         */ 
        public ANT_Channel GetChannel(ushort deviceNumber)
        {
            ANT_Channel channel = null;
            responseChannels.TryGetValue(deviceNumber, out channel);

            // If there is already a channel assigned with this device number, return it
            if (channel != null)
            {
                return channel;
            }
            // Otherwise, assign a new channel with this device number
            else
            {
                channel = device.getChannel(nextChannelNum);
                nextChannelNum = nextChannelNum < maxChannelNum ? (byte)(nextChannelNum + 1) : minChannelNum;

                if (channel.assignChannel(channelType, 0, waitTime))
                {
                    Console.WriteLine("Response channel assigned");
                }
                else
                {
                    throw new Exception("Response channel assignment operation failed.");
                }

                if (channel.setChannelID(deviceNumber, false, deviceType, transmissionType, waitTime))
                {
                    Console.WriteLine("Set response channel ID to " + deviceNumber + ", " + deviceType + ", " + transmissionType);
                }
                else
                {
                    throw new Exception("Set response channel ID operation failed.");
                }

                responseChannels.Add(deviceNumber, channel);

                return channel;
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
            }
        }
    }    
}
