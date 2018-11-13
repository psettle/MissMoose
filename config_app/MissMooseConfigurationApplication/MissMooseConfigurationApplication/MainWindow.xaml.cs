using ANT_Managed_Library;
using System;
using System.Collections.Generic;
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
    public partial class MainWindow : Window
    {
        //This delegate is for using the dispatcher to avoid conflicts of the feedback thread referencing the UI elements
        delegate void dAppendText(String toAppend);

        // devices
        ANT_Device master;
        ANT_Device slave;

        Boolean master_running = false;
        Boolean slave_running = false;

        public MainWindow()
        {
            InitializeComponent();
        }

        public void Start_Master(object sender, RoutedEventArgs e)
        {
            if (!master_running)
            {
                if (ANT_Master_Start())
                {
                    master_running = true;
                    button_master.Background = System.Windows.Media.Brushes.Salmon;
                    button_master.Content = "Stop Master";
                    return;
                }
            }
            //If we get here it means we failed startup or are stopping the demo
            Shutdown_Master();
            master_running = false;
            button_master.Background = System.Windows.Media.Brushes.LightGreen;
            button_master.Content = "Start Master";
        }

        public Boolean ANT_Master_Start()
        {
            try
            {
                master = new ANT_Device();
                master.deviceResponse += new ANT_Device.dDeviceResponseHandler(master_deviceResponse);
                master.getChannel(0).channelResponse += new dChannelResponseHandler(slave_channelResponse);

                setupAndOpen(master, ANT_ReferenceLibrary.ChannelType.BASE_Master_Transmit_0x10);
            }
            catch(Exception e)
            {
                Console.WriteLine("Can't start master: " + e.Message);
                return false;
            }
            return true;
        }

        public void Shutdown_Master()
        {
            //If you need to manually disconnect from a ANTDevice call the shutdownDeviceInstance method,
            //It releases all the resources, the device, and nullifies the object reference
            ANT_Device.shutdownDeviceInstance(ref master);
        }

        public void Start_Slave(object sender, RoutedEventArgs e)
        {
            if (!slave_running)
            {
                if (ANT_Slave_Start())
                {
                    slave_running = true;
                    button_slave.Background = System.Windows.Media.Brushes.Salmon;
                    button_slave.Content = "Stop Slave";
                    return;
                }
            }
            //If we get here it means we failed startup or are stopping the demo
            Shutdown_Slave();
            master_running = false;
            button_slave.Background = System.Windows.Media.Brushes.LightGreen;
            button_slave.Content = "Start Slave";
        }

        public Boolean ANT_Slave_Start()
        {
            try
            {
                slave = new ANT_Device();
                slave.deviceResponse += new ANT_Device.dDeviceResponseHandler(slave_deviceResponse);
                slave.getChannel(0).channelResponse += new dChannelResponseHandler(slave_channelResponse);

                setupAndOpen(slave, ANT_ReferenceLibrary.ChannelType.BASE_Slave_Receive_0x00);
            }
            catch (Exception e)
            {
                Console.WriteLine("Can't start slave: " + e.Message);
                return false;
            }
            return true;
        }

        public void Shutdown_Slave()
        {
            //If you need to manually disconnect from a ANTDevice call the shutdownDeviceInstance method,
            //It releases all the resources, the device, and nullifies the object reference
            ANT_Device.shutdownDeviceInstance(ref slave);
        }

        private void setupAndOpen(ANT_Device deviceToSetup, ANT_ReferenceLibrary.ChannelType channelType)
        {
            //We try-catch and forward exceptions to the calling function to handle and pass the errors to the user
            try
            {
                //To access an ANTChannel on a paticular device we need to get the channel from the device
                //Once again, this ensures you have a valid object associated with a real-world ANTChannel
                //ie: You can only get channels that actually exist
                ANT_Channel channel0 = deviceToSetup.getChannel(0);

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
                if (channel0.assignChannel(channelType, 0, 500))
                    Console.WriteLine("Ch assigned to " + channelType + " on net 0.");
                else
                    throw new Exception("Channel assignment operation failed.");

                //Next we have to set the channel id. Slaves will only communicate with a master device that
                //has the same id unless one or more of the id parameters are set to a wild card 0. If wild cards are included
                //the slave will search until it finds a broadcast that matches all the non-wild card parameters in the id.
                //The pairing bit ensures on a search that you only pair with devices that also are requesting
                //pairing, but we don't need it here so we set it to false
                if (channel0.setChannelID(0, false, 0, 0, 500))
                    Console.WriteLine("Set channel ID to 0, 0, 0");
                else
                    throw new Exception("Set Channel ID operation failed.");

                //Setting the channel period isn't mandatory, but we set it slower than the default period so messages aren't coming so fast
                //The period parameter is divided by 32768 to set the period of a message in seconds. So here, 32768/32768 = 1 sec/msg = 1Hz
                if (channel0.setChannelPeriod(32768, 500))
                    Console.WriteLine("Message Period set to 1 second per message");

                //Now we open the channel
                if (channel0.openChannel(500))
                    Console.WriteLine("Opened Channel");
                else
                    throw new Exception("Channel Open operation failed.");
            }
            catch (Exception ex)
            {
                throw new Exception("Setup and Open Failed. " + ex.Message + Environment.NewLine);
            }
        }

        //Print the device response to the textbox
        void master_deviceResponse(ANT_Response response)
        {
            threadSafePrintLine(decodeDeviceFeedback(response), textBox_master);
        }

        //Print the channel response to the textbox
        void master_channelResponse(ANT_Response response)
        {
            threadSafePrintLine(decodeChannelFeedback(response), textBox_master);
        }

        //Print the device response to the textbox
        void slave_deviceResponse(ANT_Response response)
        {
            threadSafePrintLine(decodeDeviceFeedback(response), textBox_slave);
        }

        //Print the channel response to the textbox
        void slave_channelResponse(ANT_Response response)
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

            //In the channel feedback we will get either RESPONSE_EVENTs or receive events,
            //If it is a response event we display what the event was and the error code if it failed.
            //Mostly, these response_events will all be broadcast events from a Master channel.
            if (response.responseID == (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
                stringToPrint.AppendLine(((ANT_ReferenceLibrary.ANTEventID)response.messageContents[2]).ToString());
            else   //This is a receive event, so display the ID
                stringToPrint.AppendLine("Received " + ((ANT_ReferenceLibrary.ANTMessageID)response.responseID).ToString());

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
        }
    }
}
