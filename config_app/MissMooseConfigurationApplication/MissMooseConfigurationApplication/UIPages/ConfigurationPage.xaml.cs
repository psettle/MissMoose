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

        private Viewbox ActiveViewbox = null;
        #endregion

        #region Public Methods

        public ConfigurationPage()
        {
            InitializeComponent();
            InitializeViewboxes();
            InitializePulses();
            

            AddNewNode(new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_1_PIR_1_LIDAR_LEDS, 2));
            TransferNode(NewSensorViewbox, sensorViewboxes[1][1]);
            AddNewNode(new SensorNode(HardwareConfiguration.HARDWARECONFIGURATION_2_PIR, 3));

            

            //SetNodeRotation(sensorViewboxes[1][1], NodeRotation.NODEROTATION_180);

            //TransferNode(sensorViewboxes[1][1], sensorViewboxes[2][2]);
        }

        public void AddNewNode(SensorNode node)
        {
            NewSensorViewbox.Child = null;
            NewSensorViewbox.Child = node;
            node.Button.Click += NodeClick;
            NewNodeLabel.Content = "New Node!";
            SetActiveViewbox(NewSensorViewbox);
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
                    v.Opacity = 0.5;
                }
            }

            NewSensorViewbox.Opacity = 0.5;

            ActiveViewbox = newActive;
            ActiveViewbox.Opacity = 1.0f;

            var node = ActiveViewbox.Child as SensorNode;

            NodeNameLabel.Content = "Node " + node.NodeID;
        }

        private void TransferNode(Viewbox source, Viewbox destination)
        {
            SensorNode node = source.Child as SensorNode;

            source.Child = null;
            destination.Child = node;

            if(source == NewSensorViewbox)
            {
                NewNodeLabel.Content = "";
            }

            SetNodeRotation(destination, node.rotation);
            UpdatePulseStates();
        }

        private void SetNodeRotation(Viewbox viewbox, NodeRotation rotation)
        {
            double newAngle = 0;

            switch(rotation)
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

        private void InitializePulses()
        {
            pulses = new List<List<PulsingCircle>>
            {
                new List<PulsingCircle>
                {
                    Pulse_0_0,
                    Pulse_0_1,
                    Pulse_0_2
                },
                new List<PulsingCircle>
                {
                    Pulse_1_0,
                    Pulse_1_1,
                    Pulse_1_2
                },
                new List<PulsingCircle>
                {
                    Pulse_2_0,
                    Pulse_2_1,
                    Pulse_2_2
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

        private void RotateClockwise_Click(object sender, RoutedEventArgs e)
        {
            if(ActiveViewbox == null || ActiveViewbox.Child == null)
            {
                return;
            }

            var node = ActiveViewbox.Child as SensorNode;
            var rotation = node.rotation;

            switch(rotation)
            {
                case NodeRotation.NODEROTATION_0:
                    rotation = NodeRotation.NODEROTATION_45;
                    break;
                case NodeRotation.NODEROTATION_45:
                    rotation = NodeRotation.NODEROTATION_90;
                    break;
                case NodeRotation.NODEROTATION_90:
                    rotation = NodeRotation.NODEROTATION_135;
                    break;
                case NodeRotation.NODEROTATION_135:
                    rotation = NodeRotation.NODEROTATION_180;
                    break;
                case NodeRotation.NODEROTATION_180:
                    rotation = NodeRotation.NODEROTATION_225;
                    break;
                case NodeRotation.NODEROTATION_225:
                    rotation = NodeRotation.NODEROTATION_270;
                    break;
                case NodeRotation.NODEROTATION_270:
                    rotation = NodeRotation.NODEROTATION_315;
                    break;
                case NodeRotation.NODEROTATION_315:
                    rotation = NodeRotation.NODEROTATION_0;
                    break;
            }

            SetNodeRotation(ActiveViewbox, rotation);
        }

        private void RotateCounterClockwise_Click(object sender, RoutedEventArgs e)
        {
            if (ActiveViewbox == null || ActiveViewbox.Child == null)
            {
                return;
            }

            var node = ActiveViewbox.Child as SensorNode;
            var rotation = node.rotation;

            switch (rotation)
            {
                case NodeRotation.NODEROTATION_0:
                    rotation = NodeRotation.NODEROTATION_315;
                    break;
                case NodeRotation.NODEROTATION_45:
                    rotation = NodeRotation.NODEROTATION_0;
                    break;
                case NodeRotation.NODEROTATION_90:
                    rotation = NodeRotation.NODEROTATION_45;
                    break;
                case NodeRotation.NODEROTATION_135:
                    rotation = NodeRotation.NODEROTATION_90;
                    break;
                case NodeRotation.NODEROTATION_180:
                    rotation = NodeRotation.NODEROTATION_135;
                    break;
                case NodeRotation.NODEROTATION_225:
                    rotation = NodeRotation.NODEROTATION_180;
                    break;
                case NodeRotation.NODEROTATION_270:
                    rotation = NodeRotation.NODEROTATION_225;
                    break;
                case NodeRotation.NODEROTATION_315:
                    rotation = NodeRotation.NODEROTATION_270;
                    break;
            }

            SetNodeRotation(ActiveViewbox, rotation);
        }

        #endregion


    }
}
