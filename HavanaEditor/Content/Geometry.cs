using HavanaEditor.Common;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
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
        // STATE
        private readonly List<LoDGroup> lodGroups = new List<LoDGroup>();
        
        // PUBLIC
        public Geometry() : base(AssetType.Mesh)
        {
        }
        
        public void FromRawData(byte[] data)
        {
            Debug.Assert(data?.Length > 0);

            lodGroups.Clear();

            using BinaryReader reader = new BinaryReader(new MemoryStream(data));
            // Skip scene name string
            int s = reader.ReadInt32();
            reader.BaseStream.Position += s;
            // Get number of LoDs
            int numLoDGroups = reader.ReadInt32();
            Debug.Assert(numLoDGroups > 0);

            // Read data for each LoD
            for (int i = 0; i < numLoDGroups; i++)
            {
                // Get this LoD group name
                s = reader.ReadInt32();
                string lodGroupName;
                if (s > 0)
                {
                    byte[] nameBytes = reader.ReadBytes(s);
                    lodGroupName = Encoding.UTF8.GetString(nameBytes);
                }
                else
                {
                    lodGroupName = $"lod_{ContentHelper.GetRandomString()}";
                }

                // Get number of meshes in this LoD group
                int numMeshes = reader.ReadInt32();
                Debug.Assert(numMeshes > 0);
                List<MeshLoD> lods = ReadMeshLoDs(numMeshes, reader);

                LoDGroup lodGroup = new LoDGroup() { Name = lodGroupName };
                lods.ForEach(l => lodGroup.LoDs.Add(l));
                lodGroups.Add(lodGroup);
            }
        }

        public LoDGroup GetLoDGroup(int lodGroup = 0)
        {
            Debug.Assert(lodGroup >= 0 && lodGroup < lodGroups.Count);

            return lodGroups.Any() ? lodGroups[lodGroup] : null;
        }

        // PRIVATE
        private static List<MeshLoD> ReadMeshLoDs(int numMeshes, BinaryReader reader)
        {
            List<int> lodIDs = new List<int>();
            List<MeshLoD> lodList = new List<MeshLoD>();
            for (int i = 0; i < numMeshes; i++)
            {
                ReadMesh(reader, lodIDs, lodList);
            }

            return lodList;
        }

        private static void ReadMesh(BinaryReader reader, List<int> lodIDs, List<MeshLoD> lodList)
        {
            // Get mesh name
            int s = reader.ReadInt32();
            string meshName;
            
            if (s > 0)
            {
                byte[] nameBytes = reader.ReadBytes(s);
                meshName = Encoding.UTF8.GetString(nameBytes);
            }
            else
            {
                meshName = $"mesh_{ContentHelper.GetRandomString()}";
            }

            Mesh mesh = new Mesh();
            int lodID = reader.ReadInt32();
            mesh.VertexSize = reader.ReadInt32();
            mesh.VertexCount = reader.ReadInt32();
            mesh.IndexSize = reader.ReadInt32();
            mesh.IndexCount = reader.ReadInt32();
            float lodThreshold = reader.ReadSingle();

            int vertexBufferSize = mesh.VertexSize * mesh.VertexCount;
            int indexBufferSize = mesh.IndexSize * mesh.IndexCount;

            mesh.Vertices = reader.ReadBytes(vertexBufferSize);
            mesh.Indices = reader.ReadBytes(indexBufferSize);

            MeshLoD lod;
            if (ID.IsValid(lodID) && lodIDs.Contains(lodID))
            {
                lod = lodList[lodIDs.IndexOf(lodID)];
                Debug.Assert(lod != null);
            }
            else
            {
                lodIDs.Add(lodID);
                lod = new MeshLoD() { Name = meshName, LoDThreshold = lodThreshold };
                lodList.Add(lod);
            }

            lod.Meshes.Add(mesh);
        }
    }
}
