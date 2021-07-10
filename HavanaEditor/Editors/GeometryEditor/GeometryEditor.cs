using HavanaEditor.Content;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
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

        private Vector3D cameraDirections = new Vector3D(0, 0, -10);
        public Vector3D CameraDirections
        {
            get => cameraDirections;
            set
            {
                if (cameraDirections != value)
                {
                    cameraDirections = value;
                    OnPropertyChanged(nameof(CameraDirections));
                }
            }
        }

        private Point3D cameraPosition = new Point3D(0, 0, 10);
        public Point3D CameraPosition
        {
            get => cameraPosition;
            set
            {
                if (cameraPosition != value)
                {
                    cameraPosition = value;
                    OnPropertyChanged(nameof(CameraPosition));
                    OnPropertyChanged(nameof(OffsetCameraPosition));
                }
            }
        }

        private Point3D cameraTarget = new Point3D(0, 0, 0);
        public Point3D CameraTarget
        {
            get => cameraTarget;
            set
            {
                if (cameraTarget != value)
                {
                    cameraTarget = value;
                    OnPropertyChanged(nameof(CameraTarget));
                    OnPropertyChanged(nameof(OffsetCameraPosition));
                }
            }
        }

        public Point3D OffsetCameraPosition => 
            new Point3D(CameraPosition.X + CameraTarget.X, CameraPosition.Y + CameraTarget.Y, CameraPosition.Z + CameraTarget.Z);

        private Color keyLight = (Color)ColorConverter.ConvertFromString("#ffaeaeae");
        public Color KeyLight
        {
            get => keyLight;
            set
            {
                if (keyLight != value)
                {
                    keyLight = value;
                    OnPropertyChanged(nameof(KeyLight));
                }
            }
        }

        private Color skyLight = (Color)ColorConverter.ConvertFromString("#ff111b30");
        public Color SkyLight
        {
            get => skyLight;
            set
            {
                if (skyLight != value)
                {
                    skyLight = value;
                    OnPropertyChanged(nameof(SkyLight));
                }
            }
        }

        private Color groundLight = (Color)ColorConverter.ConvertFromString("#ff3f2f1e");
        public Color GroundLight
        {
            get => groundLight;
            set
            {
                if (groundLight != value)
                {
                    groundLight = value;
                    OnPropertyChanged(nameof(GroundLight));
                }
            }
        }

        private Color ambientLight = (Color)ColorConverter.ConvertFromString("#ff3b3b3b");
        public Color AmbientLight
        {
            get => ambientLight;
            set
            {
                if (ambientLight != value)
                {
                    ambientLight = value;
                    OnPropertyChanged(nameof(AmbientLight));
                }
            }
        }

        public MeshRenderer(MeshLoD lod, MeshRenderer old)
        {
            Debug.Assert(lod?.Meshes.Any() == true);
            // Calculate vertex size minus position and normal vectors
            var offset = lod.Meshes[0].VertexSize - 3 * sizeof(float) - sizeof(int) - 2 * sizeof(short);
            // Figure out bounding box of the object being rendered
            double minX, minY, minZ; minX = minY = minZ = double.MaxValue;
            double maxX, maxY, maxZ; maxX = maxY = maxZ = double.MinValue;
            Vector3D avgNormal = new Vector3D();
            // Unpack the packed normals
            var intervals = 2.0f / ((1 << 16) - 1);
            foreach (var mesh in lod.Meshes)
            {
                var vertexData = new MeshRendererVertexData();
                // Unpack all vertices
                using (var reader = new BinaryReader(new MemoryStream(mesh.Vertices)))
                {
                    for (int i = 0; i < mesh.VertexCount; i++)
                    {
                        // Read positions
                        var posX = reader.ReadSingle();
                        var posY = reader.ReadSingle();
                        var posZ = reader.ReadSingle();
                        var signs = (reader.ReadUInt32() >> 24) & 0x000000ff;
                        vertexData.Positions.Add(new Point3D(posX, posY, posZ));

                        // Adjust bounding box
                        minX = Math.Min(minX, posX); maxX = Math.Max(maxX, posX);
                        minY = Math.Min(minY, posY); maxY = Math.Max(maxY, posY);
                        minZ = Math.Min(minZ, posZ); maxZ = Math.Max(maxZ, posZ);

                        // Read normals
                        var nrmX = reader.ReadUInt16() * intervals - 1.0f;
                        var nrmY = reader.ReadUInt16() * intervals - 1.0f;
                        var nrmZ = Math.Sqrt(Math.Clamp(1f - (nrmX * nrmX + nrmY * nrmY), 0f, 1f)) * ((signs & 0x2) - 1.0f);
                        var normal = new Vector3D(nrmX, nrmY, nrmZ);
                        normal.Normalize();
                        vertexData.Normals.Add(normal);
                        avgNormal += normal;

                        // Read UVs (skip tangent and joint data)
                        reader.BaseStream.Position += (offset - sizeof(float) * 2);
                        var u = reader.ReadSingle();
                        var v = reader.ReadSingle();
                        vertexData.UVs.Add(new Point(u, v));
                    }
                }
                // Unpack all indices
                using(var reader = new BinaryReader(new MemoryStream(mesh.Indices)))
                {
                    if (mesh.IndexSize == sizeof(short))
                        for (int i = 0; i < mesh.IndexCount; i++)
                            vertexData.Indices.Add(reader.ReadUInt16());
                    else
                        for (int i = 0; i < mesh.IndexCount; i++)
                            vertexData.Indices.Add(reader.ReadInt32());
                }

                vertexData.Positions.Freeze();
                vertexData.Normals.Freeze();
                vertexData.UVs.Freeze();
                vertexData.Indices.Freeze();
                Meshes.Add(vertexData);
            }

            // Set camera target and position
            if (old != null)
            {
                CameraTarget = old.CameraTarget;
                CameraPosition = old.CameraPosition;
            }
            else
            {
                // Compute bounding box dimensions
                var width = maxX - minX;
                var height = maxY - minY;
                var depth = maxZ - minZ;
                var radius = new Vector3D(height, width, depth).Length * 1.2;
                if (avgNormal.Length > 0.8)
                {
                    avgNormal.Normalize();
                    avgNormal *= radius;
                    CameraPosition = new Point3D(avgNormal.X, avgNormal.Y, avgNormal.Z);
                }
                else
                {
                    CameraPosition = new Point3D(width, height * 0.5, radius);
                }

                CameraTarget = new Point3D(minX + width * 0.5, minY + height * 0.5, minZ + depth * 0.5);
            }
        }
    }

    class GeometryEditor : ViewModelBase, IAssetEditor
    {
        // STATE
        private Content.Geometry geometry;
        private MeshRenderer meshRenderer;

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
        public MeshRenderer MeshRenderer
        {
            get => meshRenderer;
            set
            {
                if (meshRenderer != value)
                {
                    meshRenderer = value;
                    OnPropertyChanged(nameof(MeshRenderer));
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
                MeshRenderer = new MeshRenderer(Geometry.GetLoDGroup().LoDs[0], MeshRenderer);
            }
        }
    }
}
