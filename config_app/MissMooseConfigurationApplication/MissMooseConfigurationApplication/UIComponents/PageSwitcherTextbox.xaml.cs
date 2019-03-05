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
    public partial class PageSwitcherTextbox
    {
        private String title;

        public PageSwitcherTextbox()
        {
            InitializeComponent();
        }

        public String Title
        {
            set
            {
                TitleButton.Content = value;
                title = value;
            }
        }

        private void PageSwitchClick(object sender, EventArgs e)
        {
            // navigate to new page
            MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
            PageSwitcherButton falseSender = new PageSwitcherButton();
            falseSender.Name = Regex.Replace(title, @"\s+", "");
            parentWindow.PageSwitchClick(falseSender);
        }

        public void UpdateScrollBar()
        {
            if (VisualTreeHelper.GetChildrenCount(TextList) > 0)
            {
                Border border = (Border)VisualTreeHelper.GetChild(TextList, 0);
                ScrollViewer scrollViewer = (ScrollViewer)VisualTreeHelper.GetChild(border, 0);
                scrollViewer.ScrollToBottom();
            }
        }
    }
}
