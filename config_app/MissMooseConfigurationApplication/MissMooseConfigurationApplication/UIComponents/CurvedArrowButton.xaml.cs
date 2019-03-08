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
    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class CurvedArrowButton : Button
    {
        private bool counterClockwise = false;

        public bool IsCounterClockwise
        {
            get { return counterClockwise; }
            set { counterClockwise = value; }
        }

        public CurvedArrowButton()
        {
            this.Loaded += OnLoaded;

            InitializeComponent();
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            if (IsCounterClockwise)
            {
                FlowDirection = FlowDirection.RightToLeft;
            }
        }
    }
}
