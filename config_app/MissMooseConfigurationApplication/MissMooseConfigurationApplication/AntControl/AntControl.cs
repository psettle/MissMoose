using ANT_Managed_Library;
using MissMooseConfigurationApplication.UIPages;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Windows;

namespace MissMooseConfigurationApplication
{
    class AntControl
    {
        #region Public Members
		
        public static AntControl Instance = new AntControl();
		
        #endregion
		
        #region Private Members

        private ConfigurationPage ConfigUI;
        private SystemOverviewPage MonitoringUI;

        // ANT Device which opens a slave channel to search for node devices
        ANT_Device device;        
		
        // Slave channel to search for node devices
        ANT_Channel channel;

        // Parser to decode any valid received data pages
        PageParser pageParser;

        // Handles sending data pages
        PageSender pageSender;

        // Properties for the ANT slave channel
        static readonly byte rfFrequency = 66;  // 2466 MHz
        byte transmissionType = 0;              // wildcard for search
        static readonly byte deviceType = 1;    // search for node master device (nodes have device type 1)
        UInt16 deviceNumber = 0;                // wildcard for search
        static readonly UInt16 channelPeriod = 8192; // same channel period as node master devices
        static readonly ANT_ReferenceLibrary.ChannelType channelType = ANT_ReferenceLibrary.ChannelType.BASE_Slave_Receive_0x00;

        // BLAZE node config properties
        static readonly UInt16 maxNodeId = 512;
        static UInt16 nextNodeId = 1;
        static readonly UInt16 networkId = 20000;

        // Device number of the connected master device
        ushort masterDeviceNumber = 0;

        // Device numbers of all discovered nodes
        List<ushort> nodeDeviceNumbers;

        // Device numbers of all discovered gateway devices, using network number as dictionary keys
        Dictionary<ushort, ushort> gatewayDeviceNumbers;

        // Node configuration data waiting to be sent to a gateway node, using node ID as dictionary keys
        Dictionary<ushort, NodeConfigurationData> nodeConfigList;

        // Booleans to keep track of state
        bool deviceRunning = false;
        bool sendConfiguration = false;

        // Default wait time in milliseconds for ANT function calls
        static readonly ushort waitTime = 500;

        #endregion

        #region Public Methods

        public AntControl()
        {
            // Set up the page parser to decode valid data pages
            pageParser = new PageParser();
            pageParser.AddDataPage(new NodeStatusPage());

            // Create node data lists
            nodeDeviceNumbers = new List<ushort>();
            gatewayDeviceNumbers = new Dictionary<ushort, ushort>();
            nodeConfigList = new Dictionary<ushort, NodeConfigurationData>();            
        }

        public void AddConfigUI(ConfigurationPage ConfigUI)
        {
            this.ConfigUI = ConfigUI;

            // Open the ANT slave channel to search for a node
            startConfigDevice();

            //demo add existing node
            //var node = new SensorNode(HardwareConfiguration.PirLidarLed, 2);
            //node.xpos = 1;
            //node.ypos = 1;
            //node.Rotation = new NodeRotation(NodeRotation.R90);
            //node.xoffset = -3;
            //node.yoffset = 3;

            //ConfigUI.AddExistingNode(node);

            //demo add new node
            //ConfigUI.AddNewNode(new SensorNode(HardwareConfiguration.Pir2, 3));


            //demo read input state
            //var nodes = ConfigUI.GetCurrentNodes();
        }
		
        public void AddMonitoringUI(SystemOverviewPage MonitoringUI)
        {
            this.MonitoringUI = MonitoringUI;

            //demo: add existing node
            var node = new SensorNode(HardwareConfiguration.PirLidarLed, 2, true);
            node.xpos = 1;
            node.ypos = 1;
            node.Rotation = new NodeRotation(NodeRotation.R90);
            node.xoffset = -3;
            node.yoffset = 3;

            MonitoringUI.AddNode(node);

            //demo: remove, modify, add node
            MonitoringUI.RemoveNode(node);
            node.xoffset = 1;
            node.yoffset = 1;
            node.Rotation = new NodeRotation(NodeRotation.R180);
            MonitoringUI.AddNode(node);

            //demo: Display sensor data
            MonitoringUI.MarkSensorDetection(node, LineDirection.Down, StatusColour.Red);

            //demo: add existing node
            node = new SensorNode(HardwareConfiguration.Pir2, 3, false);
            node.xpos = 1;
            node.ypos = 2;
            node.Rotation = new NodeRotation(NodeRotation.R90);
            node.xoffset = -3;
            node.yoffset = 3;

            MonitoringUI.AddNode(node);

            //demo: remove, modify, add node
            MonitoringUI.RemoveNode(node);
            node.xoffset = 0;
            node.yoffset = 0;
            node.Rotation = new NodeRotation(NodeRotation.R0);
            MonitoringUI.AddNode(node);

            //demo: Display sensor data
            MonitoringUI.MarkSensorDetection(node, LineDirection.Up, StatusColour.Red);
            MonitoringUI.MarkSensorDetection(node, LineDirection.Left, StatusColour.Yellow);
        }

