using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace MissMooseConfigurationApplication.UIPages
{
    /// <summary>
    /// Interaction logic for ConfigurationPage.xaml
    /// </summary>
    public partial class ConfigurationPage : Page
    {
        #region Private Members
        private List<List<PulsingCircle>> pulses;
        private List<List<Viewbox>> sensorViewboxes;

        private const int GridSize = 3;
        private const int OffsetScalePixels = 10;

        private Viewbox ActiveViewbox = null;
        #endregion

        #region Public Members
        public List<SensorNode> nodes { get; private set; } = new List<SensorNode>();
        #endregion

        #region Public Methods

        public ConfigurationPage()
        {
            InitializeComponent();
            InitializeViewboxes();
            InitializePulses();
            ClockwiseButton.Button.Click += RotateClockwise_Click;
            CounterClockwiseButton.Button.Click += RotateCounterClockwise_Click;
            UpButton.Button.Click += DirectionClick;
            DownButton.Button.Click += DirectionClick;
            RightButton.Button.Click += DirectionClick;
            LeftButton.Button.Click += DirectionClick;

            AntControl.Instance.AddConfigUI(this);
        }

        public void AddNewNode(SensorNode node)
        {
            NewSensorViewbox.Child = null;
            NewSensorViewbox.Child = node;
            node.Button.Click += NodeClick;
            NewNodeLabel.Content = "New Node!";
            SetActiveViewbox(NewSensorViewbox);
        }

        public void AddExistingNode(SensorNode node)
        {
            node.Button.Click += NodeClick;

            var viewbox = sensorViewboxes[node.xpos][node.ypos];
            viewbox.Child = node;
            nodes.Add(node);

            AddViewboxOffset(viewbox, node.xoffset, node.yoffset);
            
            SetNodeRotation(viewbox, node.Rotation);
            UpdatePulseStates();

            SetActiveViewbox(viewbox);
        }

        public List<SensorNode> GetCurrentNodes()
        {
            return nodes;
        }

        #endregion

        #region Private Methods

        private void NodeClick(object sender, RoutedEventArgs e)
        {
            Viewbox source = null;

            foreach (var list in sensorViewboxes)
            {
                foreach (var v in list)
                {
                    if(v.Child == null)
                    {
                        continue;
                    }

                    var node = v.Child as SensorNode;

                    if(node.Button == sender)
                    {
                        source = v;
                        break;
                    }
                }

                if(source != null)
                {
                    break;
                }
            }

            if(source == null)
            {
                var node = NewSensorViewbox.Child as SensorNode;

                if (node.Button == sender)
                {
                    source = NewSensorViewbox;
                }
                else
                {
                    throw new InvalidOperationException("Unknown Viewbox");
                }
            }

            SetActiveViewbox(source);
        }

        private void SetActiveViewbox(Viewbox newActive)
        {
            if(newActive.Child == null)
            {
                throw new InvalidOperationException("A non occupied viewbox cannot be active");
            }

            foreach(var list in sensorViewboxes)
            {
                foreach(var v in list)
                {
                    v.Opacity = 0.8;
                }
            }

            NewSensorViewbox.Opacity = 0.8;

            ActiveViewbox = newActive;
            ActiveViewbox.Opacity = 1.0f;

            var node = ActiveViewbox.Child as SensorNode;

            NodeNameLabel.Content = "Node " + node.NodeID;
        }

        private void TransferNode(Viewbox source, Viewbox destination)
        {
            SensorNode node = source.Child as SensorNode;

            AddViewboxOffset(source, -node.xoffset, -node.yoffset);
            node.xoffset = 0; node.yoffset = 0;

            source.Child = null;
            destination.Child = node;

            if(source == NewSensorViewbox)
            {
                NewNodeLabel.Content = "";
                nodes.Add(node);
            }

            SetNodeRotation(destination, node.Rotation);
            UpdatePulseStates();

            int row, col;
            for(row = 0; row < GridSize; ++row)
            {
                for (col = 0; col < GridSize; ++col)
                {
                    if(sensorViewboxes[row][col] == destination)
                    {
                        node.xpos = row;
                        node.ypos = col;
                        break;
                    }
                }
            }
        }

        private void SetNodeRotation(Viewbox viewbox, NodeRotation rotation)
        {
            SensorNode node = viewbox.Child as SensorNode;
            node.Rotation = new NodeRotation(rotation.Val);

            var r = viewbox.RenderTransform as RotateTransform;
            r.Angle = rotation.Val;
        }

        private void InitializePulses()
        {
            pulses = new List<List<PulsingCircle>>
            {
                new List<PulsingCircle>
                {
                    ConfigGrid.Pulse_0_0,
                    ConfigGrid.Pulse_0_1,
                    ConfigGrid.Pulse_0_2
                },
                new List<PulsingCircle>
                {
                    ConfigGrid.Pulse_1_0,
                    ConfigGrid.Pulse_1_1,
                    ConfigGrid.Pulse_1_2
                },
                new List<PulsingCircle>
                {
                    ConfigGrid.Pulse_2_0,
                    ConfigGrid.Pulse_2_1,
                    ConfigGrid.Pulse_2_2
                }
            };

            foreach(var list in pulses)
            {
                foreach(var pulse in list)
                {
                    pulse.Button.Click += OnPulseClick;
                }
            }

            UpdatePulseStates();
        }

        private void UpdatePulseStates()
        {
            int row = 0, column = 0;

            for (row = 0; row < GridSize; ++row)
            {
                for (column = 0; column < GridSize; ++column)
                {
                    if (sensorViewboxes[row][column].Child == null)
                    {
                        pulses[row][column].Configure(10, 20, 3);
                    }
                    else
                    {
                        pulses[row][column].Configure(10, 10, 1);
                    }
                }

            }
        }

        private void OnPulseClick(object sender, RoutedEventArgs e)
        {
            int row = 0, column = 0;
            bool found = false;

            for (row = 0; row < GridSize; ++row)
            {
                for (column = 0; column < GridSize; ++column)
                {
                    if(sender == pulses[row][column].Button)
                    {
                        found = true;
                        break;
                    }
                }

                if(found)
                {
                    break;
                }
            }

            if(ActiveViewbox != null && sensorViewboxes[row][column].Child == null)
            {
                TransferNode(ActiveViewbox, sensorViewboxes[row][column]);
                SetActiveViewbox(sensorViewboxes[row][column]);
            }
            
        }

        private void InitializeViewboxes()
        {
            sensorViewboxes = new List<List<Viewbox>>
            {
                new List<Viewbox>
                {
                    ConfigGrid.SensorViewbox_0_0,
                    ConfigGrid.SensorViewbox_0_1,
                    ConfigGrid.SensorViewbox_0_2
                },
                new List<Viewbox>
                {
                    ConfigGrid.SensorViewbox_1_0,
                    ConfigGrid.SensorViewbox_1_1,
                    ConfigGrid.SensorViewbox_1_2
                },
                new List<Viewbox>
                {
                    ConfigGrid.SensorViewbox_2_0,
                    ConfigGrid.SensorViewbox_2_1,
                    ConfigGrid.SensorViewbox_2_2
                }
            };
        }

        private void RotateClockwise_Click(object sender, RoutedEventArgs e)
        {
            if(ActiveViewbox == null || ActiveViewbox.Child == null)
            {
                return;
            }

            var node = ActiveViewbox.Child as SensorNode;
            node.Rotation.Add(NodeRotation.R90);

            SetNodeRotation(ActiveViewbox, node.Rotation);
        }

        private void RotateCounterClockwise_Click(object sender, RoutedEventArgs e)
        {
            if (ActiveViewbox == null || ActiveViewbox.Child == null)
            {
                return;
            }

            var node = ActiveViewbox.Child as SensorNode;
            node.Rotation.Add(-NodeRotation.R90);

            SetNodeRotation(ActiveViewbox, node.Rotation);
        }

        private void DirectionClick(object sender, RoutedEventArgs e)
        {
            if(ActiveViewbox == null || ActiveViewbox.Child == null || ActiveViewbox == NewSensorViewbox)
            {
                return;
            }

            var node = ActiveViewbox.Child as SensorNode;

            AddViewboxOffset(ActiveViewbox, -node.xoffset, -node.yoffset);

            if (sender == UpButton.Button)
            {
                if (node.yoffset < SensorNode.MaxOffset)
                {
                    node.yoffset++;
                }
            }
            else if(sender == DownButton.Button)
            {
                if (node.yoffset > -SensorNode.MaxOffset)
                {
                    node.yoffset--;
                }
            }
            else if(sender == LeftButton.Button)
            {
                if (node.xoffset > -SensorNode.MaxOffset)
                {
                    node.xoffset--;
                }
            }
            else if(sender == RightButton.Button)
            {
                if (node.xoffset < SensorNode.MaxOffset)
                {
                    node.xoffset++;
                }
            }

            AddViewboxOffset(ActiveViewbox, node.xoffset, node.yoffset);  
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

        #endregion


    }
}
