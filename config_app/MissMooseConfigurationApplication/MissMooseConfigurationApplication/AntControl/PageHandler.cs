using MissMooseConfigurationApplication.UIPages;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace MissMooseConfigurationApplication
{
    class PageHandler
    {
        #region Private Members

        //interfaces to UI
        private ConfigurationPage ConfigUI;
        private SystemOverviewPage MonitoringUI;

        // BLAZE node config properties
        private static readonly ushort maxNodeId = 512;
        private static ushort nextNodeId = 1;
        private static readonly ushort networkId = 20000;

        // Node configuration data waiting to be sent to a gateway node, using node ID as dictionary keys
        private Dictionary<ushort, NodeConfigurationData> nodeConfigList;

        // Default distance between nodes in meters
        private static readonly byte nodeSeparation = 8;

        #endregion

        #region Public Methods

        public PageHandler()
        {
            // Create node data lists
            nodeConfigList = new Dictionary<ushort, NodeConfigurationData>();
        }

        public void AddConfigUI(ConfigurationPage ConfigUI)
        {
            this.ConfigUI = ConfigUI;
        }

        public void AddMonitoringUI(SystemOverviewPage MonitoringUI)
        {
            this.MonitoringUI = MonitoringUI;
        }

        /*
         * Processes a received Node Status data page 
         */
        public bool HandlePage(NodeStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            // If the node is missing a node ID or a network ID, send a Network Configuration Command page
            if (dataPage.NodeId == 0 || dataPage.NetworkId == 0)
            {
                ushort nodeId;
                SensorNode node = ConfigUI.nodes.Where(x => x.DeviceNumber == deviceNum).FirstOrDefault();

                // If we previously configured a node with this ANT device number,
                // assign it the same node ID as last time.                
                if (node != null)
                {
                    nodeId = node.NodeID;

                    // If this node which we are reconfiguring is the gateway node,
                    // it has probably lost all of its node position data.
                    // Mark all node data as unsent so that the gateway gets it again.
                    foreach (NodeConfigurationData data in nodeConfigList.Values)
                    {
                        data.isSent = false;
                    }
                }
                // Otherwise, give it a new node ID.
                else
                {
                    nodeId = GetNextNodeId();
                }

                SendNetworkConfiguration(nodeId, responder);

                
            }

            if (dataPage.IsGatewayNode)
            {
                return HandleNodeConfigPage_Gateway(dataPage, deviceNum, responder);
            }
            else
            {
                return HandleNodeConfigPage_Node(dataPage, deviceNum, responder);
            }
        }

        public bool HandlePage(LidarMonitoringPage dataPage, ushort deviceNum, PageSender responder)
        {
            SensorNode node = ConfigUI.nodes.Where(x => x.NodeID == dataPage.NodeId).FirstOrDefault();

            if (node != null)
            {
                Rotation totalRotation = new Rotation(node.Rotation.Val);
                totalRotation.Add(dataPage.SensorRotation);


                if (GetLineDirection(totalRotation, out LineDirection direction))
                {
                    Brush colour = dataPage.Distance < (2 * nodeSeparation * 100) ? StatusColour.Red : StatusColour.Blue;

                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        MonitoringUI.MarkSensorDetection(node, direction, colour);
                    });
                }
 
            }

            // Send an acknowledgement page so the gateway knows this sensor data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for Lidar Msg: " + ackPage.MessageId);

            //keep connection alive
            return true;
        }

        public bool HandlePage(PirMonitoringPage dataPage, ushort deviceNum, PageSender responder)
        {
            SensorNode node = ConfigUI.nodes.Where(x => x.NodeID == dataPage.NodeId).FirstOrDefault();

            if (node != null)
            {
                Rotation totalRotation = new Rotation(node.Rotation.Val);
                totalRotation.Add(dataPage.SensorRotation);


                if (GetLineDirection(totalRotation, out LineDirection direction))
                {
                    Brush colour = dataPage.Detection ? StatusColour.Red : StatusColour.Blue;

                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        MonitoringUI.MarkSensorDetection(node, direction, colour);
                    });
                }

                
            }

            // Send an acknowledgement page so the gateway knows this sensor data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for PIR Msg: " + ackPage.MessageId);

            //keep connection alive
            return true;
        }

        #endregion

        #region Private Methods

        private bool HandleNodeConfigPage_Gateway(NodeStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            if (dataPage.NodeId != 0 && dataPage.NodeType != HardwareConfiguration.Unknown)
            {
                if (ConfigUI.nodes.Count(x => x.DeviceNumber == deviceNum) == 0)
                {
                    // Add a new node to the UI for the user
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate {

                        ConfigUI.AddNewNode(new SensorNode(deviceNum, dataPage.NodeType, dataPage.NodeId, true));
                    });
                }
            }

            SendNodeConfigurationToGateway(responder);

            //keep connection alive
            return true;
        }

        private bool HandleNodeConfigPage_Node(NodeStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            if (dataPage.NodeId != 0 && dataPage.NodeType != HardwareConfiguration.Unknown)
            {
                if (ConfigUI.nodes.Count(x => x.DeviceNumber == deviceNum) == 0)
                {
                    // Add a new node to the UI for the user
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate {

                        ConfigUI.AddNewNode(new SensorNode(deviceNum, dataPage.NodeType, dataPage.NodeId, false));
                    });

                    //close connection
                    return false;
                }
            }

            //keep connection alive
            return true;
        }

        private void SendNetworkConfiguration(ushort nodeId, PageSender responder)
        {
            NetworkConfigurationCommandPage configPage = new NetworkConfigurationCommandPage
            {
                NodeId = nodeId,
                NetworkId = networkId
            };

            responder.SendAcknowledged(configPage, true, 5);

            Console.Out.WriteLine("Device" +  configPage.NodeId  + "config started.");
        }

        private void SendNodeConfigurationToGateway(PageSender responder)
        {
            // TODO: add a save button to the UI that triggers updating the node configuration data list.
            // For now, poll the configuration page's sensor node list for updates every time we send data to the gateway
            UpdateNodes();

            KeyValuePair<ushort, NodeConfigurationData> dataToSend = nodeConfigList.Where(x => !x.Value.isSent).FirstOrDefault();
            if (!dataToSend.Equals(default(KeyValuePair<ushort, NodeConfigurationData>)))
            {
                PositionConfigurationCommandPage positionPage = new PositionConfigurationCommandPage
                {
                    NodeId = dataToSend.Key,
                    NodeType = dataToSend.Value.nodeType,
                    NodeRotation = dataToSend.Value.nodeRotation,
                    xpos = dataToSend.Value.xpos,
                    ypos = dataToSend.Value.xpos,
                    xoffset = dataToSend.Value.xoffset,
                    yoffset = dataToSend.Value.yoffset
                };

                if (positionPage.xpos == -1 || positionPage.ypos == -1)
                {
                    return;
                }

                dataToSend.Value.isSent = true;

                // Send the node data as an acknowledged message, retrying up to 5 times
                if (!responder.SendAcknowledged(positionPage, true, 5))
                {

                    // If the message failed, mark the node data as unsent
                    // so that we try to send it again
                    dataToSend.Value.isSent = false;
                }
                else
                {
                    Console.Out.WriteLine("Sent position for: " + positionPage.NodeId);
                }
            }
        }

        private void UpdateNodes()
        {
            // Go through the list of nodes and check if any have changed
            foreach (SensorNode node in ConfigUI.nodes)
            {
                NodeConfigurationData newData = new NodeConfigurationData(
                    node.configuration,
                    new Rotation(node.Rotation.Val),
                    node.xpos, node.ypos, node.xoffset, node.yoffset);

                nodeConfigList.TryGetValue((ushort)node.NodeID, out NodeConfigurationData dataToReplace);

                // Save the data if data for this node ID is not yet in the list,
                // OR data for this node ID exists in the list and differs from the new data
                if (dataToReplace == null || !dataToReplace.Equals(newData))
                {
                    // Update the list to be sent
                    nodeConfigList[(ushort)node.NodeID] = newData;

                    // TODO: Could trigger a visual indication that there is data waiting to be sent to the gateway node

                    // A node has changed, so update the monitoring UI
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate {

                        MonitoringUI.UpdateNode(node);
                    });
                }
            }
        }

        private ushort GetNextNodeId()
        {
            while (ConfigUI.nodes.Count(x => x.NodeID == nextNodeId) != 0)
            {
                nextNodeId++;
            }

            if (nextNodeId > maxNodeId) Console.WriteLine("Maximum node ID (512) reached. Cannot configure another node.");

            return nextNodeId;
        }

        private bool GetLineDirection(Rotation rotation, out LineDirection direction)
        {
            switch (rotation.ToEnum())
            {
                case 0:
                    direction = LineDirection.Up;
                    return true;
                case 2:
                    direction = LineDirection.Right;
                    return true;
                case 4:
                    direction = LineDirection.Down;
                    return true;
                case 6:
                    direction = LineDirection.Left;
                    return true;
                default:
                    direction = LineDirection.Up;
                    // Return false for in-between angles like 45 degrees (we don't have coloured lines for those angles)
                    return false;
            }
        }

        #endregion
    }
}
