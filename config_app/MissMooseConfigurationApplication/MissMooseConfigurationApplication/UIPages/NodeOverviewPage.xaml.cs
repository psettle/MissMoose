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
            AssignNode(MainSensorViewbox, node.xpos, node.ypos, CentreNodeClick);
            AssignNode(LeftSensorViewbox, node.xpos - 1, node.ypos, AdjacentNodeClick);
            AssignNode(RightSensorViewbox, node.xpos + 1, node.ypos, AdjacentNodeClick);
            AssignNode(TopSensorViewbox, node.xpos, node.ypos - 1, AdjacentNodeClick);
            AssignNode(BottomSensorViewbox, node.xpos, node.ypos + 1, AdjacentNodeClick);
        }
        #endregion

        #region Private Methods
        private void AssignNode(Viewbox target, int xpos, int ypos, RoutedEventHandler OnClick)
        {
            try
            {
                var localNode = nodes[xpos][ypos].Child as SensorNode;
                localNode = localNode.Clone();

                target.Child = localNode;
                var rotation = target.RenderTransform as RotateTransform;
                rotation.Angle = localNode.Rotation.Val;
                localNode.Button.Click += OnClick;
            }
            catch (ArgumentOutOfRangeException) { }
            catch (NullReferenceException) { }
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
            SensorNode localNode = ((Grid)((Button)sender).Parent).Parent as SensorNode;
            MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
            parentWindow.PageFrame.Navigate(new NodeOverviewPage(localNode, nodes));
        }
        #endregion
    }
}
