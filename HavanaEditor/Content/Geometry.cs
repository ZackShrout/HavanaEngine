using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Text;

namespace HavanaEditor.Content
{
    enum PrimitiveMeshType
    {
        Plane,
        Cube,
        UVSphere,
        ICOSphere,
        Cylinder,
        Capsule
    }

    class Mesh : ViewModelBase
    {
        // STATE
        private int vertexSize;
        private int vertexCount;
        private int indexSize;
        private int indexCount;

        // PROPERTIES
        public int VertexSize
        {
            get => vertexSize;
            set
            {
                if (vertexSize != value)
                {
                    vertexSize = value;
                    OnPropertyChanged(nameof(VertexSize));
                }
            }
        }
        public int VertexCount
        {
            get => vertexCount;
            set
            {
                if (vertexCount != value)
                {
                    vertexCount = value;
                    OnPropertyChanged(nameof(VertexCount));
                }
            }
        }
        public int IndexSize
        {
            get => indexSize;
            set
            {
                if (indexSize != value)
                {
                    indexSize = value;
                    OnPropertyChanged(nameof(IndexSize));
                }
            }
        }
        public int IndexCount
        {
            get => indexCount;
            set
            {
                if (indexCount != value)
                {
                    indexCount = value;
                    OnPropertyChanged(nameof(IndexCount));
                }
            }
        }
        public byte[] Vertices { get; set; }
        public byte[] Indices { get; set; }
    }
    
    class MeshLoD : ViewModelBase
    {
        // STATE
        private string name;
        private float lodThreshold;

        // PROPERTIES
        public string Name
        {
            get => name;
            set
            {
                if (name != value)
                {
                    name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        public float LoDThreshold
        {
            get => lodThreshold;
            set
            {
                if (lodThreshold != value)
                {
                    lodThreshold = value;
                    OnPropertyChanged(nameof(LoDThreshold));
                }
            }
        }
        public ObservableCollection<Mesh> Meshes { get; } = new ObservableCollection<Mesh>();

    }

    class LoDGroup : ViewModelBase
    {
        // STATE
        private string name;

        // PROPERTIES
        public string Name
        {
            get => name;
            set
            {
                if (name != value)
                {
                    name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        public ObservableCollection<MeshLoD> LoDs { get; } = new ObservableCollection<MeshLoD>();
    }
    
    class Geometry : Asset
    {
        // PUBLIC
        public Geometry() : base(AssetType.Mesh)
        {
        }

        // INTERNAL
        internal void FromRawData(byte[] data)
        {
            Debug.Assert(data?.Length > 0);

        }
    }
}
