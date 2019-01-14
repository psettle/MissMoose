using MissMooseConfigurationApplication.UIPages;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    class AntControl
    {
        public static AntControl Instance = new AntControl();

        private ConfigurationPage ConfigUI;
        private SystemOverviewPage MonitoringUI;

        public AntControl()
        {
            
        }

        public void AddConfigUI(ConfigurationPage ConfigUI)
        {
            this.ConfigUI = ConfigUI;

            //demo: add existing node
            var node = new SensorNode(HardwareConfiguration.PirLidarLed, 2);
            node.xpos = 1;
            node.ypos = 1;
            node.Rotation = new NodeRotation(NodeRotation.R90);
            node.xoffset = -3;
            node.yoffset = 3;

            ConfigUI.AddExistingNode(node);


            //demo: add new node
            ConfigUI.AddNewNode(new SensorNode(HardwareConfiguration.Pir2, 3));


            //demo: read input state
            var nodes = ConfigUI.GetCurrentNodes();
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
    }
}
