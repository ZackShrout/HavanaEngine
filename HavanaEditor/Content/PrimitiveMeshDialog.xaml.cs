using HavanaEditor.ContentToolsAPIStructs;
using HavanaEditor.DllWrapper;
using HavanaEditor.Editors;
using HavanaEditor.Utilities.Controls;
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
            Loaded += (s, e) => UpdatePrimitive();
        }

        private void OnPrimitiveType_Combobox_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdatePrimitive();

        private void OnSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e) => UpdatePrimitive();

        private void OnScalarBox_ValueChanged(object sender, RoutedEventArgs e) => UpdatePrimitive();

        private void UpdatePrimitive()
        {
            if (!IsInitialized) return;

            PrimitiveMeshType primitiveType = (PrimitiveMeshType)primTypeComboBox.SelectedItem;
            PrimitiveInitInfo info = new PrimitiveInitInfo() { Type = primitiveType };

            switch (primitiveType)
            {
                case PrimitiveMeshType.Plane:
                    info.SegmentX = (int)xSliderPlane.Value;
                    info.SegmentZ = (int)zSliderPlane.Value;
                    info.Size.X = Value(widthScalarBoxPlane, 0.001f);
                    info.Size.Z = Value(lengthScalarBoxPlane, 0.001f);
                    break;
                case PrimitiveMeshType.Cube:
                    break;
                case PrimitiveMeshType.UVSphere:
                    break;
                case PrimitiveMeshType.ICOSphere:
                    break;
                case PrimitiveMeshType.Cylinder:
                    break;
                case PrimitiveMeshType.Capsule:
                    break;
                default:
                    break;
            }

            Geometry geometry = new Geometry();
            ContentToolsAPI.CreatePrimitiveMesh(geometry, info);
            (DataContext as GeometryEditor).SetAsset(geometry);
        }

        private float Value(ScalarBox scalarBox, float min)
        {
            float.TryParse(scalarBox.Value, out float result);
            return Math.Max(result, min);
        }
    }
}
