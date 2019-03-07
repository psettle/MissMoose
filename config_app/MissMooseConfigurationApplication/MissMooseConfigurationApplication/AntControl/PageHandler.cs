using ANT_Managed_Library;
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

        // The most recently received monitoring data message ID
        private byte? monitoringMessageId = null;

        // The most recently received error data message ID
        private byte? errorMessageId = null;

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
        public void HandlePage(NodeStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            if (dataPage.IsGatewayNode)
            {
                HandleNodeConfigPage_Gateway(dataPage, deviceNum, responder);
            }
            else
            {
                HandleNodeConfigPage_Node(dataPage, deviceNum, responder);
            }
        }

        public void HandlePage(LidarMonitoringPage dataPage, ushort deviceNum, PageSender responder)
        {
            // Only handle the data page if its message ID is different
            // than the most recently received monitoring data page.
            // This prevents unnecessary UI updates and duplicate logs.
            if (dataPage.MessageId != monitoringMessageId)
            {
                // Save the data page's message ID so we can ignore duplicates
                monitoringMessageId = dataPage.MessageId;

                SensorNode node = ConfigUI.nodes.Where(x => x.NodeID == dataPage.NodeId).FirstOrDefault();

                if (node != null)
                {
                    Rotation totalRotation = new Rotation(node.Rotation.Val);
                    totalRotation.Add(dataPage.SensorRotation);

                    if (GetLineDirection(totalRotation, out LineDirection direction))
                    {
                        SolidColorBrush colour = dataPage.Region > LidarRegion.None ? StatusColour.Red : StatusColour.Blue;

                        Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                        {
                            MonitoringUI.MarkSensorDetection(node, direction, colour);
                        });
                    }

                    String logString = GetStringFromRotation(totalRotation)
                        + " LIDAR sensor on node " + node.DisplayID;

                    switch (dataPage.Region)
                    {
                        case LidarRegion.None:
                            logString += " is not detecting an object within the network";
                            break;
                        case LidarRegion.Region1:
                            logString += " is detecting an object at " + dataPage.Distance / 100.0 + " m"
                                + ", which is in the closest network region.";
                            break;
                        case LidarRegion.Region2:
                            logString += " is detecting an object at " + dataPage.Distance / 100.0 + " m"
                                + ", which is in the adjacent network region.";
                            break;
                        default:
                            // Do nothing (shouldn't get here)
                            break;
                    }

                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        MonitoringUI.LogEvent(logString);
                    });
                }
            }

            // Send an acknowledgement page so the gateway knows this sensor data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId,
                AckedId = dataPage.DataPageNumber
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for Lidar Msg: " + ackPage.MessageId);
        }

        public void HandlePage(PirMonitoringPage dataPage, ushort deviceNum, PageSender responder)
        {
            // Only handle the data page if its message ID is different
            // than the most recently received monitoring data page.
            // This prevents unnecessary UI updates and duplicate logs.
            if (dataPage.MessageId != monitoringMessageId)
            {
                // Save the data page's message ID so we can ignore duplicates
                monitoringMessageId = dataPage.MessageId;

                SensorNode node = ConfigUI.nodes.Where(x => x.NodeID == dataPage.NodeId).FirstOrDefault();

                if (node != null)
                {
                    Rotation totalRotation = new Rotation(node.Rotation.Val);
                    totalRotation.Add(dataPage.SensorRotation);

                    if (GetLineDirection(totalRotation, out LineDirection direction))
                    {
                        SolidColorBrush colour = dataPage.Detection ? StatusColour.Red : StatusColour.Blue;

                        Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                        {
                            MonitoringUI.MarkSensorDetection(node, direction, colour);
                        });
                    }

                    String logString = GetStringFromRotation(totalRotation)
                        + " PIR sensor on node " + node.DisplayID;

                    if (dataPage.Detection)
                    {
                        logString += " started detecting";
                    }
                    else
                    {
                        logString += " stopped detecting";
                    }

                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        MonitoringUI.LogEvent(logString);
                    });
                }
            }

            // Send an acknowledgement page so the gateway knows this sensor data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId,
                AckedId = dataPage.DataPageNumber
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for PIR Msg: " + ackPage.MessageId);
        }

        public void HandlePage(LedOutputStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            // Only handle the data page if its message ID is different
            // than the most recently received monitoring data page.
            // This prevents unnecessary UI updates and duplicate logs.
            if (dataPage.MessageId == 0)
            {
                // Save the data page's message ID so we can ignore duplicates
                monitoringMessageId = dataPage.MessageId;

                SensorNode node = ConfigUI.nodes.Where(x => x.NodeID == dataPage.NodeId).FirstOrDefault();

                if (node != null)
                {
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        bool functionApplied = node.SetLedFunction(dataPage.LedFunction);
                        bool colourApplied = node.SetLedColour(dataPage.LedColour);

                        if (functionApplied || colourApplied)
                        {
                            MonitoringUI.UpdateNode(node);
                        }
                    });
                }

                Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                {
                    if (dataPage.LedFunction == LedFunction.Off)
                    {
                        MonitoringUI.LogEvent("LED turned off on node " + node.DisplayID);
                    }
                    else
                    {
                        MonitoringUI.LogEvent("LED changed to " + dataPage.LedColour + " and " + dataPage.LedFunction
                        + " on node " + node.DisplayID);
                    }
                });
            }

            // Send an acknowledgement page so the gateway knows this LED data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId,
                AckedId = dataPage.DataPageNumber
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for LED Msg: " + ackPage.MessageId);
        }

        public void HandlePage(RegionActivityVariablePage dataPage, ushort deviceNum, PageSender responder)
        {
            // Only handle the data page if its message ID is different
            // than the most recently received monitoring data page.
            // This prevents unnecessary UI updates and duplicate logs.
            if (dataPage.MessageId != monitoringMessageId)
            {
                // Save the data page's message ID so we can ignore duplicates
                monitoringMessageId = dataPage.MessageId;

                Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                {
                    MonitoringUI.SetRegionActivityVariable(dataPage.XCoordinate, dataPage.YCoordinate, dataPage.RegionStatus);
                });

                String regionString = "x = " + dataPage.XCoordinate + ", y = " + dataPage.YCoordinate;

                String logString = "";

                Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                {
                    switch (dataPage.RegionStatus)
                    {
                        case RegionStatus.NoDetection:
                            logString += "System is no longer detecting wildlife";
                            break;
                        case RegionStatus.ProbableDetection:
                            logString += "System is detecting probable wildlife";
                            break;
                        case RegionStatus.DefiniteDetection:
                            logString += "System is detecting wildlife";
                            MonitoringUI.IncrementDetectionCount();
                            break;
                        default:
                            // Do nothing (shouldn't get here)
                            break;
                    }

                    logString += " in region " + regionString + " with Activity Variable " + dataPage.ActivityVariable;

                    MonitoringUI.LogEvent(logString);
                });
            }

            // Send an acknowledgement page so the gateway knows this region AV data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId,
                AckedId = dataPage.DataPageNumber
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for Region AV Msg: " + ackPage.MessageId);
        }

        public void HandlePage(HyperactivityErrorStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            // Only handle the data page if its message ID is different
            // than the most recently received error status page.
            // This prevents unnecessary UI updates and duplicate logs.
            if (dataPage.MessageId != errorMessageId)
            {
                // Save the data page's message ID so we can ignore duplicates
                errorMessageId = dataPage.MessageId;

                SensorNode node = ConfigUI.nodes.Where(x => x.NodeID == dataPage.NodeId).FirstOrDefault();

                if (node != null)
                {
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        Rotation totalRotation = new Rotation(node.Rotation.Val);
                        totalRotation.Add(dataPage.SensorRotation);

                        String logString = GetStringFromRotation(totalRotation)
                        + dataPage.SensorType.ToString().ToUpper() + " sensor on node " + node.DisplayID;

                        if (dataPage.ErrorOccurring)
                        {
                            logString += " is hyperactive, and may be malfunctioning";
                            MonitoringUI.LogSystemProblem(logString);

                            node.SetStatusColour(StatusColour.Red);
                            MonitoringUI.UpdateNode(node);
                        }
                        else
                        {
                            logString += " is no longer hyperactive";

                            node.SetStatusColour(StatusColour.Blue);
                            MonitoringUI.UpdateNode(node);
                        }

                        MonitoringUI.LogEvent(logString);
                    });
                }
            }

            // Send an acknowledgement page so the gateway knows this error data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId,
                AckedId = dataPage.DataPageNumber
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for Hyperactivity Error Msg: " + ackPage.MessageId);
        }

        public void HandlePage(InactiveSensorErrorStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            // Only handle the data page if its message ID is different
            // than the most recently received error status page.
            // This prevents unnecessary UI updates and duplicate logs.
            if (dataPage.MessageId != errorMessageId)
            {
                // Save the data page's message ID so we can ignore duplicates
                errorMessageId = dataPage.MessageId;

                SensorNode node = ConfigUI.nodes.Where(x => x.NodeID == dataPage.NodeId).FirstOrDefault();

                if (node != null)
                {
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate
                    {
                        Rotation totalRotation = new Rotation(node.Rotation.Val);
                        totalRotation.Add(dataPage.SensorRotation);

                        String logString = GetStringFromRotation(totalRotation)
                        + dataPage.SensorType.ToString().ToUpper() + " sensor on node " + node.DisplayID;

                        if (dataPage.ErrorOccurring)
                        {
                            logString += " is inactive, and may be malfunctioning";
                            MonitoringUI.LogSystemProblem(logString);

                            node.SetStatusColour(StatusColour.Red);
                            MonitoringUI.UpdateNode(node);
                        }
                        else
                        {
                            logString += " is no longer inactive";

                            node.SetStatusColour(StatusColour.Blue);
                            MonitoringUI.UpdateNode(node);
                        }

                        MonitoringUI.LogEvent(logString);
                    });
                }
            }

            // Send an acknowledgement page so the gateway knows this error data was received
            MonitoringDataAckPage ackPage = new MonitoringDataAckPage
            {
                MessageId = dataPage.MessageId,
                AckedId = dataPage.DataPageNumber
            };

            responder.SendBroadcast(ackPage);
            Console.Out.WriteLine("Sent Ack for Inactive Sensor Error Msg: " + ackPage.MessageId);
        }

        #endregion

        #region Private Methods

        private void HandleNodeConfigPage_Gateway(NodeStatusPage dataPage, ushort deviceNum, PageSender responder)
        {
            // If the node is missing a network ID, send a Network Configuration Command page
            if (dataPage.NetworkId == 0)
            {
                SensorNode node = ConfigUI.nodes.Where(x => x.DeviceNumber == deviceNum).FirstOrDefault();

                // If we previously configured a node with this ANT device number,
                // assign it the same node ID as last time.                
                if (node != null)
                {
                    // We are reconfiguring the gateway node,
                    // so it has probably lost all of its node position data.
                    // Mark all node data as unsent so that the gateway gets it again.
                    foreach (NodeConfigurationData data in nodeConfigList.Values)
                    {
                        data.isSent = false;
                    }
                }

                // The gateway node has a hardcoded node ID, so we only need to send it a network ID.
                // Send the node ID field in the network configuration page to 0.
                SendNetworkConfiguration(0, responder);
            }
            else if (dataPage.NodeType != HardwareConfiguration.Unknown)
            {
                if (ConfigUI.nodes.Count(x => x.DeviceNumber == deviceNum) == 0)
                {
                    // Add a new node to the UI for the user
                    Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate {
                        if (!ConfigUI.NewNodeBoxFull)
                        {
                            ConfigUI.AddNewNode(new SensorNode(deviceNum, dataPage.NodeType, dataPage.NodeId, true));
                        }                        
                    });
                }
            }

            SendNodeConfigurationToGateway(deviceNum, responder);
        }

        private void HandleNodeConfigPage_Node(NodeStatusPage dataPage, ushort deviceNum, PageSender responder)
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
                }
                // Otherwise, give it a new node ID.
                else
                {
                    nodeId = GetNextNodeId();
                }

                if(SendNetworkConfiguration(nodeId, responder))
                {
                    if (ConfigUI.nodes.Count(x => x.DeviceNumber == deviceNum) == 0)
                    {
                        // Add a new node to the UI for the user
                        Application.Current.Dispatcher.BeginInvoke((ThreadStart)delegate {
                            if (!ConfigUI.NewNodeBoxFull)
                            {
                                ConfigUI.AddNewNode(new SensorNode(deviceNum, dataPage.NodeType, nodeId, false));
                            }
                        });
                    }
                }
            }
        }

        private bool SendNetworkConfiguration(ushort nodeId, PageSender responder)
        {
            NetworkConfigurationCommandPage configPage = new NetworkConfigurationCommandPage
            {
                NodeId = nodeId,
                NetworkId = networkId
            };

            bool result = responder.SendAcknowledged(configPage, true, 5);

            Console.Out.WriteLine("Device" +  configPage.NodeId  + "config: " + result);

            return result;
        }

        private void SendNodeConfigurationToGateway(ushort deviceNum, PageSender responder)
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
                    ypos = dataToSend.Value.ypos,
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

        private String GetStringFromRotation(Rotation rotation)
        {
            switch (rotation.ToEnum())
            {
                case 0:
                    return "road facing";
                case 2:
                    return "right of road facing";
                case 4:
                    return "anti-road facing";
                case 6:
                    return "left of road facing";
                default:
                    // Should never get here
                    return "unknown rotation";
            }
        }

        #endregion
    }
}
