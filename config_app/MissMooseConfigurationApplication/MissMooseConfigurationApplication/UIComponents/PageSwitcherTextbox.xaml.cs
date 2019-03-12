using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
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
        private ScrollViewer scrollViewer;

        #region Public Methods

        public PageSwitcherTextbox()
        {
            InitializeComponent();

            this.Loaded += OnLoaded;
        }        

        public String Title
        {
            set
            {
                TitleButton.Content = value;
                title = value;
            }
        }

        #endregion

        #region Private Methods

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            Border border = (Border)VisualTreeHelper.GetChild(TextList, 0);
            scrollViewer = (ScrollViewer)VisualTreeHelper.GetChild(border, 0);
            //scrollViewer.Style = (Style)FindResource("PrettyScrollViewer");
            scrollViewer.ScrollToBottom();

            ((INotifyCollectionChanged)TextList.Items).CollectionChanged += OnTextListChanged;
        }

        private void PageSwitchClick(object sender, EventArgs e)
        {
            // navigate to new page
            MainWindow parentWindow = Window.GetWindow(this) as MainWindow;
            PageSwitcherButton falseSender = new PageSwitcherButton();
            falseSender.Name = Regex.Replace(title, @"\s+", "");
            parentWindow.PageSwitchClick(falseSender);
        }

        private void OnTextListChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.Action == NotifyCollectionChangedAction.Add)
            {
                double visibleItemsStartIndex = scrollViewer.VerticalOffset;
                double numVisibleItems = scrollViewer.ViewportHeight;

                if ((visibleItemsStartIndex + numVisibleItems) >= (TextList.Items.Count - 1))
                {
                    scrollViewer.ScrollToBottom();
                }
            }
        }

        #endregion
    }
}