        #endregion

        #region Private Methods

        private void startConfigDevice()
        {
            if (!deviceRunning)
            {
                if (startSlaveChannel())
                {
                    deviceRunning = true;
                    return;
                }
            }
            //If we get here it means we failed startup or are stopping node configuration
            shutdownConfigDevice();
        }

        private bool startSlaveChannel()
        {
            try
            {
                device = new ANT_Device();
                device.deviceResponse += new ANT_Device.dDeviceResponseHandler(deviceResponse);
                device.getChannel(0).channelResponse += new dChannelResponseHandler(channelResponse);

                // Check whether the connected ANT USB device supports the required capabilities
                ANT_DeviceCapabilities capabilities = device.getDeviceCapabilities(waitTime);
                if (!capabilities.SearchList)
                {
                    Console.WriteLine("Connected ANT USB device does not support exclusion search list");
                    shutdownConfigDevice();
                }
                else if (!capabilities.ExtendedMessaging)
                {
                    Console.WriteLine("Connected ANT USB device does not support extended messages");
                    shutdownConfigDevice();
                }
                else
                {
                    device.enableRxExtendedMessages(true);

                    // Get an unused ANT channel from the device
                    channel = device.getChannel(0);

                    // Initialize a page sender with the channel
                    pageSender = new PageSender(channel);

                    setupAndOpenChannel(channelType);
                }                
            }
            catch (Exception e)
            {
                Console.WriteLine("Can't start configuration slave channel: " + e.Message);
                return false;
            }
            return true;
        }

        private void shutdownConfigDevice()
        {
            //If you need to manually disconnect from a ANTDevice call the shutdownDeviceInstance method,
            //It releases all the resources, the device, and nullifies the object reference
            ANT_Device.shutdownDeviceInstance(ref device);
            deviceRunning = false;
        }

        private void setupAndOpenChannel(ANT_ReferenceLibrary.ChannelType channelType)
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

        /*
         * Handles the ANT device response
         */
        private void deviceResponse(ANT_Response response)
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
                            setupAndOpenChannel(channelType);

