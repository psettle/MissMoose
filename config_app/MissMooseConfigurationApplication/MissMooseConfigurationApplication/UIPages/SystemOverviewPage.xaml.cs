using MissMooseConfigurationApplication.UIPages;
using MissMooseConfigurationApplication.UIComponents;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
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
    public enum LineDirection
    {
        Up,
        Right,
        Down,
        Left
    }

    public partial class SystemOverviewPage : Page
    {
        #region Private Members
        private List<List<Viewbox>> sensorViewboxes;
        private List<SensorNode> nodes = new List<SensorNode>();
        private List<List<Dictionary<LineDirection, LineWithBorder>>> lineSegmentAssociations;
        private List<List<Path>> shadedRegions;
        private const int GridSize = 3;
        private const int OffsetScalePixels = 5;
        private const string timestampFormatString = "yyyy-MMM-dd hh:mm:ss tt";
        #endregion

        #region Public Methods
        public SystemOverviewPage()
        {
            InitializeComponent();
            InitializeViewboxes();
            InitializeLineSegments();
            InitializeShadedRegions();

            AntControl.Instance.AddMonitoringUI(this);
        }

        /// <summary>
        /// Add a node to be displayed on overview page
        /// 
        /// Added nodes cannot be modified, re-add to update display
        /// </summary>
        /// <param name="node">The node to be displayed, can not be modified while added</param>
        public void UpdateNode(SensorNode node)
        {
            //attempt to remove first to ensure everything is cleaned up correctly
            RemoveNode(node);

            node = node.Clone();
            node.UseActivePalette();

            node.Button.Click += NodeClick;

            var viewbox = sensorViewboxes[node.xpos][node.ypos];
            viewbox.Child = node;
            nodes.Add(node);

            AddViewboxOffset(viewbox, node.xoffset, node.yoffset);

            SetNodeRotation(viewbox, node.Rotation);
            InitializeDetectionLines(node);
        }

        /// <summary>
        /// Remove a node from the overview page
        /// </summary>
        /// <param name="node">The node to remove from display</param>
        public void RemoveNode(SensorNode node)
        {
            bool found = false;
            //switch 'node' to point to the copy we made when it was added.
            foreach(var copy in nodes)
            {
                if(copy.NodeID == node.NodeID)
                {
                    node = copy;
                    found = true;
                    break;
                }
            }

            if(!found)
            {
                //if not found it was never added
                return;
            }

            var viewbox = sensorViewboxes[node.xpos][node.ypos];

            AddViewboxOffset(viewbox, -node.xoffset, -node.yoffset);

            viewbox.Child = null;

            nodes.Remove(node);

            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Up))
            {
                lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Up].Visibility = Visibility.Hidden;
            }

            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Right))
            {
                lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Right].Visibility = Visibility.Hidden;
            }

            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Down))
            {
                lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Down].Visibility = Visibility.Hidden;
            }

            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Left))
            {
                lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Left].Visibility = Visibility.Hidden;
            }
        }

        public void MarkSensorDetection(int xpos, int ypos, LineDirection direction, Brush colour)
        {
            if (lineSegmentAssociations[xpos][ypos].ContainsKey(direction))
            {
                lineSegmentAssociations[xpos][ypos][direction].ColoredLine.Stroke = colour;
                lineSegmentAssociations[xpos][ypos][direction].Visibility = Visibility.Visible;                
            }
            else
            {
                //ignore invalid requests, makes it easy to try every direction for broad colour setting.
            }
        }

        public void MarkSensorDetection(SensorNode node, LineDirection direction, Brush colour)
        {
            MarkSensorDetection(node.xpos, node.ypos, direction, colour);
        }


        public void SetRegionActivityVariable(byte xCoordinate, byte yCoordinate, RegionStatus regionStatus)
        {
            Brush colour;
            switch (regionStatus)
            {
                case RegionStatus.NoDetection:
                    colour = Brushes.Transparent;
                    break;
                case RegionStatus.ProbableDetection:
                    colour = StatusColour.Yellow;
                    break;
                case RegionStatus.DefiniteDetection:
                    colour = StatusColour.Red;
                    break;
                default:
                    colour = Brushes.Transparent;
                    break;
            }
            shadedRegions[xCoordinate][yCoordinate].Fill = colour;
        }

        public void IncrementDetectionCount()
        {
            DetectionsPastHour.NumberField.Content = (int.Parse(DetectionsPastHour.NumberField.Content.ToString()) + 1).ToString();
        }

        public void LogSystemProblem(String logString)
        {
            SystemProblems.TextList.Items.Add(CreateLogItem(logString));
        }

        public void LogEvent(String logString)
        {
            EventLog.TextList.Items.Add(CreateLogItem(logString));
        }

        #endregion

        #region Private Methods

        private TextBox CreateLogItem(String logString)
        {
            TextBox textBox = new TextBox();
            textBox.TextWrapping = TextWrapping.Wrap;
            textBox.BorderThickness = new Thickness(0);
            textBox.Text = DateTimeOffset.UtcNow.ToLocalTime().ToString(timestampFormatString) + ":\n" + logString;

            return textBox;
        }

        private void NodeClick(object sender, RoutedEventArgs e)
        {
            foreach(var node in nodes)
            {
                if(node.Button == sender)
                {
                    MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
                    parentWindow.PageFrame.Navigate(new NodeOverviewPage(node, sensorViewboxes));
                    break;
                }
            }            
        }

        private void InitializeViewboxes()
        {
            sensorViewboxes = new List<List<Viewbox>>
            {
                new List<Viewbox>
                {
                    MonitGrid.SensorViewbox_0_0,
                    MonitGrid.SensorViewbox_0_1,
                    MonitGrid.SensorViewbox_0_2
                },
                new List<Viewbox>
                {
                    MonitGrid.SensorViewbox_1_0,
                    MonitGrid.SensorViewbox_1_1,
                    MonitGrid.SensorViewbox_1_2
                },
                new List<Viewbox>
                {
                    MonitGrid.SensorViewbox_2_0,
                    MonitGrid.SensorViewbox_2_1,
                    MonitGrid.SensorViewbox_2_2
                }
            };
        }

        private void InitializeLineSegments()
        {
            lineSegmentAssociations = new List<List<Dictionary<LineDirection, LineWithBorder>>>
            {
                new List<Dictionary<LineDirection, LineWithBorder>>
                {
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Right, MonitGrid.Line_0_0_Right },
                        { LineDirection.Down, MonitGrid.Line_0_0_Down },
                    },
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Right, MonitGrid.Line_0_1_Right },
                        { LineDirection.Down, MonitGrid.Line_0_1_Down },
                        { LineDirection.Up, MonitGrid.Line_0_1_Up },
                    },
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Right, MonitGrid.Line_0_2_Right },
                        { LineDirection.Up, MonitGrid.Line_0_2_Up },
                    },
                },
                new List<Dictionary<LineDirection, LineWithBorder>>
                {
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Right, MonitGrid.Line_1_0_Right },
                        { LineDirection.Down, MonitGrid.Line_1_0_Down },
                        { LineDirection.Left, MonitGrid.Line_1_0_Left },
                    },
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Right, MonitGrid.Line_1_1_Right },
                        { LineDirection.Down, MonitGrid.Line_1_1_Down },
                        { LineDirection.Up, MonitGrid.Line_1_1_Up },
                        { LineDirection.Left, MonitGrid.Line_1_1_Left },
                    },
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Right, MonitGrid.Line_1_2_Right },
                        { LineDirection.Up, MonitGrid.Line_1_2_Up },
                        { LineDirection.Left, MonitGrid.Line_1_2_Left },
                    },
                },
                new List<Dictionary<LineDirection, LineWithBorder>>
                {
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Left, MonitGrid.Line_2_0_Left },
                        { LineDirection.Down, MonitGrid.Line_2_0_Down },
                    },
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Left, MonitGrid.Line_2_1_Left },
                        { LineDirection.Down, MonitGrid.Line_2_1_Down },
                        { LineDirection.Up, MonitGrid.Line_2_1_Up },
                    },
                    new Dictionary<LineDirection, LineWithBorder>
                    {
                        { LineDirection.Left, MonitGrid.Line_2_2_Left },
                        { LineDirection.Up, MonitGrid.Line_2_2_Up },
                    },
                }
            };
        }

        private void InitializeShadedRegions()
        {
            shadedRegions = new List<List<Path>>
            {
                new List<Path>
                {
                    MonitGrid.Rectangle_0_0,
                    MonitGrid.Rectangle_0_1,
                },
                new List<Path>
                {
                    MonitGrid.Rectangle_1_0,
                    MonitGrid.Rectangle_1_1,
                },
            };
        }

        private void AddViewboxOffset(Viewbox viewbox, int offsetx, int offsety)
        {
            var margin = viewbox.Margin;
            margin.Top -= offsety * OffsetScalePixels;
            margin.Bottom += offsety * OffsetScalePixels;
            margin.Left += offsetx * OffsetScalePixels;
            margin.Right -= offsetx * OffsetScalePixels;
            viewbox.Margin = margin;
        }

        private void SetNodeRotation(Viewbox viewbox, Rotation rotation)
        {
            var r = viewbox.RenderTransform as RotateTransform;
            r.Angle = rotation.Val;
        }

        private void InitializeDetectionLines(SensorNode node)
        {
            switch(node.configuration)
            {
                case HardwareConfiguration.Pir2:
                    if(node.Rotation.Val == Rotation.R0)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Blue);
                    }
                    if (node.Rotation.Val == Rotation.R90)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Disabled);
                    }
                    if (node.Rotation.Val == Rotation.R180)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Disabled);
                    }
                    if (node.Rotation.Val == Rotation.R270)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Blue);
                    }
                    break;
                case HardwareConfiguration.PirLidar:
                case HardwareConfiguration.PirLidarLed:
                    if (node.Rotation.Val == Rotation.R0)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Disabled);
                    }
                    if (node.Rotation.Val == Rotation.R90)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Disabled);
                    }
                    if (node.Rotation.Val == Rotation.R180)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Blue);
                    }
                    if (node.Rotation.Val == Rotation.R270)
                    {
                        MarkSensorDetection(node, LineDirection.Up, StatusColour.Blue);
                        MarkSensorDetection(node, LineDirection.Right, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Down, StatusColour.Disabled);
                        MarkSensorDetection(node, LineDirection.Left, StatusColour.Blue);
                    }
                    break;
                case HardwareConfiguration.Unknown:
                    break;
            }
        }
        #endregion
    }
}
