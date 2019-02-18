using ANT_Managed_Library;
using MissMooseConfigurationApplication.UIPages;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Windows;
using System.Windows.Media;

namespace MissMooseConfigurationApplication
{
    class AntControl
    {
        #region Public Members
		
        public static AntControl Instance = new AntControl();
		
        #endregion
		
        #region Private Members

        private ChannelManager channelManager = new ChannelManager();
        private PageHandler pageHandler = new PageHandler();
        private PageParser pageParser = new PageParser();

        #endregion

        #region Public Methods

        public AntControl()
        {
            // Set up the page parser to decode valid data pages
            pageParser.AddDataPage(new NodeStatusPage());
            pageParser.AddDataPage(new LidarMonitoringPage());
            pageParser.AddDataPage(new PirMonitoringPage());
            pageParser.AddDataPage(new LedOutputStatusPage());
        }

        public void AddConfigUI(ConfigurationPage ConfigUI)
        {
            pageHandler.AddConfigUI(ConfigUI);

            // Open the ANT slave channel to scan for nodes
            channelManager.StartConfigDevice();
            channelManager.channel.channelResponse += ChannelResponse;
        }
		
        public void AddMonitoringUI(SystemOverviewPage MonitoringUI)
        {
            pageHandler.AddMonitoringUI(MonitoringUI);
        }

        #endregion

        #region Private Methods

        /*
         * Handles the ANT channel response for the node scanning channel
         */
        private void ChannelResponse(ANT_Response response)
        {
            //This is a receive event, so handle the received ANT message
            if (response.responseID != (byte)ANT_ReferenceLibrary.ANTMessageID.RESPONSE_EVENT_0x40)
            {
                // Check if this is an extended message
                if (response.isExtended())
                {
                    // Get device number of the master device
                    ANT_ChannelID channelId = response.getDeviceIDfromExt();

                    byte[] rxBuffer = response.getDataPayload();

                    dynamic dataPage = pageParser.Parse(rxBuffer);

                    if (dataPage != null)
                    {
                        pageHandler.HandlePage(dataPage, channelId.deviceNumber, 
                            new PageSender(channelManager.GetChannel(channelId.deviceNumber), channelId));
                    }
                }
            }
        }
       
        #endregion
    }
}
