using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace HavanaEditor.Editors
{
    /// <summary>
    /// Interaction logic for GeometryView.xaml
    /// </summary>
    public partial class GeometryView : UserControl
    {
        // STATE
        private Point _clickedPosition;
        private bool _capturedLeft;
        private bool _capturedRight;
        // NOTE: used for capturing the mesh as an icon for it's asset file
        private static readonly GeometryView _geometryView = new GeometryView() { Background = (Brush)Application.Current.FindResource("Editor.Window.GreyBrush4") };

        // PUBLIC
        public GeometryView()
        {
            InitializeComponent();
            DataContextChanged += (s, e) => SetGeometry();
        }

        /// <summary>
        /// Set the geometry to display in the viewer
        /// </summary>
        /// <param name="index"> - Which mesh LoD to display. Default is -1, which diplays everything.</param>
        public void SetGeometry(int index = -1)
        {
            if (!(DataContext is MeshRenderer viewModel)) return;

            if (viewModel.Meshes.Any() && viewport.Children.Count == 2)
            {
                viewport.Children.RemoveAt(1); // remove old geometry - not the lights
            }

            int meshIndex = 0;
            Model3DGroup modelGroup = new Model3DGroup();

            foreach (var mesh in viewModel.Meshes)
            {
                // Skip over meshed we don't want to display
                if (index != -1 && meshIndex != index)
                {
                    meshIndex++;
                    continue;
                }

                MeshGeometry3D mesh3D = new MeshGeometry3D()
                {
                    Positions = mesh.Positions,
                    Normals = mesh.Normals,
                    TriangleIndices = mesh.Indices,
                    TextureCoordinates = mesh.UVs
                };
                DiffuseMaterial diffuse = new DiffuseMaterial(mesh.Diffuse);
                SpecularMaterial specular = new SpecularMaterial(mesh.Specular, 50);
                MaterialGroup matGroup = new MaterialGroup();
                matGroup.Children.Add(diffuse);
                matGroup.Children.Add(specular);
                GeometryModel3D model = new GeometryModel3D(mesh3D, matGroup);
                modelGroup.Children.Add(model);

                Binding binding = new Binding(nameof(mesh.Diffuse)) { Source = mesh };
                BindingOperations.SetBinding(diffuse, DiffuseMaterial.BrushProperty, binding);

                if (meshIndex == index) break;
            }

            ModelVisual3D visual = new ModelVisual3D() { Content = modelGroup };
            viewport.Children.Add(visual);
        }
        
        // INTERNAL
        internal static BitmapSource RenderToBitmap(MeshRenderer mesh, int width, int height)
        {
            RenderTargetBitmap bmp = new RenderTargetBitmap(width, height, 96, 96, PixelFormats.Default);

            _geometryView.DataContext = mesh;
            _geometryView.Width = width;
            _geometryView.Height = height;
            _geometryView.Measure(new Size(width, height));
            _geometryView.Arrange(new Rect(0, 0, width, height));
            _geometryView.UpdateLayout();

            bmp.Render(_geometryView);
            return bmp;
        }

        // PRIVATE
        private void OnGrid_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _clickedPosition = e.GetPosition(this);
            _capturedLeft = true;
            Mouse.Capture(sender as UIElement);
        }

        private void OnGrid_Mouse_Move(object sender, MouseEventArgs e)
        {
            if (!_capturedLeft && !_capturedRight) return;

            Point position = e.GetPosition(this);
            Vector difference = position - _clickedPosition;

            if (_capturedLeft && !_capturedRight)
            {
                MoveCamera(difference.X, difference.Y, 0);
            }
            else if (!_capturedLeft && _capturedRight)
            {
                MeshRenderer viewModel = DataContext as MeshRenderer;
                Point3D cameraPosition = viewModel.CameraPosition;
                double yOffset = difference.Y * 0.001 * Math.Sqrt((cameraPosition.X * cameraPosition.X) + (cameraPosition.Z * cameraPosition.Z));
                viewModel.CameraTarget = new Point3D(viewModel.CameraTarget.X, viewModel.CameraTarget.Y + yOffset, viewModel.CameraTarget.Z);
            }

            _clickedPosition = position;
        }

        private void OnGrid_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            _capturedLeft = false;

            if (!_capturedRight) Mouse.Capture(null);
        }

        private void OnGrid_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            MoveCamera(0, 0, Math.Sign(e.Delta));
        }

        private void OnGrid_Mouse_RBD(object sender, MouseButtonEventArgs e)
        {
            _clickedPosition = e.GetPosition(this);
            _capturedRight = true;
            Mouse.Capture(sender as UIElement);
        }

        private void OnGrid_Mouse_RBU(object sender, MouseButtonEventArgs e)
        {
            _capturedRight = false;

            if (!_capturedLeft) Mouse.Capture(null);
        }
        
        private void MoveCamera(double dx, double dy, int dz)
        {
            MeshRenderer viewModel = DataContext as MeshRenderer;
            Vector3D vector = new Vector3D(viewModel.CameraPosition.X, viewModel.CameraPosition.Y, viewModel.CameraPosition.Z);

            double r = vector.Length;
            double theta = Math.Acos(vector.Y / r);
            double phi = Math.Atan2(-vector.Z, vector.X);

            theta -= dy * 0.01;
            phi -= dx * 0.01;
            r *= 1.0 - 0.1 * dz; // dz is either +1 or -1
            theta = Math.Clamp(theta, 0.0001, Math.PI - 0.0001);

            vector.X = r * Math.Sin(theta) * Math.Cos(phi);
            vector.Y = r * Math.Cos(theta);
            vector.Z = -r * Math.Sin(theta) * Math.Sin(phi);

            viewModel.CameraPosition = new Point3D(vector.X, vector.Y, vector.Z);
        }
    }
}
