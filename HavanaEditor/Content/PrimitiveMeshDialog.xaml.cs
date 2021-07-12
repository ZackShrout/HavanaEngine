using HavanaEditor.ContentToolsAPIStructs;
using HavanaEditor.DllWrapper;
using HavanaEditor.Editors;
using HavanaEditor.Utilities.Controls;
using System;
using System.Collections.Generic;
using System.IO;
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
        // STATE
        private static readonly List<ImageBrush> textures = new List<ImageBrush>();
        
        // PUBLIC
        public PrimitiveMeshDialog()
        {
            InitializeComponent();
            Loaded += (s, e) => UpdatePrimitive();
        }
        
        static PrimitiveMeshDialog()
        {
            LoadTextures();
        }

        // PRIVATE
        private static void LoadTextures()
        {
            List<Uri> uris = new List<Uri>
            {
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/PlaneTexture.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/UVTest1.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/Checkermap.png"),
            };

            textures.Clear();

            foreach (var uri in uris)
            {
                var resource = Application.GetResourceStream(uri);
                using BinaryReader reader = new BinaryReader(resource.Stream);
                byte[] data = reader.ReadBytes((int)resource.Stream.Length);
                BitmapSource imageSource = (BitmapSource)new ImageSourceConverter().ConvertFrom(data);
                imageSource.Freeze();
                ImageBrush brush = new ImageBrush(imageSource);
                brush.Transform = new ScaleTransform(1, -1, 0.5, 0.5);
                brush.ViewportUnits = BrushMappingMode.Absolute;
                brush.Freeze();
                textures.Add(brush);
            }
        }

        private void OnPrimitiveType_Combobox_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdatePrimitive();

        private void OnSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e) => UpdatePrimitive();

        private void OnScalarBox_ValueChanged(object sender, RoutedEventArgs e) => UpdatePrimitive();

        private void UpdatePrimitive()
        {
            if (!IsInitialized) return;

            PrimitiveMeshType primitiveType = (PrimitiveMeshType)primTypeComboBox.SelectedItem;
            PrimitiveInitInfo info = new PrimitiveInitInfo() { Type = primitiveType };
            int smoothingAngle = 0;

            switch (primitiveType)
            {
                case PrimitiveMeshType.Plane:
                    info.SegmentX = (int)xSliderPlane.Value;
                    info.SegmentZ = (int)zSliderPlane.Value;
                    info.Size.X = Value(widthScalarBoxPlane, 0.001f);
                    info.Size.Z = Value(lengthScalarBoxPlane, 0.001f);
                    break;
                case PrimitiveMeshType.Cube:
                    return;
                case PrimitiveMeshType.UVSphere:
                    info.SegmentX = (int)xSliderUVSphere.Value;
                    info.SegmentY = (int)ySliderUVSphere.Value;
                    info.Size.X = Value(xScalarBoxUVSphere, 0.001f);
                    info.Size.Y = Value(yScalarBoxUVSphere, 0.001f);
                    info.Size.Z = Value(zScalarBoxUVSphere, 0.001f);
                    smoothingAngle = (int)angleSliderUVSphere.Value;
                    break;
                case PrimitiveMeshType.ICOSphere:
                    return;
                case PrimitiveMeshType.Cylinder:
                    return;
                case PrimitiveMeshType.Capsule:
                    return;
                default:
                    break;
            }

            Geometry geometry = new Geometry();
            geometry.ImportSettings.SmoothingAngle = smoothingAngle;
            ContentToolsAPI.CreatePrimitiveMesh(geometry, info);
            (DataContext as GeometryEditor).SetAsset(geometry);
            OnTexture_Checkbox_Click(textureCheckBox, null);
        }

        private float Value(ScalarBox scalarBox, float min)
        {
            float.TryParse(scalarBox.Value, out float result);
            return Math.Max(result, min);
        }

        private void OnTexture_Checkbox_Click(object sender, RoutedEventArgs e)
        {
            Brush brush = Brushes.White;

            if ((sender as CheckBox).IsChecked == true)
            {
                brush = textures[(int)primTypeComboBox.SelectedItem];
            }

            GeometryEditor viewModel = DataContext as GeometryEditor;
            foreach (var mesh in viewModel.MeshRenderer.Meshes)
            {
                mesh.Diffuse = brush;
            }
        }
    }
}
