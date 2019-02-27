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

namespace MissMooseConfigurationApplication.UIComponents
{
    /// <summary>
    /// Interaction logic for ActivityRegion.xaml
    /// </summary>
    public partial class ActivityRegion : UserControl
    {
        private Point TopRightPoint;
        private Point TopRightCoordinate;
        private Point TopLeftPoint;
        private Point TopLeftCoordinate;
        private Point BottomRightPoint;
        private Point BottomRightCoordinate;
        private Point BottomLeftPoint;
        private Point BottomLeftCoordinate;

        public PointCollection Points
        {
            get
            {
                return new PointCollection() { TopRightPoint, TopLeftPoint, BottomRightPoint, BottomLeftPoint };
            }
            set
            {
                Region.Points = value;
                TopRightPoint = value[0];
                TopLeftPoint = value[1];
                BottomRightPoint = value[2];
                BottomLeftPoint = value[3];
            }
        }

        public PointCollection PointCoordinates
        {
            get
            {
                return new PointCollection() { TopRightCoordinate, TopLeftCoordinate, BottomRightCoordinate, BottomLeftCoordinate };
            }
            set
            {
                TopRightCoordinate = value[0];
                TopLeftCoordinate = value[1];
                BottomRightCoordinate = value[2];
                BottomLeftCoordinate = value[3];
            }
        }

        public ActivityRegion()
        {
            InitializeComponent();
            Region.Fill = Brushes.Red;
        }

        public void UpdateActivityRegion(Point coordinatePoint, int xoffset, int yoffset)
        {
            if (coordinatePoint == TopRightCoordinate)
            {
                TopRightPoint = new Point() { X = TopRightPoint.X + xoffset, Y = TopRightPoint.Y - yoffset };
            }
            if (coordinatePoint == TopLeftCoordinate)
            {
                TopLeftPoint = new Point() { X = TopLeftPoint.X + xoffset, Y = TopLeftPoint.Y - yoffset };
            }
            if (coordinatePoint == BottomLeftCoordinate)
            {
                BottomLeftPoint = new Point() { X = BottomLeftPoint.X + xoffset, Y = BottomLeftPoint.Y - yoffset };
            }
            if (coordinatePoint == BottomRightCoordinate)
            {
                BottomRightPoint = new Point() { X = BottomRightPoint.X + xoffset, Y = BottomRightPoint.Y - yoffset };
            }
            Region.Points = Points;
        }
    }
}
