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
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MissMooseConfigurationApplication
{
    public enum HardwareConfiguration
    {
        Unknown,
        Pir2,
        PirLidar,
        PirLidarLed,
    }

    public enum SensorType : byte
    {
        Pir,
        Lidar,
    }

    public enum LedFunction : byte
    {
        Off,
        Blinking,
        Continuous,
    }

    public enum LedColourEnum : byte
    {
        Yellow,
        Red,
    }

    public class LedColour
    {
        public static readonly SolidColorBrush Red = (SolidColorBrush)Application.Current.FindResource("ThemeBrushRed");
        public static readonly SolidColorBrush Yellow = (SolidColorBrush)Application.Current.FindResource("ThemeBrushYellow");
        public static readonly SolidColorBrush Blue = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForegroundBlue");
        public static readonly SolidColorBrush Disabled = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground");
    }

    public class StatusColour
    {
        public static readonly SolidColorBrush Red = (SolidColorBrush)Application.Current.FindResource("ThemeBrushRed");
        public static readonly SolidColorBrush Yellow = (SolidColorBrush)Application.Current.FindResource("ThemeBrushYellow");
        public static readonly SolidColorBrush Blue = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForegroundBlue");
        public static readonly SolidColorBrush Disabled = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground");
    }

    public class Rotation
    {
        public double Val { get; private set; }

        public Rotation(double val)
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

        public void Add(Rotation val)
        {
            Add(val.Val);
        }

        public int ToEnum()
        {
            double range = R45 / 2.0;

            if (Val < R0 + range)
            {
                return 0;
            }

            if (Val < R45 + range)
            {
                return 1;
            }

            if (Val < R90 + range)
            {
                return 2;
            }

            if (Val < R135 + range)
            {
                return 3;
            }

            if (Val < R180 + range)
            {
                return 4;
            }

            if (Val < R225 + range)
            {
                return 5;
            }

            if (Val < R270 + range)
            {
                return 6;
            }

            return 7;
        }

        internal void FromEnum(byte e)
        {
            switch (e)
            {
            case 0:
                Val = R0;
                break;
            case 1:
                Val = R45;
                break;
            case 2:
                Val = R90;
                break;
            case 3:
                Val = R135;
                break;
            case 4:
                Val = R180;
                break;
            case 5:
                Val = R225;
                break;
            case 6:
                Val = R270;
                break;
            case 7:
                Val = R315;
                break;
            };
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
        public const int OffsetScale = 10;

        public ushort DeviceNumber { get; }

        public string DisplayID {
            get
            {
                return displayID;
            }
            private set
            {
                displayID = value;
                NodeIDLabel.Content = value;
            }
        }

        public ushort NodeID { get; }
        
        public Rotation Rotation
        {
            get { return rotation; }
            set
            {
                rotation = value;
                ApplyVisualRotation();
            }
        }

        public sbyte xpos
        {
            get
            {
                return x;
            }
            set
            {
                x = value;
                SetDisplayID();
            }
        }

        public sbyte ypos
        {
            get
            {
                return y;
            }
            set
            {
                y = value;
                SetDisplayID();
            }
        }

        public sbyte xoffset { get; set; } = 0;
        public sbyte yoffset { get; set; } = 0;
        public HardwareConfiguration configuration { get; private set; }

        public bool isgateway
        {
            get
            {
                return isGateway;
            }
            set
            {
                isGateway = value;
                NodeGatewayLabel.Visibility = isGateway ? Visibility.Visible : Visibility.Hidden;
            }
        }

        #endregion

        #region Private Members

        private string displayID = "";
        private sbyte x = -1;
        private sbyte y = -1;
        private LedFunction ledFunction;
        private Brush ledColour;
        private Brush statusColour;
        private bool isGateway;
        private Rotation rotation;

        #endregion

        #region Public Methods

        public SensorNode(ushort deviceNumber, HardwareConfiguration configuration, ushort NodeID, bool IsGateway)
        {
            InitializeComponent();

            this.DeviceNumber = deviceNumber;
            this.configuration = configuration;
            this.NodeID = NodeID;
            this.Rotation = new Rotation(Rotation.R0);
            isgateway = IsGateway;

            switch (configuration)
            {
                case HardwareConfiguration.Pir2:
                    PIR_0_Deg.Visibility = Visibility.Visible;
                    PIR_90_Deg.Visibility = Visibility.Hidden;
                    PIR_270_Deg.Visibility = Visibility.Visible;
                    Lidar_0_Deg.Visibility = Visibility.Hidden;
                    break;
                case HardwareConfiguration.PirLidar:
                case HardwareConfiguration.PirLidarLed:
                    PIR_0_Deg.Visibility = Visibility.Hidden;
                    PIR_90_Deg.Visibility = Visibility.Visible;
                    PIR_270_Deg.Visibility = Visibility.Hidden;
                    Lidar_0_Deg.Visibility = Visibility.Visible;
                    if (configuration == HardwareConfiguration.PirLidarLed)
                    {
                        InnerRingBorder.Fill = Brushes.Black;
                    }
                    break;
            }

            SetStatusColour(StatusColour.Blue);
            SetLedFunction(LedFunction.Continuous);
            SetLedColour(LedColour.Red);
        }

        public bool SetLedFunction(LedFunction ledFunction)
        {
            if (this.ledFunction != ledFunction)
            {
                this.ledFunction = ledFunction;

                Storyboard sb = (Storyboard)this.FindResource("Blink");
                if (ledFunction == LedFunction.Blinking)
                {
                    sb.Begin();
                }
                else
                {
                    sb.Stop();
                }

                if (ledFunction == LedFunction.Off)
                {
                    SetLedColour(LedColour.Blue);
                }

                // The function was changed
                return true;
            }

            // The function was not changed
            return false;
        }

        public bool SetLedColour(Brush colour)
        {
            if(configuration == HardwareConfiguration.PirLidarLed
                && colour != this.ledColour
                && (ledFunction != LedFunction.Off || colour == LedColour.Blue))
            {
                ledColour = colour;
                OuterRing.Fill = colour;

                // The colour was changed
                return true;
            }

            // The colour was not changed
            return false;
        }

        public bool SetLedColour(LedColourEnum colour)
        {
            switch (colour)
            {
                case LedColourEnum.Yellow:
                    return SetLedColour(LedColour.Yellow);
                case LedColourEnum.Red:
                    return SetLedColour(LedColour.Red);
                default:
                    // Do nothing
                    return false;
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
            var node = new SensorNode(DeviceNumber, configuration, NodeID, isgateway)
            {
                xoffset = xoffset,
                yoffset = yoffset,
                xpos = xpos,
                ypos = ypos,
                Rotation = new Rotation(Rotation.Val)
            };
            node.SetLedFunction(ledFunction);
            node.SetLedColour(ledColour);
            node.SetStatusColour(statusColour);

            if (xpos == 0)
            {
                SetLedColour(LedColour.Yellow);
            }

            return node;
        }

        /// <summary>
        /// Change to a set of visual properties that make the node look active
        /// </summary>
        public void UseActivePalette()
        {
            if (configuration != HardwareConfiguration.PirLidarLed)
            {
                InnerRing.Fill = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground");
                InnerRingBorder.Fill = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground");
            }
            InnerCircle.Fill = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground");

            Effect = new DropShadowEffect
            {
                Direction = 75 + rotation.Val,
                ShadowDepth = 5,
                BlurRadius = 15

            };
            NodeIDLabel.Foreground = (SolidColorBrush)Application.Current.FindResource("ThemeBrushText");
            NodeIDLabel.Effect = new DropShadowEffect
            {
                Direction = 75,
                ShadowDepth = 1,
                BlurRadius = 15
            };
            NodeGatewayLabel.Foreground = (SolidColorBrush)Application.Current.FindResource("ThemeBrushText");
            NodeGatewayLabel.Effect = new DropShadowEffect
            {
                Direction = 75,
                ShadowDepth = 1,
                BlurRadius = 15
            };
        }

        /// <summary>
        /// Change to a set of visual properties that make the node look inactive
        /// </summary>
        public void UseInactivePalette()
        {
            if (configuration != HardwareConfiguration.PirLidarLed)
            {
                InnerRing.Fill = (SolidColorBrush)Application.Current.FindResource("ThemeBrushBackground");
                InnerRingBorder.Fill = (SolidColorBrush)Application.Current.FindResource("ThemeBrushBackground");
            }
            InnerCircle.Fill = (SolidColorBrush)Application.Current.FindResource("ThemeBrushBackground");
            Effect = null;
            NodeIDLabel.Foreground = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground");
            NodeGatewayLabel.Foreground = (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground");
            NodeIDLabel.Effect = null;
            NodeGatewayLabel.Effect = null;
        }

        #endregion

        #region Private Methods

        private void ApplyVisualRotation()
        {
            /* Set shadow rotation */
            if(Effect != null)
            {
                UseActivePalette();
            }

            NodeIDLabel.RenderTransform = new RotateTransform()
            {
                Angle = -rotation.Val,
                CenterX = 225,
                CenterY = 225
            };

            /* Set text rotation */
            NodeGatewayLabel.RenderTransform = new RotateTransform()
            {
                Angle = -rotation.Val,
                CenterX = 225,
                CenterY = 40
            };
        }
        
        private void SetDisplayID()
        {
            DisplayID = (char)('A' + y) + (x + 1).ToString();
        }

        #endregion
    }
}
