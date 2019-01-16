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

namespace MissMooseConfigurationApplication.UIPages
{
    /// <summary>
    /// Interaction logic for NodeOverviewPage.xaml
    /// </summary>
    public partial class NodeOverviewPage : Page
    {
        #region Private Members
        private List<List<Viewbox>> nodes;
        #endregion

        #region Public Members
        public NodeOverviewPage(SensorNode node, List<List<Viewbox>> otherNodes)
        {
            InitializeComponent();

            nodes = otherNodes;
            node = node.Clone();

            AssignNode(MainSensorViewbox, node);
            node.Button.Click += CentreNodeClick;

            //setup peripheral nodes
            if(node.xpos - 1 >= 0 && otherNodes[node.xpos - 1][node.ypos].Child != null)
            {
                var localNode = otherNodes[node.xpos - 1][node.ypos].Child as SensorNode;
                localNode = localNode.Clone();
                AssignNode(LeftSensorViewbox, localNode);
                localNode.Button.Click += AdjacentNodeClick;
            }

            if (node.xpos + 1 < otherNodes.Count && otherNodes[node.xpos + 1][node.ypos].Child != null)
            {
                var localNode = otherNodes[node.xpos + 1][node.ypos].Child as SensorNode;
                localNode = localNode.Clone();
                AssignNode(RightSensorViewbox, localNode);
                localNode.Button.Click += AdjacentNodeClick;
            }

            if (node.ypos - 1 >= 0 && otherNodes[node.xpos][node.ypos - 1].Child != null)
            {
                var localNode = otherNodes[node.xpos][node.ypos - 1].Child as SensorNode;
                localNode = localNode.Clone();
                AssignNode(TopSensorViewbox, localNode);
                localNode.Button.Click += AdjacentNodeClick;
            }

            if (node.ypos + 1 < otherNodes[node.xpos].Count && otherNodes[node.xpos][node.ypos + 1].Child != null)
            {
                var localNode = otherNodes[node.xpos][node.ypos + 1].Child as SensorNode;
                localNode = localNode.Clone();
                AssignNode(BottomSensorViewbox, localNode);
                localNode.Button.Click += AdjacentNodeClick;
            }
        }
        #endregion

        #region Private Methods
        private void AssignNode(Viewbox target, SensorNode clonedNode)
        {
            //assign to viewbox, set angle
            target.Child = clonedNode;

            var rotation = target.RenderTransform as RotateTransform;
            rotation.Angle = clonedNode.Rotation.Val;
        }

        private void CentreNodeClick(object sender, RoutedEventArgs e)
        {
            //send back to main overview page
            MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
            PageSwitcherButton falseSender = new PageSwitcherButton();
            falseSender.Name = "SystemOverviewPageButton";
            parentWindow.PageSwitchClick(falseSender);
        }

        private void AdjacentNodeClick(object sender, RoutedEventArgs e)
        {
            //identify clicked viewbox and create a new page to show the clicked node.
            SensorNode localNode;
 
            do
            {
                localNode = TopSensorViewbox.Child as SensorNode;          
                if(localNode != null && localNode.Button == sender)
                {
                    break;
                }

                localNode = RightSensorViewbox.Child as SensorNode;
                if (localNode != null && localNode.Button == sender)
                {
                    break;
                }

                localNode = BottomSensorViewbox.Child as SensorNode;
                if (localNode != null && localNode.Button == sender)
                {
                    break;
                }

                localNode = LeftSensorViewbox.Child as SensorNode;
                if (localNode != null && localNode.Button == sender)
                {
                    break;
                }

                throw new InvalidOperationException("Unknown adjacent node clicked?");
            } while (false);

            MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
            parentWindow.PageFrame.Navigate(new NodeOverviewPage(localNode, nodes));
        }
        #endregion
    }
}
