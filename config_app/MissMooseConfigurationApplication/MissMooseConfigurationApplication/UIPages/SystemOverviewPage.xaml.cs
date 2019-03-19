﻿using MissMooseConfigurationApplication.UIPages;
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
        private List<List<Dictionary<LineDirection, LineWithBorder.LineSegment>>> lineSegmentAssociations;
        private Dictionary<LineWithBorder.LineSegment, StatusColour> lineSegmentColours;
        private Dictionary<LineWithBorder.LineSegment, bool> lineSegmentVisibilities;
        private List<List<ActivityRegion>> shadedRegions;
        private Dictionary<ActivityRegion, RegionStatus> shadedRegionStatuses;
        private const int GridSize = 3;
        private const int OffsetScalePixels = 5;
        private const string timestampFormatString = "yyyy-MMM-dd hh:mm:ss tt";
        private const int maxLogCount = 100;
        private String eventLogFile = "EventLog.txt";
        private String systemProblemsFile = "SystemProblems.txt";

        private SolidColorBrush Red
        {
            get { return (SolidColorBrush)Application.Current.FindResource("ThemeBrushRed"); }
        }

        private SolidColorBrush Yellow
        {
            get { return (SolidColorBrush)Application.Current.FindResource("ThemeBrushYellow"); }
        }

        private SolidColorBrush Blue
        {
            get { return (SolidColorBrush)Application.Current.FindResource("ThemeBrushForegroundBlue"); }
        }

        private SolidColorBrush Disabled
        {
            get { return (SolidColorBrush)Application.Current.FindResource("ThemeBrushForeground"); }
        }

        #endregion

        #region Public Methods
        public SystemOverviewPage()
        {
            InitializeComponent();
            InitializeViewboxes();
            InitializeLineSegments();
            InitializeShadedRegions();

            AntControl.Instance.AddMonitoringUI(this);

            // Register for the UpdateTheme event so we know when to update the colours
            ((MainWindow)Application.Current.MainWindow).UpdateTheme += OnUpdateTheme;
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

            var viewbox = sensorViewboxes[node.xpos][node.ypos];
            viewbox.Child = node;
            nodes.Add(node);

            AddViewboxOffset(viewbox, node.xoffset, node.yoffset);
            AddShadedRegionOffset(node);

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
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Up];
                linesegment.line.SetVisibility(linesegment.half, false);

                lineSegmentColours[linesegment] = StatusColour.Transparent;
                lineSegmentVisibilities[linesegment] = false;
            }

            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Right))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Right];
                linesegment.line.SetVisibility(linesegment.half, false);

                lineSegmentColours[linesegment] = StatusColour.Transparent;
                lineSegmentVisibilities[linesegment] = false;
            }

            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Down))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Down];
                linesegment.line.SetVisibility(linesegment.half, false);

                lineSegmentColours[linesegment] = StatusColour.Transparent;
                lineSegmentVisibilities[linesegment] = false;
            }

            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Left))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Left];
                linesegment.line.SetVisibility(linesegment.half, false);

                lineSegmentColours[linesegment] = StatusColour.Transparent;
                lineSegmentVisibilities[linesegment] = false;
            }
        }

        public void MarkSensorDetection(int xpos, int ypos, LineDirection direction, StatusColour colour)
        {
            if (lineSegmentAssociations[xpos][ypos].ContainsKey(direction))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[xpos][ypos][direction];
                linesegment.line.SetVisibility(linesegment.half, true);
                linesegment.line.SetColour(linesegment.half, GetBrushFromStatusColour(colour).Color);

                lineSegmentColours[linesegment] = colour;
                lineSegmentVisibilities[linesegment] = true;
            }
            else
            {
                //ignore invalid requests, makes it easy to try every direction for broad colour setting.
            }
        }

        public void MarkSensorDetection(SensorNode node, LineDirection direction, StatusColour colour)
        {
            MarkSensorDetection(node.xpos, node.ypos, direction, colour);
        }

        public void SetRegionActivityVariable(byte xCoordinate, byte yCoordinate, RegionStatus regionStatus)
        {
            SetRegionActivityVariable(shadedRegions[xCoordinate][yCoordinate], regionStatus);

            shadedRegionStatuses[shadedRegions[xCoordinate][yCoordinate]] = regionStatus;
        }

        public void LogSystemProblem(String logString)
        {
            String fullLogString = AddTimestamp(logString);
            if (SystemProblems.TextList.Items.Count == maxLogCount)
            {
                SystemProblems.TextList.Items.RemoveAt(0);
            }
            SystemProblems.TextList.Items.Add(CreateLogItem(fullLogString));
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(systemProblemsFile, true))
            {
                file.WriteLine(fullLogString);
            }
        }

        public void LogEvent(String logString)
        {
            String fullLogString = AddTimestamp(logString);
            if (EventLog.TextList.Items.Count == maxLogCount)
            {
                EventLog.TextList.Items.RemoveAt(0);
            }
            EventLog.TextList.Items.Add(CreateLogItem(fullLogString));
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(eventLogFile, true))
            {
                file.WriteLine(fullLogString);
            }
        }

        #endregion

        #region Private Methods

        private TextBox CreateLogItem(String logString)
        {
            TextBox textBox = new TextBox();
            textBox.TextWrapping = TextWrapping.Wrap;
            textBox.BorderThickness = new Thickness(0);
            textBox.Background = (SolidColorBrush)Application.Current.FindResource("ThemeBrushBackground");
            textBox.Foreground = (SolidColorBrush)Application.Current.FindResource("ThemeBrushText");
            textBox.Text = logString;

            return textBox;
        }

        private String AddTimestamp(String logString)
        {
            return DateTimeOffset.UtcNow.ToLocalTime().ToString(timestampFormatString) + ":\n" + logString;
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
            lineSegmentAssociations = new List<List<Dictionary<LineDirection, LineWithBorder.LineSegment>>>
            {
                // lines starting from logical x = 0
                new List<Dictionary<LineDirection, LineWithBorder.LineSegment>>
                {
                    // lines touching node 0,0
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Right, new LineWithBorder.LineSegment(MonitGrid.Line_00_10, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Down, new LineWithBorder.LineSegment(MonitGrid.Line_00_01, LineWithBorder.LineHalves.FirstHalf) },
                    },
                    // lines touching node 0,1
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Right, new LineWithBorder.LineSegment(MonitGrid.Line_01_11, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Down, new LineWithBorder.LineSegment(MonitGrid.Line_01_02, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Up, new LineWithBorder.LineSegment(MonitGrid.Line_00_01, LineWithBorder.LineHalves.SecondHalf) },
                    },
                    // lines touching node 0,2
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Right, new LineWithBorder.LineSegment(MonitGrid.Line_02_12, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Up, new LineWithBorder.LineSegment(MonitGrid.Line_01_02, LineWithBorder.LineHalves.SecondHalf) },
                    },
                },
                // lines starting from logical x = 1
                new List<Dictionary<LineDirection, LineWithBorder.LineSegment>>
                {
                    // lines touching node 1,0
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Right, new LineWithBorder.LineSegment(MonitGrid.Line_10_20, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Down, new LineWithBorder.LineSegment(MonitGrid.Line_10_11, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Left, new LineWithBorder.LineSegment(MonitGrid.Line_00_10, LineWithBorder.LineHalves.SecondHalf) },
                    },
                    // lines touching node 1,1
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Right, new LineWithBorder.LineSegment(MonitGrid.Line_11_21, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Down, new LineWithBorder.LineSegment(MonitGrid.Line_11_12, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Up, new LineWithBorder.LineSegment(MonitGrid.Line_10_11, LineWithBorder.LineHalves.SecondHalf) },
                        { LineDirection.Left, new LineWithBorder.LineSegment(MonitGrid.Line_01_11, LineWithBorder.LineHalves.SecondHalf) },
                    },
                    // lines touching node 1,2
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Right, new LineWithBorder.LineSegment(MonitGrid.Line_12_22, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Up, new LineWithBorder.LineSegment(MonitGrid.Line_11_12, LineWithBorder.LineHalves.SecondHalf) },
                        { LineDirection.Left, new LineWithBorder.LineSegment(MonitGrid.Line_02_12, LineWithBorder.LineHalves.SecondHalf) },
                    },
                },
                // lines starting from logical x = 2
                new List<Dictionary<LineDirection, LineWithBorder.LineSegment>>
                {
                    // lines touching node 2,0
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Left, new LineWithBorder.LineSegment(MonitGrid.Line_10_20, LineWithBorder.LineHalves.SecondHalf) },
                        { LineDirection.Down, new LineWithBorder.LineSegment(MonitGrid.Line_20_21, LineWithBorder.LineHalves.FirstHalf) },
                    },
                    // lines touching node 2,1
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Left, new LineWithBorder.LineSegment(MonitGrid.Line_11_21, LineWithBorder.LineHalves.SecondHalf) },
                        { LineDirection.Down, new LineWithBorder.LineSegment(MonitGrid.Line_21_22, LineWithBorder.LineHalves.FirstHalf) },
                        { LineDirection.Up, new LineWithBorder.LineSegment(MonitGrid.Line_20_21, LineWithBorder.LineHalves.SecondHalf) },
                    },
                    // lines touching node 2,2
                    new Dictionary<LineDirection, LineWithBorder.LineSegment>
                    {
                        { LineDirection.Left, new LineWithBorder.LineSegment(MonitGrid.Line_12_22, LineWithBorder.LineHalves.SecondHalf) },
                        { LineDirection.Up, new LineWithBorder.LineSegment(MonitGrid.Line_21_22, LineWithBorder.LineHalves.SecondHalf) },
                    },
                }
            };

            lineSegmentColours = new Dictionary<LineWithBorder.LineSegment, StatusColour>();
            lineSegmentVisibilities = new Dictionary<LineWithBorder.LineSegment, bool>();
            foreach (List<Dictionary<LineDirection, LineWithBorder.LineSegment>> list in lineSegmentAssociations)
            {
                foreach (Dictionary<LineDirection, LineWithBorder.LineSegment> dict in list)
                {
                    foreach (LineWithBorder.LineSegment lineSegment in dict.Values)
                    {
                        lineSegmentColours.Add(lineSegment, StatusColour.Transparent);
                        lineSegmentVisibilities.Add(lineSegment, false);
                    }
                }
            }
        }

        private void InitializeShadedRegions()
        {
            shadedRegions = new List<List<ActivityRegion>>
            {
                new List<ActivityRegion>
                {
                    MonitGrid.Rectangle_0_0,
                    MonitGrid.Rectangle_0_1,
                },
                new List<ActivityRegion>
                {
                    MonitGrid.Rectangle_1_0,
                    MonitGrid.Rectangle_1_1,
                },
            };

            shadedRegionStatuses = new Dictionary<ActivityRegion, RegionStatus>();
            foreach(List<ActivityRegion> list in shadedRegions)
            {
                foreach(ActivityRegion activityRegion in list)
                {
                    shadedRegionStatuses.Add(activityRegion, RegionStatus.NoDetection);
                }
            }
        }

        private void SetRegionActivityVariable(ActivityRegion region, RegionStatus regionStatus)
        {
            Brush colour;
            switch (regionStatus)
            {
                case RegionStatus.NoDetection:
                    colour = Brushes.Transparent;
                    break;
                case RegionStatus.ProbableDetection:
                    colour = GetBrushFromStatusColour(StatusColour.Yellow);
                    break;
                case RegionStatus.DefiniteDetection:
                    colour = GetBrushFromStatusColour(StatusColour.Red);
                    break;
                default:
                    colour = Brushes.Transparent;
                    break;
            }
            region.Region.Fill = colour;
        }

        private void AddShadedRegionOffset(SensorNode node)
        {
            Point nodePoint = new Point() { X = node.xpos, Y = node.ypos };
            for (int i = 0; i < shadedRegions.Count; i++)
            {
                for (int j = 0; j < shadedRegions[i].Count; j++)
                {
                    if (shadedRegions[i][j].PointCoordinates.Contains(nodePoint))
                    {
                        shadedRegions[i][j].UpdateActivityRegion(nodePoint, node.xoffset * OffsetScalePixels, node.yoffset * OffsetScalePixels);
                    }
                }
            }
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
            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Up))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Up];
                linesegment.line.X2_Offset = node.xoffset * OffsetScalePixels;
                linesegment.line.Y2_Offset = -node.yoffset * OffsetScalePixels;
            }
            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Right))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Right];
                linesegment.line.X1_Offset = node.xoffset * OffsetScalePixels;
                linesegment.line.Y1_Offset = -node.yoffset * OffsetScalePixels;
            }
            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Down))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Down];
                linesegment.line.X1_Offset = node.xoffset * OffsetScalePixels;
                linesegment.line.Y1_Offset = -node.yoffset * OffsetScalePixels;
            }
            if (lineSegmentAssociations[node.xpos][node.ypos].ContainsKey(LineDirection.Left))
            {
                LineWithBorder.LineSegment linesegment = lineSegmentAssociations[node.xpos][node.ypos][LineDirection.Left];
                linesegment.line.X2_Offset = node.xoffset * OffsetScalePixels;
                linesegment.line.Y2_Offset = -node.yoffset * OffsetScalePixels;
            }
            switch (node.configuration)
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

        private void OnUpdateTheme()
        {
            foreach (TextBox item in EventLog.TextList.Items)
            {
                item.Background = (SolidColorBrush)Application.Current.FindResource("ThemeBrushBackground");
                item.Foreground = (SolidColorBrush)Application.Current.FindResource("ThemeBrushText");
            }

            foreach (TextBox item in SystemProblems.TextList.Items)
            {
                item.Background = (SolidColorBrush)Application.Current.FindResource("ThemeBrushBackground");
                item.Foreground = (SolidColorBrush)Application.Current.FindResource("ThemeBrushText");
            }

            foreach (LineWithBorder.LineSegment lineSegment in lineSegmentColours.Keys)
            {
                lineSegment.line.SetColour(lineSegment.half, GetBrushFromStatusColour(lineSegmentColours[lineSegment]).Color);
            }

            foreach (LineWithBorder.LineSegment lineSegment in lineSegmentVisibilities.Keys)
            {
                lineSegment.line.SetVisibility(lineSegment.half, lineSegmentVisibilities[lineSegment]);
            }

            foreach (ActivityRegion region in shadedRegionStatuses.Keys)
            {
                SetRegionActivityVariable(region, shadedRegionStatuses[region]);
            }
        }

        private SolidColorBrush GetBrushFromStatusColour(StatusColour colour)
        {
            switch (colour)
            {
                case StatusColour.Yellow:
                    return Yellow;
                case StatusColour.Red:
                    return Red;
                case StatusColour.Blue:
                    return Blue;
                case StatusColour.Disabled:
                    return Disabled;
                case StatusColour.Transparent:
                    return Brushes.Transparent;
                default:
                    return Disabled;
            }
        }

        #endregion
    }
}
