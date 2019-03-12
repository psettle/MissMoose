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
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using MissMooseConfigurationApplication.UIPages;

namespace MissMooseConfigurationApplication.UIComponents
{
    public partial class PageSwitcherButton : Button
    {
        #region Public Methods

        public PageSwitcherButton()
        {
            InitializeComponent();
        }

        public void SetButtonColour(bool isClicked)
        {
            this.Effect = isClicked ?
                new DropShadowEffect
                {
                    Direction = 320,
                    ShadowDepth = 2,
                    BlurRadius = 5
                }
                :
                null;

            this.Opacity = isClicked ? 1.0 : 0.75;
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
