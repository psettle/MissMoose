using MissMooseConfigurationApplication.UIPages;
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
        private List<List<Dictionary<LineDirection,Line>>> lineSegmentAssociations;
        private const int GridSize = 3;
        private const int OffsetScalePixels = 5;
        #endregion

        #region Public Methods
        public SystemOverviewPage()
        {
            InitializeComponent();
            InitializeViewboxes();
            InitializeLineSegments();

            AntControl.Instance.AddMonitoringUI(this);
        }

        /// <summary>
        /// Add a node to be displayed on overview page
        /// 
        /// Added nodes cannot be modified, re-add to update display
        /// </summary>
        /// <param name="node">The node to be displayed, can not be modified while added</param>
        public void AddNode(SensorNode node)
        {
            //attempt to remove first to ensure everything is cleaned up correctly
            RemoveNode(node);

            node = node.Clone();

            node.Button.Click += NodeClick;

            node.GetPos(out int xpos, out int ypos);
            var viewbox = sensorViewboxes[xpos][ypos];
            viewbox.Child = node;
            nodes.Add(node);

            node.GetOffset(out int xoffset, out int yoffset);
            AddViewboxOffset(viewbox, xoffset, yoffset);

            SetNodeRotation(viewbox, node.GetRotation());

            MarkSensorDetection(node, LineDirection.Up, StatusColour.STATUSCOLOUR_BLUE);
            MarkSensorDetection(node, LineDirection.Right, StatusColour.STATUSCOLOUR_BLUE);
            MarkSensorDetection(node, LineDirection.Down, StatusColour.STATUSCOLOUR_BLUE);
            MarkSensorDetection(node, LineDirection.Left, StatusColour.STATUSCOLOUR_BLUE);
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
                if(copy.GetNodeID() == node.GetNodeID())
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

            node.GetPos(out int xpos, out int ypos);
            var viewbox = sensorViewboxes[xpos][ypos];

            node.GetOffset(out int xoffset, out int yoffset);
            AddViewboxOffset(viewbox, -xoffset, -yoffset);

            viewbox.Child = null;

            nodes.Remove(node);

            if(lineSegmentAssociations[xpos][ypos].ContainsKey(LineDirection.Up))
            {
                lineSegmentAssociations[xpos][ypos][LineDirection.Up].Visibility = Visibility.Hidden;
            }

            if (lineSegmentAssociations[xpos][ypos].ContainsKey(LineDirection.Right))
            {
                lineSegmentAssociations[xpos][ypos][LineDirection.Right].Visibility = Visibility.Hidden;
            }

            if (lineSegmentAssociations[xpos][ypos].ContainsKey(LineDirection.Down))
            {
                lineSegmentAssociations[xpos][ypos][LineDirection.Down].Visibility = Visibility.Hidden;
            }

            if (lineSegmentAssociations[xpos][ypos].ContainsKey(LineDirection.Left))
            {
                lineSegmentAssociations[xpos][ypos][LineDirection.Left].Visibility = Visibility.Hidden;
            }
        }

        public void MarkSensorDetection(int xpos, int ypos, LineDirection direction, StatusColour colour)
        {
            if (lineSegmentAssociations[xpos][ypos].ContainsKey(direction))
            {
                Brush brush;
                switch (colour)
                {
                    case StatusColour.STATUSCOLOUR_BLUE:
                        brush = Brushes.Blue;
                        break;
                    case StatusColour.STATUSCOLOUR_RED:
                        brush = Brushes.Red;
                        break;
                    case StatusColour.STATUSCOLOUR_YELLOW:
                        brush = Brushes.Yellow;
                        break;
                    default:
                        throw new InvalidOperationException("Unknown status colour");
                }

                lineSegmentAssociations[xpos][ypos][direction].Stroke = brush;
                lineSegmentAssociations[xpos][ypos][direction].Visibility = Visibility.Visible;
            }
            else
            {
                //ignore invalid requests, makes it easy to try every direction for broad colour setting.
            }
        }

        public void MarkSensorDetection(SensorNode node, LineDirection direction, StatusColour colour)
        {
            node.GetPos(out int xpos, out int ypos);
            MarkSensorDetection(xpos, ypos, direction, colour);
        }
        #endregion

        #region Private Methods
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
                    SensorViewbox_0_0,
                    SensorViewbox_0_1,
                    SensorViewbox_0_2
                },
                new List<Viewbox>
                {
                    SensorViewbox_1_0,
                    SensorViewbox_1_1,
                    SensorViewbox_1_2
                },
                new List<Viewbox>
                {
                    SensorViewbox_2_0,
                    SensorViewbox_2_1,
                    SensorViewbox_2_2
                }
            };
        }

        private void InitializeLineSegments()
        {
            lineSegmentAssociations = new List<List<Dictionary<LineDirection, Line>>>
            {
                new List<Dictionary<LineDirection, Line>>
                {
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, Line_0_0_Right },
                        { LineDirection.Down, Line_0_0_Down },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, Line_0_1_Right },
                        { LineDirection.Down, Line_0_1_Down },
                        { LineDirection.Up, Line_0_1_Up },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, Line_0_2_Right },
                        { LineDirection.Up, Line_0_2_Up },
                    },
                },
                new List<Dictionary<LineDirection, Line>>
                {
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, Line_1_0_Right },
                        { LineDirection.Down, Line_1_0_Down },
                        { LineDirection.Left, Line_1_0_Left },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, Line_1_1_Right },
                        { LineDirection.Down, Line_1_1_Down },
                        { LineDirection.Up, Line_1_1_Up },
                        { LineDirection.Left, Line_1_1_Left },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, Line_1_2_Right },
                        { LineDirection.Up, Line_1_2_Up },
                        { LineDirection.Left, Line_1_2_Left },
                    },
                },
                new List<Dictionary<LineDirection, Line>>
                {
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Left, Line_2_0_Left },
                        { LineDirection.Down, Line_2_0_Down },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Left, Line_2_1_Left },
                        { LineDirection.Down, Line_2_1_Down },
                        { LineDirection.Up, Line_2_1_Up },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Left, Line_2_2_Left },
                        { LineDirection.Up, Line_2_2_Up },
                    },
                }
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

        private void SetNodeRotation(Viewbox viewbox, NodeRotation rotation)
        {
            double newAngle = 0;

            switch (rotation)
            {
                case NodeRotation.NODEROTATION_0:
                    newAngle = 0.0;
                    break;
                case NodeRotation.NODEROTATION_45:
                    newAngle = 45.0;
                    break;
                case NodeRotation.NODEROTATION_90:
                    newAngle = 90.0;
                    break;
                case NodeRotation.NODEROTATION_135:
                    newAngle = 135.0;
                    break;
                case NodeRotation.NODEROTATION_180:
                    newAngle = 180.0;
                    break;
                case NodeRotation.NODEROTATION_225:
                    newAngle = 225.0;
                    break;
                case NodeRotation.NODEROTATION_270:
                    newAngle = 270.0;
                    break;
                case NodeRotation.NODEROTATION_315:
                    newAngle = 315.0;
                    break;
                default:
                    throw new InvalidOperationException("Invalid Angle");
            }

            SensorNode node = viewbox.Child as SensorNode;
            node.SetRotation(rotation);

            var r = viewbox.RenderTransform as RotateTransform;
            r.Angle = newAngle;
        }
        #endregion
    }
}
