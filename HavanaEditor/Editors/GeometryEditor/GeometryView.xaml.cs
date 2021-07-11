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
        private Point clickedPosition;
        private bool capturedLeft;
        private bool capturedRight;

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

        // PRIVATE
        private void OnGrid_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            clickedPosition = e.GetPosition(this);
            capturedLeft = true;
            Mouse.Capture(sender as UIElement);
        }

        private void OnGrid_Mouse_Move(object sender, MouseEventArgs e)
        {
            if (!capturedLeft && !capturedRight) return;

            Point position = e.GetPosition(this);
            Vector difference = position - clickedPosition;

            if (capturedLeft && !capturedRight)
            {
                MoveCamera(difference.X, difference.Y, 0);
            }
            else if (!capturedLeft && capturedRight)
            {
                MeshRenderer viewModel = DataContext as MeshRenderer;
                Point3D cameraPosition = viewModel.CameraPosition;
                double yOffset = difference.Y * 0.001 * Math.Sqrt((cameraPosition.X * cameraPosition.X) + (cameraPosition.Z * cameraPosition.Z));
                viewModel.CameraTarget = new Point3D(viewModel.CameraTarget.X, viewModel.CameraTarget.Y + yOffset, viewModel.CameraTarget.Z);
            }
        }

        private void OnGrid_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            capturedLeft = false;

            if (!capturedRight) Mouse.Capture(null);
        }

        private void OnGrid_MouseWheel(object sender, MouseWheelEventArgs e)
        {

        }

        private void OnGrid_Mouse_RBD(object sender, MouseButtonEventArgs e)
        {

        }

        private void OnGrid_Mouse_RBU(object sender, MouseButtonEventArgs e)
        {

        }
        
        private void MoveCamera(double x, double y, int v)
        {
            // TODO: implement
        }
    }
}
