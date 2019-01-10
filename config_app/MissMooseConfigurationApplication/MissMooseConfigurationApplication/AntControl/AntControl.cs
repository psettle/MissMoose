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
            var node = new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS, 2);
            node.SetPosition(1, 1);
            node.SetRotation(NodeRotation.NODEROTATION_135);
            node.SetOffset(-3, 3);

            ConfigUI.AddExistingNode(node);


            //demo: add new node
            ConfigUI.AddNewNode(new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_2_PIR, 3));


            //demo: read input state
            var nodes = ConfigUI.GetCurrentNodes();
        }

        public void AddMonitoringUI(SystemOverviewPage MonitoringUI)
        {
            this.MonitoringUI = MonitoringUI;

            //demo: add existing node
            var node = new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS, 2);
            node.SetPosition(1, 1);
            node.SetRotation(NodeRotation.NODEROTATION_135);
            node.SetOffset(-3, 3);

            MonitoringUI.AddNode(node);

            //demo: remove, modify, add node
            MonitoringUI.RemoveNode(node);
            node.SetOffset(1, 1);
            node.SetRotation(NodeRotation.NODEROTATION_180);
            MonitoringUI.AddNode(node);

            //demo: Display sensor data
            MonitoringUI.MarkSensorDetection(node, LineDirection.Down, StatusColour.STATUSCOLOUR_RED);

            //demo: add existing node
            node = new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_2_PIR, 3);
            node.SetPosition(1, 2);
            node.SetRotation(NodeRotation.NODEROTATION_135);
            node.SetOffset(-3, 3);

            MonitoringUI.AddNode(node);

            //demo: remove, modify, add node
            MonitoringUI.RemoveNode(node);
            node.SetOffset(0, 0);
            node.SetRotation(NodeRotation.NODEROTATION_0);
            MonitoringUI.AddNode(node);

            //demo: Display sensor data
            MonitoringUI.MarkSensorDetection(node, LineDirection.Up, StatusColour.STATUSCOLOUR_RED);
            MonitoringUI.MarkSensorDetection(node, LineDirection.Left, StatusColour.STATUSCOLOUR_YELLOW);
        }
    }
}
