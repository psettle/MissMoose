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
    /// Interaction logic for LineWithBorder.xaml
    /// </summary>
    public partial class LineWithBorder : UserControl
    {
        private bool lineIsVertical = false;
        public int X1
        {
            set
            {
                BorderLine.X1 += value;
                ColoredLine.X1 += value;
            }
        }

        public int X2
        {
            set
            {
                BorderLine.X2 += value;
                ColoredLine.X2 += value;
            }
        }

        public int Y1
        {
            set
            {
                BorderLine.Y1 += value;
                ColoredLine.Y1 += value;
            }
        }

        public int Y2
        {
            set
            {
                BorderLine.Y2 += value;
                ColoredLine.Y2 += value;
            }
        }

        public int Thickness
        {
            set
            {
                BorderLine.StrokeThickness = value;
                ColoredLine.StrokeThickness = value - 4;
            }
        }

        public bool isVertical
        {
            set
            {
                lineIsVertical = value;
                if (isVertical)
                {
                    ColoredLine.Y1 -= 3;
                    ColoredLine.Y2 += 3;
                    ColoredLine.UpdateLayout();
                }
                else
                {
                    ColoredLine.X1 -= 3;
                    ColoredLine.X2 += 3;
                    ColoredLine.UpdateLayout();
                }
            }
            get
            {
                return lineIsVertical;
            }
        }

        public LineWithBorder()
        {
            InitializeComponent();
        }
    }
}
