using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace HavanaEditor.Editors
{
    /// <summary>
    /// Interaction logic for ComponentView.xaml
    /// </summary>
    [ContentProperty("ComponentContent")]
    public partial class ComponentView : UserControl
    {

        // PROPERTIES
        public string Header
        {
            get { return (string)GetValue(HeaderProperty); }
            set { SetValue(HeaderProperty, value); }
        }
        public FrameworkElement ComponentContent
        {
            get { return (FrameworkElement)GetValue(ComponentContentProperty); }
            set { SetValue(ComponentContentProperty, value); }
        }

        // CONFIG DATA
        public static readonly DependencyProperty HeaderProperty =
            DependencyProperty.Register(nameof(Header), typeof(string), typeof(ComponentView));
        public static readonly DependencyProperty ComponentContentProperty =
            DependencyProperty.Register(nameof(ComponentContent), typeof(FrameworkElement), typeof(ComponentView));



        public ComponentView()
        {
            InitializeComponent();
        }

        private void OnExpander_Mouse_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            Logger.Log(MessageTypes.Info, $"You double clicked on {sender}");
        }
    }
}
