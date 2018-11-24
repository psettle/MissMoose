using ANT_Managed_Library;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MissMooseConfigurationApplication
{
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        //This delegate is for using the dispatcher to avoid conflicts of the feedback thread referencing the UI elements
        delegate void dAppendText(String toAppend);
        delegate void dScrollToEnd();

        // ANT Device which opens a slave channel to search for node devices
        ANT_Device device;

        // Slave channel to search for node devices
        ANT_Channel channel;

        // Properties for the ANT slave channel
        static readonly byte rfFrequency        = 66;           // 2466 MHz
        byte transmissionType                   = 0;            // wildcard for search
        static readonly byte deviceType         = 1;            // search for node master device (nodes have device type 1)
        UInt16 deviceNumber                     = 0;            // wildcard for search
        static readonly UInt16 channelPeriod    = 8192;         // same channel period as node master devices
        static readonly ANT_ReferenceLibrary.ChannelType channelType = ANT_ReferenceLibrary.ChannelType.BASE_Slave_Receive_0x00;

        Boolean deviceRunning = false;

        Boolean nodeRequiresConfiguration = true;

        // BLAZE node config properties
        static readonly UInt16 maxNodeId = 512;
        static UInt16 nextNodeId = 1;
        static readonly UInt16 networkId = 1;

        public NodeStatusPage NodeStatusPage { get; set; }

        public ConfigurationCommandPage ConfigurationCommandPage { get; set; }

        ushort masterDeviceNumber = 0;
        public ushort MasterDeviceNumber
        {
            get { return masterDeviceNumber; }
            set { masterDeviceNumber = value; OnPropertyChanged("MasterDeviceNumber"); }
        }

        public MainWindow()
        {
            InitializeComponent();

            this.DataContext = this;

            NodeStatusPage = new NodeStatusPage();
            ConfigurationCommandPage = new ConfigurationCommandPage();
        }

        public void StartConfigDevice(object sender, RoutedEventArgs e)
        {
            if (!deviceRunning)
            {
                if (ANT_Slave_Start())
                {
                    deviceRunning = true;
                    button_slave.Background = System.Windows.Media.Brushes.Salmon;
                    button_slave.Content = "Stop Configuration";
                    return;
                }
            }
            //If we get here it means we failed startup or are stopping the demo
            ShutdownConfigDevice();
            button_slave.Background = System.Windows.Media.Brushes.LightGreen;
            button_slave.Content = "Start Configuration";
        }

        public Boolean ANT_Slave_Start()
        {
            try
            {
                device = new ANT_Device();
                device.deviceResponse += new ANT_Device.dDeviceResponseHandler(deviceResponse);
                device.getChannel(0).channelResponse += new dChannelResponseHandler(channelResponse);

                setupAndOpen(device, channelType);
            }
            catch (Exception e)
            {
                Console.WriteLine("Can't start configuration slave channel: " + e.Message);
                return false;
            }
            return true;
        }

        public void ShutdownConfigDevice()
        {
            //If you need to manually disconnect from a ANTDevice call the shutdownDeviceInstance method,
            //It releases all the resources, the device, and nullifies the object reference
            ANT_Device.shutdownDeviceInstance(ref device);
            deviceRunning = false;
        }

        private void setupAndOpen(ANT_Device deviceToSetup, ANT_ReferenceLibrary.ChannelType channelType)
        {
            //We try-catch and forward exceptions to the calling function to handle and pass the errors to the user
            try
            {
                //To access an ANTChannel on a paticular device we need to get the channel from the device
                //Once again, this ensures you have a valid object associated with a real-world ANTChannel
                //ie: You can only get channels that actually exist
                channel = deviceToSetup.getChannel(0);

                //Almost all functions in the library have two overloads, one with a response wait time and one without
                //If you give a wait time, you can check the return value for success or failure of the command, however
                //the wait time version is blocking. 500ms is usually a safe value to ensure you wait long enough for any response.
                //But with no wait time, the command is simply sent and you have to monitor the device response for success or failure.

                //To setup channels for communication there are three mandatory operations assign, setID, and Open
                //Various other settings such as message period and network key affect communication
                //between two channels as well, see the documentation for further details on these functions.

                //So, first we assign the channel, we have already been passed the channelType which is an enum that has various flags
                //If we were doing something more advanced we could use a bitwise or ie:base|adv1|adv2 here too
                //We also use net 0 which has the public network key by default
                if (channel.assignChannel(channelType, 0, 500))
                    Console.WriteLine("Ch assigned to " + channelType + " on net 0.");
                else
                    throw new Exception("Channel assignment operation failed.");

                //Next we have to set the channel id. Slaves will only communicate with a master device that
                //has the same id unless one or more of the id parameters are set to a wild card 0. If wild cards are included
                //the slave will search until it finds a broadcast that matches all the non-wild card parameters in the id.
                //The pairing bit ensures on a search that you only pair with devices that also are requesting
                //pairing, but we don't need it here so we set it to false
                if (channel.setChannelID(deviceNumber, false, deviceType, transmissionType, 500))
                    Console.WriteLine("Set channel ID to " + deviceNumber + ", " + deviceType + ", " + transmissionType);
                else
                    throw new Exception("Set channel ID operation failed.");

                if (channel.setChannelFreq(rfFrequency, 500))
                    Console.WriteLine("RF frequency set to " + (rfFrequency + 2400));

                //Setting the channel period isn't mandatory, but we set it slower than the default period so messages aren't coming so fast
                //The period parameter is divided by 32768 to set the period of a message in seconds. So here, 32768/32768 = 1 sec/msg = 1Hz
                if (channel.setChannelPeriod(channelPeriod, 500))
                    Console.WriteLine("Message rate set to 4Hz");

                //Now we open the channel
                if (channel.openChannel(500))
                    Console.WriteLine("Opened channel");
                else
                    throw new Exception("Channel open operation failed.");
            }
            catch (Exception ex)
            {
                throw new Exception("Setup and open failed. " + ex.Message + Environment.NewLine);
            }
        }

        //Print the device response to the textbox
        void deviceResponse(ANT_Response response)
        {
            threadSafePrintLine(decodeDeviceFeedback(response), textBox_slave);
        }

        //Print the channel response to the textbox
        void channelResponse(ANT_Response response)
        {
            threadSafePrintLine(decodeChannelFeedback(response), textBox_slave);
        }

        //This function decodes the message code into human readable form, shows the error value on failures, and also shows the raw message contents
        String decodeDeviceFeedback(ANT_Response response)
        {
            string toDisplay = "Device: ";

            //The ANTReferenceLibrary contains all the message and event codes in user-friendly enums
            //This allows for more readable code and easy conversion to human readable strings for displays

            // So, for the device response we first check if it is an event, then we check if it failed,
            // and display the failure if that is the case. "Events" use message code 0x40.
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
            {
                //We cast the byte to its messageID string and add the channel number byte associated with the message
                toDisplay += (ANT_ReferenceLibrary.ANTMessageID)response.messageContents[1] + ", Ch:" + response.messageContents[0];
                //Check if the eventID shows an error, if it does, show the error message
                if ((ANT_ReferenceLibrary.ANTEventID)response.messageContents[2] != ANT_ReferenceLibrary.ANTEventID.RESPONSE_NO_ERROR_0x00)
                    toDisplay += Environment.NewLine + ((ANT_ReferenceLibrary.ANTEventID)response.messageContents[2]).ToString();
            }
            else   //If the message is not an event, we just show the messageID
                toDisplay += ((ANT_ReferenceLibrary.ANTMessageID)response.responseID).ToString();

            //Finally we display the raw byte contents of the response, converting it to hex
            toDisplay += Environment.NewLine + "::" + Convert.ToString(response.responseID, 16) + ", " + BitConverter.ToString(response.messageContents) + Environment.NewLine;
            return toDisplay;
        }

        String decodeChannelFeedback(ANT_Response response)
        {
            //Decoding channel events is an important consideration when building applications.
            //The channel events will be where broadcast, ack and burst messages are received, you
            //are also notified of messages sent and, for acknowledged and burst messages, their success or failure.
            //In a burst transfer, or if the device is operating at a small message period, then it is important that
            //decoding the messages is done quickly and efficiently.
            //The way the response retrieval works, messages should never be lost. If you decode messages too slow, they are simply queued.
            //However, you will be retrieving messages at a longer and longer delay from when they were actually received by the device.

            //One further consideration when handling channel events is to remember that the feedback functions will be triggered on a different thread
            //which is operating within the managed library to retrieve the feedback from the device. So, if you need to interact with your UI objects
            //you will need to use a Dispatcher invoke with a prority that will not make the UI unresponsive (see the threadSafePrint function used here).

            StringBuilder stringToPrint;    //We use a stringbuilder for speed and better memory usage, but, it doesn't really matter for the demo.
            stringToPrint = new StringBuilder("Channel: ", 100); //Begin the string and allocate some more space

            // Get device number of the connected master device
            ANT_ChannelID channelId = channel.requestID(100);
            MasterDeviceNumber = channelId.deviceNumber;

            //In the channel feedback we will get either RESPONSE_EVENTs or receive events,
            //If it is a response event we display what the event was and the error code if it failed.
            //Mostly, these response_events will all be broadcast events from a Master channel.
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
                stringToPrint.AppendLine(((ANT_ReferenceLibrary.ANTEventID)response.messageContents[2]).ToString());
            else   //This is a receive event, so display the ID
            {
                stringToPrint.AppendLine("Received " + ((ANT_ReferenceLibrary.ANTMessageID)response.responseID).ToString());

                byte[] rxBuffer = response.getDataPayload();

                // If the payload has a known data page number, decode it
                if (rxBuffer[0] == NodeStatusPage.DataPageNumber)
                {
                    NodeStatusPage.Decode(rxBuffer);

                    switch (NodeStatusPage.ConfigStatus)
                    {
                        case NodeStatusPage.ConfigurationStatus.Unconfigured:
                            {
                                nodeRequiresConfiguration = true;

                                ConfigurationCommandPage.NodeId = nextNodeId;
                                ConfigurationCommandPage.NetworkId = networkId;

                                byte[] txBuffer = new byte[8];
                                ConfigurationCommandPage.Encode(txBuffer);
                                channel.sendAcknowledgedData(txBuffer);
                            }
                            break;
                        case NodeStatusPage.ConfigurationStatus.Configured:
                            {


                                if (nodeRequiresConfiguration)
                                {
                                    nodeRequiresConfiguration = false;
                                    if (nextNodeId < maxNodeId) nextNodeId++;
                                    else throw new Exception("Maximum node ID (512) reached. Cannot configure another node.");
                                }
                            }
                            break;
                        default:
                            // do nothing
                            break;
                    }
                }
            }

            //Always print the raw contents in hex, with leading '::' for easy visibility/parsing
            //If this is a receive event it will contain the payload of the message
            stringToPrint.Append("  :: ");
            stringToPrint.Append(Convert.ToString(response.responseID, 16));
            stringToPrint.Append(", ");
            stringToPrint.Append(BitConverter.ToString(response.messageContents) + Environment.NewLine);
            return stringToPrint.ToString();
        }

        void threadSafePrintLine(String stringToPrint, TextBox boxToPrintTo)
        {
            //We need to put this on the dispatcher because sometimes it is called by the feedback thread
            //If you set the priority to 'background' then it never interferes with the UI interaction if you have a high message rate (we don't have to worry about it in the demo)
            boxToPrintTo.Dispatcher.BeginInvoke(new dAppendText(boxToPrintTo.AppendText), System.Windows.Threading.DispatcherPriority.Background, stringToPrint + Environment.NewLine);
            boxToPrintTo.Dispatcher.BeginInvoke(new dScrollToEnd(boxToPrintTo.ScrollToEnd), System.Windows.Threading.DispatcherPriority.Background);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }
}
