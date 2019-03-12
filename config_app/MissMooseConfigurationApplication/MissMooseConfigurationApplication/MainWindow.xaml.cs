using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;
using MissMooseConfigurationApplication.UIPages;
using System.Xml;
using System.Xml.Serialization;
using System.IO;
using MissMooseConfigurationApplication.Utils;

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

        #region Public Events

        public delegate void UpdateThemeEventHandler();

        public event UpdateThemeEventHandler UpdateTheme;

        #endregion

        #region Public Methods

        public MainWindow()
        {
            InitializeComponent();

            // main window owns all these pages so that state is maintained when switching between them
            navigationItems = new Dictionary<PageSwitcherButton, Page>()
            {
                { ConfigPageButton, new ConfigurationPage() },
                //{ EventLogPageButton, new EventLogPage() },
                //{ SystemProblemsPageButton, new SystemProblemsPage()},
                { SystemOverviewPageButton, new SystemOverviewPage()}
            };
            if(File.Exists(ConfigurationSaveFileName))
            {
                LoadConfiguration();
            }
            // app starts on config screen
            PageSwitchClick(ConfigPageButton);

        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            IconHelper.RemoveIcon(this);
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

        /// <summary>
        /// Container class used to convert SensorNode information into a serializable format
        /// </summary>
        public class NodeAttributes
        {
            [XmlAttribute]
            public ushort DeviceNumber;
            [XmlAttribute]
            public ushort NodeID;
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
            [XmlAttribute]
            public bool isgateway;
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
                        DeviceNumber = node.DeviceNumber,
                        NodeID = node.NodeID,
                        xpos = node.xpos,
                        ypos = node.ypos,
                        xoffset = node.xoffset,
                        yoffset = node.yoffset,
                        rotation = node.Rotation.Val,
                        configuration = node.configuration,
                        isgateway = node.isgateway });
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
                    SensorNode sensornode = new SensorNode(node.DeviceNumber, node.configuration, node.NodeID, node.isgateway);
                    sensornode.xpos = (sbyte)node.xpos;
                    sensornode.ypos = (sbyte)node.ypos;
                    sensornode.xoffset = (sbyte)node.xoffset;
                    sensornode.yoffset = (sbyte)node.yoffset;
                    sensornode.Rotation = new Rotation(node.rotation);

                    ((ConfigurationPage)navigationItems[ConfigPageButton]).AddExistingNode(sensornode);
                    ((SystemOverviewPage)navigationItems[SystemOverviewPageButton]).UpdateNode(sensornode);
                }
            }
        }

        private void SwitchThemeButton_Click(object sender, RoutedEventArgs e)
        {
            CSharpControls.Wpf.ToggleSwitch toggleSwitch = (CSharpControls.Wpf.ToggleSwitch)sender;

            ((App)Application.Current).Resources.MergedDictionaries[0].Source =
                (bool)toggleSwitch.IsChecked ?
                new Uri($"/Themes/Dark.xaml", UriKind.Relative)
                :
                new Uri($"/Themes/Light.xaml", UriKind.Relative);

            UpdateTheme();
        }

        #endregion
    }
}
