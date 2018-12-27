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
    public enum HardwareConfiguration
    {
        HARDWARECONFIGURATION_2_PIR,
        HARDWARECONFIGURATION_1_PIR_1_LIDAR,
        HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS,
    }

    public enum LedColour
    {
        LEDCOLOUR_RED,
        LEDCOLOUR_YELLOW,
        LEDCOLOUR_GREEN
    }

    public enum StatusColour
    {
        STATUSCOLOUR_RED,
        STATUSCOLOUR_YELLOW,
        STATUSCOLOUR_BLUE
    }

    public enum NodeRotation
    {
        NODEROTATION_0,
        NODEROTATION_45,
        NODEROTATION_90,
        NODEROTATION_135,
        NODEROTATION_180,
        NODEROTATION_225,
        NODEROTATION_270,
        NODEROTATION_315
    }

    /// <summary>
    /// Interaction logic for SensorNode.xaml
    /// </summary>
    public partial class SensorNode : UserControl
    {
        internal NodeRotation rotation = NodeRotation.NODEROTATION_0;
        HardwareConfiguration configuration;
        internal int NodeID;

        public SensorNode(HardwareConfiguration configuration, int NodeID)
        {
            InitializeComponent();

            this.configuration = configuration;
            this.NodeID = NodeID;

            switch(configuration)
            {
                case HardwareConfiguration.HARDWARECONFIGURATION_2_PIR:
                    PIR_0_Deg.Visibility = Visibility.Visible;
                    PIR_270_Deg.Visibility = Visibility.Visible;
                    Lidar_270_Deg.Visibility = Visibility.Hidden;
                    break;
                case HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR:
                case HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS:
                    PIR_0_Deg.Visibility = Visibility.Visible;
                    PIR_270_Deg.Visibility = Visibility.Hidden;
                    Lidar_270_Deg.Visibility = Visibility.Visible;
                    break;
            }

            SetStatusColour(StatusColour.STATUSCOLOUR_BLUE);
            SetLedColour(LedColour.LEDCOLOUR_GREEN);
        }

        public void SetLedColour(LedColour colour)
        {
            if(configuration == HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS)
            {
                switch(colour)
                {
                    case LedColour.LEDCOLOUR_GREEN:
                        OuterRing.Fill = Brushes.Green;
                        break;
                    case LedColour.LEDCOLOUR_RED:
                        OuterRing.Fill = Brushes.Red;
                        break;
                    case LedColour.LEDCOLOUR_YELLOW:
                        OuterRing.Fill = Brushes.Yellow;
                        break;
                }
            }
        }

        public void SetStatusColour(StatusColour colour)
        {
            SolidColorBrush finalColour;

            switch(colour)
            {
                case StatusColour.STATUSCOLOUR_BLUE:
                    finalColour = Brushes.Blue;
                    break;
                case StatusColour.STATUSCOLOUR_YELLOW:
                    finalColour = Brushes.Yellow;
                    break;
                case StatusColour.STATUSCOLOUR_RED:
                    finalColour = Brushes.Red;
                    break;
                default:
                    throw new InvalidOperationException("Unknown status colour");
            }

            switch(configuration)
            {
                case HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR:
                    OuterRing.Fill = finalColour;
                    break;
                case HardwareConfiguration.HARDWARECONFIGURATION_2_PIR:
                    OuterRing.Fill = finalColour;
                    break;
                case HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS:
                    InnerRing.Fill = finalColour;
                    break;
                default:
                    throw new InvalidOperationException("Unknown hardware configuration");
            }
        }

        public void SetRotation(NodeRotation rotation)
        {
            this.rotation = rotation;
        }

    }
}
