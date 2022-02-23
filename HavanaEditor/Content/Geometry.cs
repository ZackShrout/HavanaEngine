using HavanaEditor.Common;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

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
        private int _vertexSize;
        private int _vertexCount;
        private int _indexSize;
        private int _indexCount;

        // PROPERTIES
        public int VertexSize
        {
            get => _vertexSize;
            set
            {
                if (_vertexSize != value)
                {
                    _vertexSize = value;
                    OnPropertyChanged(nameof(VertexSize));
                }
            }
        }
        public int VertexCount
        {
            get => _vertexCount;
            set
            {
                if (_vertexCount != value)
                {
                    _vertexCount = value;
                    OnPropertyChanged(nameof(VertexCount));
                }
            }
        }
        public int IndexSize
        {
            get => _indexSize;
            set
            {
                if (_indexSize != value)
                {
                    _indexSize = value;
                    OnPropertyChanged(nameof(IndexSize));
                }
            }
        }
        public int IndexCount
        {
            get => _indexCount;
            set
            {
                if (_indexCount != value)
                {
                    _indexCount = value;
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
        private string _name;
        private float _lodThreshold;

        // PROPERTIES
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        public float LoDThreshold
        {
            get => _lodThreshold;
            set
            {
                if (_lodThreshold != value)
                {
                    _lodThreshold = value;
                    OnPropertyChanged(nameof(LoDThreshold));
                }
            }
        }
        public ObservableCollection<Mesh> Meshes { get; } = new ObservableCollection<Mesh>();

    }

    class LoDGroup : ViewModelBase
    {
        // STATE
        private string _name;

        // PROPERTIES
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        public ObservableCollection<MeshLoD> LoDs { get; } = new ObservableCollection<MeshLoD>();
    }

    class GeometryImportSettings : ViewModelBase
    {
        // STATE
        private float _smoothingAngle;
        private bool _calculateNormals;
        private bool _calculateTangents;
        private bool _reverseHandedness;
        private bool _importEmbededTextures;
        private bool _importAnimations;
        
        // PROPERTIES
        public float SmoothingAngle
        {
            get => _smoothingAngle;
            set
            {
                if (_smoothingAngle != value)
                {
                    _smoothingAngle = value;
                    OnPropertyChanged(nameof(SmoothingAngle));
                }
            }
        }
        public bool CalculateNormals
        {
            get => _calculateNormals;
            set
            {
                if (_calculateNormals != value)
                {
                    _calculateNormals = value;
                    OnPropertyChanged(nameof(CalculateNormals));
                }
            }
        }
        public bool CalculateTangents
        {
            get => _calculateTangents;
            set
            {
                if (_calculateTangents != value)
                {
                    _calculateTangents = value;
                    OnPropertyChanged(nameof(CalculateTangents));
                }
            }
        }
        public bool ReverseHandedness
        {
            get => _reverseHandedness;
            set
            {
                if (_reverseHandedness != value)
                {
                    _reverseHandedness = value;
                    OnPropertyChanged(nameof(ReverseHandedness));
                }
            }
        }
        public bool ImportEmbededTextures
        {
            get => _importEmbededTextures;
            set
            {
                if (_importEmbededTextures != value)
                {
                    _importEmbededTextures = value;
                    OnPropertyChanged(nameof(ImportEmbededTextures));
                }
            }
        }
        public bool ImportAnimations
        {
            get => _importAnimations;
            set
            {
                if (_importAnimations != value)
                {
                    _importAnimations = value;
                    OnPropertyChanged(nameof(ImportAnimations));
                }
            }
        }

        // PUBLIC
        public GeometryImportSettings()
        {
            SmoothingAngle = 178.0f;
            CalculateNormals = false;
            CalculateTangents = false;
            ReverseHandedness = false;
            ImportEmbededTextures = true;
            ImportAnimations = true;
        }

        public void ToBinary(BinaryWriter writer)
        {
            writer.Write(SmoothingAngle);
            writer.Write(CalculateNormals);
            writer.Write(CalculateTangents);
            writer.Write(ReverseHandedness);
            writer.Write(ImportEmbededTextures);
            writer.Write(ImportAnimations);
        }
    }
    
    class Geometry : Asset
    {
        // STATE
        private readonly List<LoDGroup> _lodGroups = new List<LoDGroup>();

        // PROPERTIES
        public GeometryImportSettings ImportSettings { get; } = new GeometryImportSettings();

        // PUBLIC
        public Geometry() : base(AssetType.Mesh)
        {
        }
        
        public void FromRawData(byte[] data)
        {
            Debug.Assert(data?.Length > 0);

            _lodGroups.Clear();

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
                _lodGroups.Add(lodGroup);
            }
        }

        public LoDGroup GetLoDGroup(int lodGroup = 0)
        {
            Debug.Assert(lodGroup >= 0 && lodGroup < _lodGroups.Count);

            return _lodGroups.Any() ? _lodGroups[lodGroup] : null;
        }

        public override IEnumerable<string> Save(string file)
        {
            Debug.Assert(_lodGroups.Any());
            
            List<string> savedFiles = new List<string>();
            
            if (!_lodGroups.Any()) return savedFiles;

            string path = Path.GetDirectoryName(file) + Path.DirectorySeparatorChar;
            string fileName = Path.GetFileNameWithoutExtension(file);
            
            try
            {
                foreach (var lodGroup in _lodGroups)
                {
                    Debug.Assert(lodGroup.LoDs.Any());
                    
                    // Use name of most detailed LoD for file name
                    string meshFileName = ContentHelper.SanitizeFileName(_lodGroups.Count > 1 ?
                        path + fileName + "_" + lodGroup.LoDs[0].Name + AssetFileExtension :
                        path + fileName + AssetFileExtension);
                    // Different id for each asset file
                    Guid = Guid.NewGuid();
                    byte[] data = null;
                    using(BinaryWriter writer = new BinaryWriter(new MemoryStream()))
                    {
                        writer.Write(lodGroup.Name);
                        writer.Write(lodGroup.LoDs.Count);
                        List<byte> hashes = new List<byte>();
                        
                        foreach (var lod in lodGroup.LoDs)
                        {
                            LoDToBinary(lod, writer, out byte[] hash);
                            hashes.AddRange(hash);
                        }

                        Hash = ContentHelper.ComputeHash(hashes.ToArray());
                        data = (writer.BaseStream as MemoryStream).ToArray();
                        Icon = GenerateIcon(lodGroup.LoDs[0]);
                    }

                    Debug.Assert(data?.Length > 0);

                    using (BinaryWriter writer = new BinaryWriter(File.Open(meshFileName, FileMode.Create, FileAccess.Write)))
                    {
                        WriteAssetFileHeader(writer);
                        ImportSettings.ToBinary(writer);
                        writer.Write(data.Length);
                        writer.Write(data);
                    };

                    savedFiles.Add(meshFileName);
                }
            }
            catch (Exception ex)
            {
                Debug.Write(ex.Message);
                Logger.Log(MessageType.Error, $"There was an error saving geometry to {file}");
            }

            return savedFiles;
        }

        // PRIVATE
        private byte[] GenerateIcon(MeshLoD lod)
        {
            int width = 90 * 4;
            BitmapSource bmp = null;

            // NOTE: it's not good practice to use a WPF control (view) in the ViewModel.
            //       But an exception must be made in this case until we have a graphics
            //       renderer to handle screenshots
            Application.Current.Dispatcher.Invoke(() => // Run on UI thread
            {
                bmp = Editors.GeometryView.RenderToBitmap(new Editors.MeshRenderer(lod, null), width, width);
                bmp = new TransformedBitmap(bmp, new ScaleTransform(0.25, 0.25, 0.5, 0.5));
            });

            using MemoryStream memStream = new MemoryStream();
            memStream.SetLength(0);

            PngBitmapEncoder encoder = new PngBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(bmp));
            encoder.Save(memStream);

            return memStream.ToArray();
        }

        private void LoDToBinary(MeshLoD lod, BinaryWriter writer, out byte[] hash)
        {
            writer.Write(lod.Name);
            writer.Write(lod.LoDThreshold);
            writer.Write(lod.Meshes.Count);

            long meshDataBegin = writer.BaseStream.Position;

            foreach (var mesh in lod.Meshes)
            {
                writer.Write(mesh.VertexSize);
                writer.Write(mesh.VertexCount);
                writer.Write(mesh.IndexSize);
                writer.Write(mesh.IndexCount);
                writer.Write(mesh.Vertices);
                writer.Write(mesh.Indices);
            }

            long meshDataSize = writer.BaseStream.Position - meshDataBegin;

            Debug.Assert(meshDataSize > 0);

            byte[] buffer = (writer.BaseStream as MemoryStream).ToArray();
            hash = ContentHelper.ComputeHash(buffer, (int)meshDataBegin, (int)meshDataSize);
        }

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
