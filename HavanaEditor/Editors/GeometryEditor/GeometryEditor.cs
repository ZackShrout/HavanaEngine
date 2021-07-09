using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Text;
using System.Windows.Media;
using System.Windows.Media.Media3D;

namespace HavanaEditor.Editors
{
    // NOTE: the purpose of this class is to enable 3D geometry in WPF prior to
    // having a graphics renderer in the game engine. Once there is a renderer
    // in the game engine, this class and the WPF viewer will be obsolete.
    class MeshRendererVertexData : ViewModelBase
    {

        private Brush specular = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#ff111111"));
        public Brush Specular
        {
            get => specular;
            set
            {
                if (specular != value)
                {
                    specular = value;
                    OnPropertyChanged(nameof(Specular));
                }
            }
        }

        private Brush diffuse = Brushes.White;
        public Brush Diffuse
        {
            get => diffuse;
            set
            {
                if (diffuse != value)
                {
                    diffuse = value;
                    OnPropertyChanged(nameof(Diffuse));
                }
            }
        }

        public Point3DCollection Positions { get; } = new Point3DCollection();
        public Vector3DCollection Normals { get; } = new Vector3DCollection();
        public PointCollection UVs { get; } = new PointCollection();
        public Int32Collection Indices { get; } = new Int32Collection();

    }

    // NOTE: the purpose of this class is to enable 3D geometry in WPF prior to
    // having a graphics renderer in the game engine. Once there is a renderer
    // in the game engine, this class and the WPF viewer will be obsolete.
    class MeshRenderer : ViewModelBase
    {
        public ObservableCollection<MeshRendererVertexData> Meshes { get; } = new ObservableCollection<MeshRendererVertexData>();
    }

    class GeometryEditor : ViewModelBase, IAssetEditor
    {
        // STATE
        private Content.Geometry geometry;

        // PROPERTIES
        public Content.Asset Asset => Geometry;
        public Content.Geometry Geometry
        {
            get => geometry;
            set
            {
                if (geometry != value)
                {
                    geometry = value;
                    OnPropertyChanged(nameof(Geometry));
                }
            }
        }

        // PUBLIC
        public void SetAsset(Content.Asset asset)
        {
            Debug.Assert(asset is Content.Geometry);
            
            if (asset is Content.Geometry geometry)
            {
                Geometry = geometry;
            }
        }
    }
}
