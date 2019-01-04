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
        private ConfigurationPage ConfigUI;

        public AntControl(ConfigurationPage ConfigUI)
        {
            this.ConfigUI = ConfigUI;

            //demo add existing node
            var node = new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS, 2);
            node.SetPosition(1, 1);
            node.SetRotation(NodeRotation.NODEROTATION_135);
            node.SetOffset(-3, 3);

            ConfigUI.AddExistingNode(node);


            //demo add new node
            ConfigUI.AddNewNode(new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_2_PIR, 3));


            //demo read input state
            var nodes = ConfigUI.GetCurrentNodes();
        }
    }
}
