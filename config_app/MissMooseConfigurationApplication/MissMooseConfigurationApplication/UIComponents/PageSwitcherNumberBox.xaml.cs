using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
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
    public partial class PageSwitcherNumberBox
    {
        private String title;

        public enum WarningStates
        {
            Ok,
            Warning,
            Error
        };

        public PageSwitcherNumberBox()
        {
            InitializeComponent();
        }

        public String Title
        {
            set
            {
                TitleText.Text = value;
                title = value;
            }
        }

        public bool ClickEnabled
        {
            set
            {
                TitleButton.IsEnabled = value;
            }
        }

        public void SetWarningState(WarningStates state)
        {
            switch(state)
            {
                case WarningStates.Warning:
                    NumberField.Background = Brushes.Yellow;
                    break;
                case WarningStates.Error:
                    NumberField.Background = Brushes.Red;
                    break;
                default:
                    NumberField.Background = Brushes.White;
                    break;
            }
        }

        private void PageSwitchClick(object sender, EventArgs e)
        {
            // pages related to this component not yet implemented

            // navigate to new page
            //MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
            //PageSwitcherButton falseSender = new PageSwitcherButton();
            //falseSender.Name = Regex.Replace(title, @"\s+", "");
            //parentWindow.PageSwitchClick(falseSender);
        }
    }
}