                            break;
                    }
                    break;                    
            }
        }

        /*
         * Handles the ANT channel response
         */
        private void channelResponse(ANT_Response response)
        {
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
            {
                // If a search timeout occurs, reopen the channel
                if (response.getChannelEventCode() == ANT_ReferenceLibrary.ANTEventID.EVENT_RX_SEARCH_TIMEOUT_0x01)
                {
                    channel.openChannel();
                }
            }
            //This is a receive event, so handle the received ANT message
            else
            {
                // Check if this is an extended message
                if (response.isExtended())
                {
                    // Save device number of the connected master device
                    ANT_ChannelID channelId = response.getDeviceIDfromExt();
                    masterDeviceNumber = channelId.deviceNumber;
                }

                byte[] rxBuffer = response.getDataPayload();

                dynamic dataPage = pageParser.Parse(rxBuffer);

                if (dataPage != null)
                {
                    handlePage(dataPage);
                }
            }
        }        

        /*
         * Processes a received Node Status data page 
         */
        private void handlePage(NodeStatusPage dataPage)
        {
            // If the node does not have a node ID, send a Network Configuration Command page
            if (dataPage.NodeId == 0)
            {
                sendNetworkConfiguration();
                sendConfiguration = true;                
            }
            // If the node has a node ID and we just configured it, increment the next node ID to be assigned
            else if (sendConfiguration)
            {
                sendConfiguration = false;

                if (nextNodeId < maxNodeId) nextNodeId++;
                else Console.WriteLine("Maximum node ID (512) reached. Cannot configure another node.");
            }

            // If this is a gateway node, save its device number and network ID
            // so that we can communicate with it later.
            // Also send any waiting node configuration data to the gateway.
            if (dataPage.IsGatewayNode)
            {
                gatewayDeviceNumbers[dataPage.NetworkId] = masterDeviceNumber;

                // If there is any node configuration data waiting to be sent to the gateway, send it
                sendNodeConfigurationToGateway();                
            }
            // If this is not a gateway node, save the node ID and node type to send to the gateway
            else if (dataPage.NodeId != 0 && dataPage.NodeType != HardwareConfiguration.Unknown)
            {
                addToNodeConfigList(dataPage.NodeId, new NodeConfigurationData(dataPage.NodeType, new NodeRotation(NodeRotation.R0)));
            }

            if (!nodeDeviceNumbers.Contains(masterDeviceNumber)
                && dataPage.NodeType != HardwareConfiguration.Unknown
                && dataPage.NodeId != 0)
            {
                // Add a new node to the UI for the user
                Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate {

                    ConfigUI.AddNewNode(new SensorNode(dataPage.NodeType, dataPage.NodeId, dataPage.IsGatewayNode));
                });

                // Save this node's device number
                nodeDeviceNumbers.Add(masterDeviceNumber);

                // Close the channel
                if (channel.closeChannel(waitTime))
                {
                    Console.WriteLine("Closed channel");
                }
                else
                {
                    throw new Exception("Channel close operation failed.");
                }
                
                // We will reopen the channel once we receive the channel closed event
            }
        }        

        private void sendNetworkConfiguration()
        {
            NetworkConfigurationCommandPage configPage = new NetworkConfigurationCommandPage();

            configPage.NodeId = nextNodeId;
            configPage.NetworkId = networkId;

            pageSender.SendAcknowledged(configPage, false);
        }

        private void sendNodeConfigurationToGateway()
        {
            // TODO: add a save button to the UI that triggers updating the node configuration data list.
            // For now, poll the configuration page's sensor node list for updates every time we send data to the gateway
            updateNodeConfigList();

            KeyValuePair<ushort, NodeConfigurationData> dataToSend = nodeConfigList.Where(x => !x.Value.isSent).FirstOrDefault();
            if (!dataToSend.Equals(default(KeyValuePair<ushort, NodeConfigurationData>)))
            {
                PositionConfigurationCommandPage positionPage = new PositionConfigurationCommandPage();
                positionPage.NodeId = dataToSend.Key;
                positionPage.NodeType = dataToSend.Value.nodeType;
                positionPage.NodeRotation = dataToSend.Value.nodeRotation;

                // Send the node data as an acknowledged message, retrying up to 5 times
                if (pageSender.SendAcknowledged(positionPage, true, 5))
                {
                    // If the message succeeded, mark the node data as sent
                    // so we do not send it to the gateway again (unless it changes)
                    dataToSend.Value.isSent = true;
                }
            }
        }

        private void updateNodeConfigList()
        {      
            // Update the list to be sent
            foreach (SensorNode node in ConfigUI.nodes)
            {
                addToNodeConfigList((ushort)node.NodeID, new NodeConfigurationData(node.configuration, node.Rotation));
            }
        }

        /*
         * Adds node config data to the node config list at the given node ID only if
         * a) there is no data yet for the given node ID or b) the provided data is different from the current data
         */
        private void addToNodeConfigList(ushort nodeId, NodeConfigurationData newData)
        {
            NodeConfigurationData dataToReplace;
            nodeConfigList.TryGetValue(nodeId, out dataToReplace);

            // Save the data if data for this node ID is not yet in the list,
            // OR data for this node ID exists in the list and differs from the new data
            if (dataToReplace == null || !dataToReplace.Equals(newData))
            {
                nodeConfigList[nodeId] = newData;

                // TODO: Could trigger a visual indication that there is data waiting to be sent to the gateway node
            }
        }

        #endregion

        #region Private Classes

        // Used to keep track of node config data which needs to be sent to the gateway node
        private class NodeConfigurationData
        {
            public HardwareConfiguration nodeType;
            public NodeRotation nodeRotation;
            public bool isSent = false;

            public NodeConfigurationData(HardwareConfiguration nodeType, NodeRotation nodeRotation)
            {
                this.nodeType = nodeType;
                this.nodeRotation = nodeRotation;
            }

            public override bool Equals(object obj)
            {
                var nodeConfigData = obj as NodeConfigurationData;

                if (nodeConfigData == null)
                {
                    return false;
                }

                return (this.nodeType == nodeConfigData.nodeType
                    && this.nodeRotation == nodeConfigData.nodeRotation);
            }

            public override int GetHashCode()
            {
                throw new InvalidOperationException("Shouldn't be called");
            }
        }

        #endregion
    }
}
