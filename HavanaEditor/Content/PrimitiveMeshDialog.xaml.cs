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
using System.Windows.Shapes;

namespace HavanaEditor.Content
{
    /// <summary>
    /// Interaction logic for PrimitiveMeshDialog.xaml
    /// </summary>
    public partial class PrimitiveMeshDialog : Window
    {
        public PrimitiveMeshDialog()
        {
            InitializeComponent();
        }

        private void OnPrimitiveType_Combobox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

        }

        private void OnSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {

        }
    }
}
