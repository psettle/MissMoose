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

            var viewbox = sensorViewboxes[node.xpos][node.ypos];
            viewbox.Child = node;
            nodes.Add(node);

            AddViewboxOffset(viewbox, node.xoffset, node.yoffset);

            SetNodeRotation(viewbox, node.Rotation);

            MarkSensorDetection(node, LineDirection.Up, StatusColour.Blue);
            MarkSensorDetection(node, LineDirection.Right, StatusColour.Blue);
            MarkSensorDetection(node, LineDirection.Down, StatusColour.Blue);
            MarkSensorDetection(node, LineDirection.Left, StatusColour.Blue);
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
                lineSegmentAssociations[xpos][ypos][direction].Stroke = colour;
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

            // If the colour is being set to red or yellow
            // or if the colour is being set to blue and there are no other lines set to red or yellow,
            // set the node status colour to the given colour
            if (!colour.Equals(StatusColour.Blue)
                || lineSegmentAssociations[node.xpos][node.ypos].Where(x => x.Key != direction && x.Value.Stroke.Equals(colour)).Count() == 0)
            {
                SensorNode nodeToSet = nodes.Where(x => x.NodeID == node.NodeID).FirstOrDefault();
                if (nodeToSet != null)
                {
                    nodeToSet.SetStatusColour(colour);
                }
            }
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
            lineSegmentAssociations = new List<List<Dictionary<LineDirection, Line>>>
            {
                new List<Dictionary<LineDirection, Line>>
                {
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, MonitGrid.Line_0_0_Right },
                        { LineDirection.Down, MonitGrid.Line_0_0_Down },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, MonitGrid.Line_0_1_Right },
                        { LineDirection.Down, MonitGrid.Line_0_1_Down },
                        { LineDirection.Up, MonitGrid.Line_0_1_Up },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, MonitGrid.Line_0_2_Right },
                        { LineDirection.Up, MonitGrid.Line_0_2_Up },
                    },
                },
                new List<Dictionary<LineDirection, Line>>
                {
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, MonitGrid.Line_1_0_Right },
                        { LineDirection.Down, MonitGrid.Line_1_0_Down },
                        { LineDirection.Left, MonitGrid.Line_1_0_Left },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, MonitGrid.Line_1_1_Right },
                        { LineDirection.Down, MonitGrid.Line_1_1_Down },
                        { LineDirection.Up, MonitGrid.Line_1_1_Up },
                        { LineDirection.Left, MonitGrid.Line_1_1_Left },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Right, MonitGrid.Line_1_2_Right },
                        { LineDirection.Up, MonitGrid.Line_1_2_Up },
                        { LineDirection.Left, MonitGrid.Line_1_2_Left },
                    },
                },
                new List<Dictionary<LineDirection, Line>>
                {
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Left, MonitGrid.Line_2_0_Left },
                        { LineDirection.Down, MonitGrid.Line_2_0_Down },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Left, MonitGrid.Line_2_1_Left },
                        { LineDirection.Down, MonitGrid.Line_2_1_Down },
                        { LineDirection.Up, MonitGrid.Line_2_1_Up },
                    },
                    new Dictionary<LineDirection, Line>
                    {
                        { LineDirection.Left, MonitGrid.Line_2_2_Left },
                        { LineDirection.Up, MonitGrid.Line_2_2_Up },
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

        private void SetNodeRotation(Viewbox viewbox, Rotation rotation)
        {
            var r = viewbox.RenderTransform as RotateTransform;
            r.Angle = rotation.Val;
        }
        #endregion
    }
}
