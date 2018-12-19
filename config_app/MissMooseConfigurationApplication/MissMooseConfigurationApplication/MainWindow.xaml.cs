using ANT_Managed_Library;
using System;
using System.Collections.Generic;
using System.ComponentModel;
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

namespace MissMooseConfigurationApplication
{
    enum SystemStatusEnum
    {
        SystemStatus_OK
    }

    public partial class MainWindow : Window
    {
        #region Private Members

        private Dictionary<PageSwitcherButton, Page> navigationItems;

        #endregion

        #region Private Delegates

        #endregion

        #region Public Properties

        #endregion

        #region Public Events

        #endregion

        #region Public Methods

        public MainWindow()
        {
            InitializeComponent();

            SetSystemStatusLabel(SystemStatusEnum.SystemStatus_OK);

            // main window owns all these pages so that state is maintained when switching between them
            navigationItems = new Dictionary<PageSwitcherButton, Page>()
            {
                { ConfigPageButton, new ConfigurationPage() },
                { EventLogPageButton, new EventLogPage() },
                { SystemProbsPageButton, new SystemProblemsPage()},
                { SystemOverviewPageButton, new SystemOverviewPage()}
            };
            // app starts on config screen
            PageSwitchClick(ConfigPageButton);
        }

        public void PageSwitchClick(PageSwitcherButton sender)
        {
            foreach ( KeyValuePair<PageSwitcherButton, Page> item in navigationItems)
            {
                if (item.Key.Name.Equals(sender.Name))
                {
                    item.Key.SetButtonColour(true);
                    PageFrame.Navigate(item.Value);
                }
                else
                {
                    item.Key.SetButtonColour(false);
                }
            }
        }

        #endregion

        #region Private Methods

        void SetSystemStatusLabel(SystemStatusEnum status)
        {
            string content = "System Status: ";

            switch(status)
            {
                case SystemStatusEnum.SystemStatus_OK:
                    content += "OK";
                    break;
            }

            label_SystemStatusBar.Content = content;
        }
      
        #endregion
    }
}
