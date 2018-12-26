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
using MissMooseConfigurationApplication.UIPages;
using System.Drawing;

namespace MissMooseConfigurationApplication
{
    public partial class PageSwitcherButton : Button
    {
        #region Private Members

        private Brush darkBackground = (SolidColorBrush)(new BrushConverter().ConvertFrom("#2e2e2e"));

        #endregion

        #region Public Methods

        public PageSwitcherButton()
        {
            InitializeComponent();
        }

        public void SetButtonColour(bool isClicked)
        {
            this.Background = isClicked ? Brushes.LightGray : darkBackground;
            this.Foreground = isClicked ? Brushes.Black : Brushes.White;
            this.BorderBrush = isClicked ? darkBackground : Brushes.Black;
        }

        #endregion

        #region Private Methods

        private void PageSwitcherButtonClick(object sender, EventArgs e)
        {
            // navigate to new page
            MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
            parentWindow.PageSwitchClick(sender as PageSwitcherButton);
        }

        #endregion
    }
}
