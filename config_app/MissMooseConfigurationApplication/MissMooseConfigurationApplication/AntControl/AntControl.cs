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

        // ANT Device which opens slave channels to pair to node devices
        private ANT_Device device;        
		
        // Slave channel to search for node devices
        private ANT_Channel nodeSearchChannel;
        private static readonly int nodeSearchChannelNum = 0;

        // Slave channel to communicate with the gateway node
        private ANT_Channel gatewayChannel;
        private static readonly int gatewayChannelNum = 1;

        // Parser to decode any valid received data pages
        private PageParser pageParser;

        // Handles sending data pages to nodes being configured
        private PageSender nodeSearchPageSender;

        // Handles sending data pages to the gateway node
        private PageSender gatewayPageSender;

        // Properties for an ANT slave channel which can pair to node devices
        private static readonly byte rfFrequency = 66;          // 2466 MHz
        private static readonly byte deviceType = 1;            // search for node master device (nodes have device type 1)
        private static readonly byte transmissionType = 0;      // wildcard for search
        private static readonly UInt16 channelPeriod = 8192;    // same channel period as node master devices
        private static readonly ANT_ReferenceLibrary.ChannelType channelType = ANT_ReferenceLibrary.ChannelType.BASE_Slave_Receive_0x00;

        // Device number for node search ANT slave channel
        private static readonly ushort nodeSearchDeviceNumber = 0; // wildcard for search

        // BLAZE node config properties
        private static readonly UInt16 maxNodeId = 512;
        private static UInt16 nextNodeId = 1;
        private static readonly UInt16 networkId = 20000;

        // Device number of the connected master device
        private ushort masterDeviceNumber = 0;

        // All assigned node IDs, using device numbers as dictionary keys
        private Dictionary<ushort, ushort> nodeIds;

        // Device numbers of discovered gateway device
        private ushort gatewayDeviceNumber;

        // Node configuration data waiting to be sent to a gateway node, using node ID as dictionary keys
        private Dictionary<ushort, NodeConfigurationData> nodeConfigList;

        // Booleans to keep track of state
        private bool deviceRunning = false;
        private bool sendingNodeId = false;
        private bool openedGatewayChannel = false;

        // Default wait time in milliseconds for ANT function calls
        private static readonly ushort waitTime = 500;

        #endregion

        #region Public Methods

        public AntControl()
        {
            // Set up the page parser to decode valid data pages
            pageParser = new PageParser();
            pageParser.AddDataPage(new NodeStatusPage());

            // Create node data lists
            nodeIds = new Dictionary<ushort, ushort>();
            nodeConfigList = new Dictionary<ushort, NodeConfigurationData>();            
        }

        public void AddConfigUI(ConfigurationPage ConfigUI)
        {
            this.ConfigUI = ConfigUI;

            // Open the ANT slave channel to search for a node
            startConfigDevice();

            ////demo add existing node
            //var node = new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS, 2);
            //node.SetPosition(1, 1);
            //node.SetRotation(NodeRotation.NODEROTATION_135);
            //node.SetOffset(-3, 3);

            //ConfigUI.AddExistingNode(node);

            ////demo add new node
            //ConfigUI.AddNewNode(new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_2_PIR, 3));


            ////demo read input state
            //var nodes = ConfigUI.GetCurrentNodes();
        }
		
        public void AddMonitoringUI(SystemOverviewPage MonitoringUI)
        {
            this.MonitoringUI = MonitoringUI;

            //demo: add existing node
            var node = new SensorNode(HardwareConfiguration.PirLidarLed, 2);
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
            node = new SensorNode(HardwareConfiguration.Pir2, 3);
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
                device = new ANT_Device();
                device.deviceResponse += deviceResponse;

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
                    deviceRunning = true;
                    device.enableRxExtendedMessages(true);

                    try
                    {
                        // Get an unused ANT channel from the device
                        nodeSearchChannel = device.getChannel(nodeSearchChannelNum);
                        nodeSearchChannel.channelResponse += new dChannelResponseHandler(nodeSearchChannelResponse);

                        setupAndOpenChannel(nodeSearchChannel, nodeSearchDeviceNumber, true);

                        // Initialize a page sender with the channel
                        nodeSearchPageSender = new PageSender(nodeSearchChannel);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("Can't start configuration slave channel: " + e.Message);
                        shutdownConfigDevice();
                    }
                }
            }
        }

        private void shutdownConfigDevice()
        {
            //If you need to manually disconnect from a ANTDevice call the shutdownDeviceInstance method,
            //It releases all the resources, the device, and nullifies the object reference
            ANT_Device.shutdownDeviceInstance(ref device);
            deviceRunning = false;
        }

        private void setupAndOpenChannel(ANT_Channel channel, ushort deviceNumber, bool useExclusionList)
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
                            nodeSearchChannel.includeExcludeList_Configure(0, true);

                            // Unassign the channel
                            if (nodeSearchChannel.unassignChannel(waitTime))
                            {
                                Console.WriteLine("Unassigned channel");
                            }
                            else
                            {
                                throw new Exception("Channel unassign operation failed.");
                            }

                            // Reassign and open the channel, excluding the current master device number from the search
                            // so we don't just pair to it again
                            setupAndOpenChannel(nodeSearchChannel, nodeSearchDeviceNumber, true);

                            break;
                    }
                    break;                    
            }
        }

        /*
         * Handles the ANT channel response for the node search channel
         */
        private void nodeSearchChannelResponse(ANT_Response response)
        {
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
            {
                // If a search timeout occurs, reopen the channel
                if (response.getChannelEventCode() == ANT_ReferenceLibrary.ANTEventID.EVENT_RX_SEARCH_TIMEOUT_0x01)
                {
                    nodeSearchChannel.openChannel();
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
         * Handles the ANT channel response for the gateway channel
         */
        private void gatewayChannelResponse(ANT_Response response)
        {
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
            {
                // If a search timeout occurs, reopen the channel
                if (response.getChannelEventCode() == ANT_ReferenceLibrary.ANTEventID.EVENT_RX_SEARCH_TIMEOUT_0x01)
                {
                    gatewayChannel.openChannel();
                }
            }
            //This is a receive event, so handle the received ANT message
            else
            {
                byte[] rxBuffer = response.getDataPayload();

                dynamic dataPage = pageParser.Parse(rxBuffer);

                if (dataPage != null)
                {
                    handlePage(dataPage);
                }

                // Send config data about the other nodes to the gateway node
                sendNodeConfigurationToGateway();
            }
        }        

        /*
         * Processes a received Node Status data page 
         */
        private void handlePage(NodeStatusPage dataPage)
        {
            // If the node is missing a node ID or a network ID, send a Network Configuration Command page
            if (dataPage.NodeId == 0 || dataPage.NetworkId == 0)
            {
                sendNetworkConfiguration();

                if (dataPage.NodeId == 0)
                {
                    sendingNodeId = true;
                }
            }
            // If the node has a node ID and we just configured it, increment the next node ID to be assigned
            else if (sendingNodeId && !dataPage.IsGatewayNode)
            {
                sendingNodeId = false;               
            }

            if (dataPage.IsGatewayNode)
            {
                // If this gateway belongs to the network the app is set up to manage,
                // open a second channel to pair to this gateway (only if not already opened)
                if (!openedGatewayChannel && dataPage.NetworkId == networkId)
                {
                    gatewayDeviceNumber = masterDeviceNumber;
                    openGatewayChannel(masterDeviceNumber);
                }
            }

            if (dataPage.NodeId != 0 && dataPage.NodeType != HardwareConfiguration.Unknown)
            {
                if (!nodeIds.Keys.Contains(masterDeviceNumber) && (!dataPage.IsGatewayNode || gatewayDeviceNumber == masterDeviceNumber))
                {
                    // Add a new node to the UI for the useru
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate {

                        ConfigUI.AddNewNode(new SensorNode(dataPage.NodeType, dataPage.NodeId));
                    });

                    // Save this node's device number
                    nodeIds.Add(masterDeviceNumber, dataPage.NodeId);

                    // Close the channel
                    if (nodeSearchChannel.closeChannel(waitTime))
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
        }
        
        private void openGatewayChannel(ushort deviceNumber)
        {
            try
            {
                // Get an unused ANT channel from the device
                gatewayChannel = device.getChannel(gatewayChannelNum);
                gatewayChannel.channelResponse += new dChannelResponseHandler(gatewayChannelResponse);

                setupAndOpenChannel(gatewayChannel, deviceNumber, false);

                // Initialize a page sender with the channel
                gatewayPageSender = new PageSender(gatewayChannel);

                openedGatewayChannel = true;
            }
            catch (Exception e)
            {
                Console.WriteLine("Can't start gateway slave channel: " + e.Message);
                shutdownConfigDevice();
            }
        }

        private void sendNetworkConfiguration()
        {
            NetworkConfigurationCommandPage configPage = new NetworkConfigurationCommandPage();

            configPage.NodeId = getNextNodeId();
            configPage.NetworkId = networkId;

            nodeSearchPageSender.SendAcknowledged(configPage, false);
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
                positionPage.xpos = dataToSend.Value.xpos;
                positionPage.ypos = dataToSend.Value.xpos;
                positionPage.xoffset = dataToSend.Value.xpos;
                positionPage.yoffset = dataToSend.Value.xpos;

                if(positionPage.xpos == -1 || positionPage.ypos == -1)
                {
                    return;
                }
				
				dataToSend.Value.isSent = true;

                // Send the node data as an acknowledged message, retrying up to 5 times
                if (!gatewayPageSender.SendAcknowledged(positionPage, true, 5))
                {
                    // If the message failed, mark the node data as unsent
                    // so that we try to send it again
                    dataToSend.Value.isSent = false;
                }
            }
        }

        private void updateNodeConfigList()
        {      
            // Update the list to be sent
            foreach (SensorNode node in ConfigUI.nodes)
            {
                addToNodeConfigList((ushort)node.NodeID, new NodeConfigurationData(
                    node.configuration,
                    new NodeRotation(node.Rotation.Val),
                    node.xpos, node.ypos, node.xoffset, node.yoffset));
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

        private ushort getNextNodeId()
        {
            while (nodeIds.Values.Contains(nextNodeId))
            {
                nextNodeId++;
            }

            if (nextNodeId > maxNodeId) Console.WriteLine("Maximum node ID (512) reached. Cannot configure another node.");

            return nextNodeId;
        }

        #endregion

        #region Private Classes

        // Used to keep track of node config data which needs to be sent to the gateway node
        private class NodeConfigurationData
        {
            public HardwareConfiguration nodeType;
            public NodeRotation nodeRotation;
            public int xpos;
            public int ypos;
            public int xoffset;
            public int yoffset;
            public bool isSent = false;

            public NodeConfigurationData(HardwareConfiguration nodeType, NodeRotation nodeRotation, int xpos, int ypos, int xoffset, int yoffset)
            {
                this.nodeType = nodeType;
                this.nodeRotation = nodeRotation;
                this.xpos = xpos;
                this.ypos = ypos;
                this.xoffset = xoffset;
                this.yoffset = yoffset;
            }

            public override bool Equals(object obj)
            {
                var nodeConfigData = obj as NodeConfigurationData;

                if (nodeConfigData == null)
                {
                    return false;
                }

                if( (nodeConfigData.xpos != xpos) ||
                    (nodeConfigData.ypos !=ypos) ||
                    (nodeConfigData.xoffset != xoffset) ||
                    (nodeConfigData.yoffset != yoffset) )
                {
                    return false;
                }

                return (this.nodeType == nodeConfigData.nodeType
                    && this.nodeRotation.ToEnum() == nodeConfigData.nodeRotation.ToEnum());
            }

            public override int GetHashCode()
            {
                throw new InvalidOperationException("Shouldn't be called");
            }
        }

        #endregion
    }
}
