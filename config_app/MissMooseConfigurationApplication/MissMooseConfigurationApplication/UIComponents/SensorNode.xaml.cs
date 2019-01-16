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
        Pir2,
        PirLidar,
        PirLidarLed,
    }

    public class LedColour
    {
        public static readonly Brush Red = Brushes.Red;
        public static readonly Brush Yellow = Brushes.Yellow;
        public static readonly Brush Green = Brushes.Green;
    }

    public class StatusColour
    {
        public static readonly Brush Red = Brushes.Red;
        public static readonly Brush Yellow = Brushes.Yellow;
        public static readonly Brush Blue = Brushes.Blue;
    }

    public class NodeRotation
    {
        public double Val { get; private set; }

        public NodeRotation(double val)
        {
            Val = val;
        }

        public void Add(double val)
        {
            Val += val;
            while(Val < 0.0)
            {
                Val += R360;
            }

            while(Val >= 360)
            {
                Val -= R360;
            }
        }

        public void Add(NodeRotation val)
        {
            Add(val.Val);
        }

        public static readonly double R0 = 0.0;
        public static readonly double R45 = 45.0;
        public static readonly double R90 = 90.0;
        public static readonly double R135 = 135.0;
        public static readonly double R180 = 180.0;
        public static readonly double R225 = 225.0;
        public static readonly double R270 = 270.0;
        public static readonly double R315 = 315.0;
        private static readonly double R360 = 360.0;
    }

    public partial class SensorNode : UserControl
    {
        #region Public Members
        public const int MaxOffset = 5;

        public int NodeID { get; set; }

        public NodeRotation Rotation { get; set; } = new NodeRotation(NodeRotation.R0);

        public int xpos { get; set; } = -1;
        public int ypos { get; set; } = -1;

        public int xoffset { get; set; } = 0;
        public int yoffset { get; set; } = 0;
        #endregion

        #region Private Members
        private HardwareConfiguration configuration;


        private Brush ledColour;
        private Brush statusColour;
        #endregion

        #region Public Methods

        public SensorNode(HardwareConfiguration configuration, int NodeID)
        {
            InitializeComponent();

            this.configuration = configuration;
            this.NodeID = NodeID;
            Name.Content = NodeID;

            switch(configuration)
            {
                case HardwareConfiguration.Pir2:
                    PIR_0_Deg.Visibility = Visibility.Visible;
                    PIR_270_Deg.Visibility = Visibility.Visible;
                    Lidar_270_Deg.Visibility = Visibility.Hidden;
                    break;
                case HardwareConfiguration.PirLidar:
                case HardwareConfiguration.PirLidarLed:
                    PIR_0_Deg.Visibility = Visibility.Visible;
                    PIR_270_Deg.Visibility = Visibility.Hidden;
                    Lidar_270_Deg.Visibility = Visibility.Visible;
                    break;
            }

            SetStatusColour(StatusColour.Blue);
            SetLedColour(LedColour.Green);
        }

        public void SetLedColour(Brush colour)
        {
            if(configuration == HardwareConfiguration.PirLidarLed)
            {
                ledColour = colour;
                OuterRing.Fill = colour;
            }
        }

        public void SetStatusColour(Brush colour)
        {
            statusColour = colour;

            switch(configuration)
            {
                case HardwareConfiguration.Pir2:
                case HardwareConfiguration.PirLidar:
                    OuterRing.Fill = colour;
                    break;
                case HardwareConfiguration.PirLidarLed:
                    InnerRing.Fill = colour;
                    break;
                default:
                    throw new InvalidOperationException("Unknown hardware configuration");
            }
        }

        public SensorNode Clone()
        {
            var node = new SensorNode(configuration, NodeID)
            {
                xoffset = xoffset,
                yoffset = yoffset,
                xpos = xpos,
                ypos = ypos,
                Rotation = new NodeRotation(Rotation.Val)
            };
            node.SetLedColour(ledColour);
            node.SetStatusColour(statusColour);

            return node;
        }

        #endregion
    }
}
