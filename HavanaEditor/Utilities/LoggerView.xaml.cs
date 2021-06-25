using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace HavanaEditor.Utilities
{
    /// <summary>
    /// Interaction logic for LoggerView.xaml
    /// </summary>
    public partial class LoggerView : UserControl
    {
        public LoggerView()
        {
            InitializeComponent();           
        }

        private void OnClear_Button_Click(object sender, RoutedEventArgs e)
        {
            Logger.Clear();
        }

        private void OnMessageFilter_Button_Click(object sender, RoutedEventArgs e)
        {
            int filter = 0x0;
            if (toggleInfo.IsChecked == true)
            {
                filter |= (int)MessageTypes.Info;
            }
            if (toggleWarning.IsChecked == true)
            {
                filter |= (int)MessageTypes.Warning;
            }
            if (toggleError.IsChecked == true)
            {
                filter |= (int)MessageTypes.Error;
            }
            Logger.SetFilter(filter);
        }
    }
}
