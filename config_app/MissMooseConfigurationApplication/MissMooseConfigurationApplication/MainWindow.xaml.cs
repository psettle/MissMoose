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
using MissMooseConfigurationApplication.UIPages;
using System.Xml;
using System.Xml.Serialization;
using System.IO;

namespace MissMooseConfigurationApplication
{
    enum SystemStatusEnum
    {
        SystemStatus_OK
    }

    public partial class MainWindow : Window
    {
        #region Private Members

        private Dictionary<PageSwitcherButton, Page> navigationItems;
        private string ConfigurationSaveFileName = "SavedConfiguration.xml";
        private string ConfigurationRootXmlElement = "Configuration";

        #endregion

        #region Public Methods

        public MainWindow()
        {
            InitializeComponent();

            SetSystemStatusLabel(SystemStatusEnum.SystemStatus_OK);

            // main window owns all these pages so that state is maintained when switching between them
            navigationItems = new Dictionary<PageSwitcherButton, Page>()
            {
                { ConfigPageButton, new ConfigurationPage() },
                { EventLogPageButton, new EventLogPage() },
                { SystemProblemsPageButton, new SystemProblemsPage()},
                { SystemOverviewPageButton, new SystemOverviewPage()}
            };
            LoadConfiguration();
            // app starts on config screen
            PageSwitchClick(ConfigPageButton);
        }

        public void PageSwitchClick(PageSwitcherButton sender)
        {
            foreach (KeyValuePair<PageSwitcherButton, Page> item in navigationItems)
            {
                if (item.Key.Name.Contains(sender.Name))
                {
                    item.Key.SetButtonColour(true);
                    PageFrame.Navigate(item.Value);
                }
                else
                {
                    item.Key.SetButtonColour(false);
                }
            }
        }

        #endregion

        #region Private Methods

        void SetSystemStatusLabel(SystemStatusEnum status)
        {
            string content = "System Status: ";

            switch (status)
            {
                case SystemStatusEnum.SystemStatus_OK:
                    content += "OK";
                    break;
            }

            label_SystemStatusBar.Content = content;
        }

        /// <summary>
        /// Container class used to convert SensorNode information into a serializable format
        /// </summary>
        public class NodeAttributes
        {
            [XmlAttribute]
            public int NodeID;
            [XmlAttribute]
            public int xpos;
            [XmlAttribute]
            public int ypos;
            [XmlAttribute]
            public int xoffset;
            [XmlAttribute]
            public int yoffset;
            [XmlAttribute]
            public double rotation;
            [XmlAttribute]
            public HardwareConfiguration configuration;
        }

        public void SaveConfiguration(object sender, EventArgs e)
        {
            XmlDocument xmlDocument = new XmlDocument();
            XmlSerializer serializer = new XmlSerializer(typeof(NodeAttributes[]), new XmlRootAttribute() { ElementName = ConfigurationRootXmlElement });
            using (MemoryStream stream = new MemoryStream())
            {
                List<NodeAttributes> nodes = new List<NodeAttributes>();
                foreach(SensorNode node in ((ConfigurationPage)navigationItems[ConfigPageButton]).nodes)
                {
                    nodes.Add(new NodeAttributes {
                        NodeID = node.NodeID,
                        xpos = node.xpos,
                        ypos = node.ypos,
                        xoffset = node.xoffset,
                        yoffset = node.yoffset,
                        rotation = node.Rotation.Val,
                        configuration = node.configuration });
                }
                serializer.Serialize(stream, nodes.ToArray());
                stream.Position = 0;
                xmlDocument.Load(stream);
                xmlDocument.Save(ConfigurationSaveFileName);
            }
        }

        public void LoadConfiguration()
        {
            XmlSerializer serializer = new XmlSerializer(typeof(NodeAttributes[]), new XmlRootAttribute() { ElementName = ConfigurationRootXmlElement });
            using (FileStream stream = new FileStream(ConfigurationSaveFileName, FileMode.Open))
            {
                NodeAttributes[] loadednodes = ((NodeAttributes[])serializer.Deserialize(stream));
                foreach(NodeAttributes node in loadednodes)
                {
                    SensorNode sensornode = new SensorNode(node.configuration, node.NodeID);
                    sensornode.xpos = node.xpos;
                    sensornode.ypos = node.ypos;
                    sensornode.xoffset = node.xoffset;
                    sensornode.yoffset = node.yoffset;
                    sensornode.Rotation = new NodeRotation(node.rotation);
                    // this is tested and working, commented out since application in still in a demo state
                    //((ConfigurationPage)navigationItems[ConfigPageButton]).AddExistingNode(sensornode);
                }
            }
        }

        #endregion
    }
}
