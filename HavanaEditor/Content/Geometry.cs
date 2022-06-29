using HavanaEditor.Common;
using HavanaEditor.DllWrapper;
using HavanaEditor.GameProject;
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

    enum ElementsType
    {
        Position = 0x00,
        Normals = 0x01,
        TSpace = 0x03,
        Joints = 0x04,
        Colors = 0x08
    }

    enum PrimitiveTopology
    {
        PointList = 1,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
    };

    class Mesh : ViewModelBase
    {
        public static int PositionSize = sizeof(float) * 3;
        
        // STATE
        private int _elementSize;
        private int _vertexCount;
        private int _indexSize;
        private int _indexCount;
        private string _name;

        // PROPERTIES
        public int ElementSize
        {
            get => _elementSize;
            set
            {
                if (_elementSize != value)
                {
                    _elementSize = value;
                    OnPropertyChanged(nameof(ElementSize));
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
        public ElementsType ElementsType { get; set; }
        public PrimitiveTopology PrimitiveTopology { get; set; }
        public byte[] Positions { get; set; }
        public byte[] Elements { get; set; }
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

        public void FromBinary(BinaryReader reader)
        {
            SmoothingAngle = reader.ReadSingle();
            CalculateNormals = reader.ReadBoolean();
            CalculateTangents = reader.ReadBoolean();
            ReverseHandedness = reader.ReadBoolean();
            ImportEmbededTextures = reader.ReadBoolean();
            ImportAnimations = reader.ReadBoolean();
        }
    }
    
    class Geometry : Asset
    {
        // STATE
        private readonly List<LoDGroup> _lodGroups = new List<LoDGroup>();
        private readonly object _lock = new object();

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

            return (lodGroup < _lodGroups.Count) ? _lodGroups[lodGroup] : null;
        }

        public override void Import(string file)
        {
            Debug.Assert(File.Exists(file));
            Debug.Assert(!string.IsNullOrEmpty(FullPath));
            var ext = Path.GetExtension(file).ToLower();

            SourcePath = file;

            try
            {
                if (ext == ".fbx")
                {
                    ImportFbx(file);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                var msg = $"Failed to read {file} for import.";
                Debug.WriteLine(msg);
                Logger.Log(MessageType.Error, msg);
            }
        }

        private void ImportFbx(string file)
        {
            Logger.Log(MessageType.Info, $"Importing FBX file {file}");
            var tempPath = Application.Current.Dispatcher.Invoke(() => Project.Current.TempFolder);
            if (string.IsNullOrEmpty(tempPath)) return;

            lock(_lock)
                if (!Directory.Exists(tempPath)) Directory.CreateDirectory(tempPath);

            var tempFile = $"{tempPath}{ContentHelper.GetRandomString()}.fbx";
            File.Copy(file, tempFile, true);
            ContentToolsAPI.ImportFbx(tempFile, this);
        }

        public override void Load(string file)
        {
            Debug.Assert(File.Exists(file));
            Debug.Assert(Path.GetExtension(file).ToLower() == AssetFileExtension);

            try
            {
                byte[] data = null;
                using(var reader = new BinaryReader(File.Open(file, FileMode.Open, FileAccess.Read)))
                {
                    ReadAssetFileHeader(reader);
                    ImportSettings.FromBinary(reader);
                    int dataLength = reader.ReadInt32();
                    Debug.Assert(dataLength > 0);
                    data = reader.ReadBytes(dataLength);
                }

                Debug.Assert(data.Length > 0);

                using (var reader = new BinaryReader(new MemoryStream(data)))
                {
                    LoDGroup lodGroup = new LoDGroup();
                    lodGroup.Name = reader.ReadString();
                    var lodCount = reader.ReadInt32();

                    for (int i = 0; i < lodCount; i++)
                    {
                        lodGroup.LoDs.Add(BinaryToLOD(reader));
                    }

                    _lodGroups.Clear();
                    _lodGroups.Add(lodGroup);
                }

                // For Testing. Remove Later!
                // PackForEngine();
                // For Testing. Remove Later!
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to load geometry asset form file: {file}");
            }
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
                    string meshFileName = ContentHelper.SanitizeFileName(
                           path + fileName + ((_lodGroups.Count > 1) ? "_" + ((lodGroup.LoDs.Count > 1) ? lodGroup.Name : lodGroup.LoDs[0].Name) : string.Empty)) + AssetFileExtension;
                    // Different id for each asset file, but if a geometry asset file with the same name
                    // already exists, use it's guid instead.
                    Guid = TryGetAssetInfo(meshFileName) is AssetInfo info && info.Type == Type ? info.Guid : Guid.NewGuid();
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

                    Logger.Log(MessageType.Info, $"Saved geometry to {meshFileName}");
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
            int width = ContentInfo.IconWidth * 4;

            using var memStream = new MemoryStream();
            BitmapSource bmp = null;
            // NOTE: it's not good practice to use a WPF control (view) in the ViewModel.
            //       But an exception must be made in this case until we have a graphics
            //       renderer to handle screenshots
            Application.Current.Dispatcher.Invoke(() => // Run on UI thread
            {
                bmp = Editors.GeometryView.RenderToBitmap(new Editors.MeshRenderer(lod, null), width, width);
                bmp = new TransformedBitmap(bmp, new ScaleTransform(0.25, 0.25, 0.5, 0.5));

                memStream.SetLength(0);

                PngBitmapEncoder encoder = new PngBitmapEncoder();
                encoder.Frames.Add(BitmapFrame.Create(bmp));
                encoder.Save(memStream);
            });

            return memStream.ToArray();
        }

        /// <summary>
        /// Packs the geometry into a byte array which can be used by the engine.
        /// </summary>
        /// <returns>
        /// A byte array that contains
        /// struct
        /// {
        ///     u32 lodCount
        ///     struct
        ///     {
        ///         f32 lodThreshold,
        ///         u32 submeshCount,
        ///         u32 sizeOfSubmeshes,
        ///         struct
        ///         {
        ///             u32 elementSize, u32 vertexCount,
        ///             u32 indexCount, u32 elementsType, u32 primitiveTopology
        ///		       u8 positions[sizeof(f32) * 3 * vertextCount],		// sizeof(positions) must be a multiple of 4 bytes. Pad if needed.
        ///		       u8 elements[sizeof(elementSize) * vertextCount],	// sizeof(elements) must be a multiple of 4 bytes. Pad if needed.
        ///		       u8 indices[indexSize * indexCount],
        ///         } submeshes[submeshCount]
        ///     } meshLods[lodCount]
        /// } geometry;
        /// </returns>
        public override byte[] PackForEngine()
        {
            using var writer = new BinaryWriter(new MemoryStream());

            writer.Write(GetLoDGroup().LoDs.Count);
            foreach (var lod in GetLoDGroup().LoDs)
            {
                writer.Write(lod.LoDThreshold);
                writer.Write(lod.Meshes.Count);
                var sizeOfSubmeshesPosition = writer.BaseStream.Position;
                writer.Write(0);
                foreach (var mesh in lod.Meshes)
                {
                    writer.Write(mesh.ElementSize);
                    writer.Write(mesh.VertexCount);
                    writer.Write(mesh.IndexCount);
                    writer.Write((int)mesh.ElementsType);
                    writer.Write((int)mesh.PrimitiveTopology);

                    var alignedPositionBuffer = new byte[MathU.AlignSizeUp(mesh.Positions.Length, 4)];
                    Array.Copy(mesh.Positions, alignedPositionBuffer, mesh.Positions.Length);
                    var alignedElementBuffer = new byte[MathU.AlignSizeUp(mesh.Elements.Length, 4)];
                    Array.Copy(mesh.Elements, alignedElementBuffer, mesh.Elements.Length);

                    writer.Write(alignedPositionBuffer);
                    writer.Write(alignedElementBuffer);
                    writer.Write(mesh.Indices);
                }

                var endOfSubmeshes = writer.BaseStream.Position;
                var sizeOfSubmeshes = (int)(endOfSubmeshes - sizeOfSubmeshesPosition - sizeof(int));

                writer.BaseStream.Position = sizeOfSubmeshesPosition;
                writer.Write(sizeOfSubmeshes);
                writer.BaseStream.Position = endOfSubmeshes;
            }

            writer.Flush();
            var data = (writer.BaseStream as MemoryStream)?.ToArray();
            Debug.Assert(data?.Length > 0);

            // For testing. Remove later!
            using (var fs = new FileStream(@"..\..\EngineTest\model.model", FileMode.Create))
            {
                fs.Write(data, 0, data.Length);
            }
            // For testing. Remove later!

            return data;
        }

        private void LoDToBinary(MeshLoD lod, BinaryWriter writer, out byte[] hash)
        {
            writer.Write(lod.Name);
            writer.Write(lod.LoDThreshold);
            writer.Write(lod.Meshes.Count);

            long meshDataBegin = writer.BaseStream.Position;

            foreach (var mesh in lod.Meshes)
            {
                writer.Write(mesh.Name);
                writer.Write(mesh.ElementSize);
                writer.Write((int)mesh.ElementsType);
                writer.Write((int)mesh.PrimitiveTopology);
                writer.Write(mesh.VertexCount);
                writer.Write(mesh.IndexSize);
                writer.Write(mesh.IndexCount);
                writer.Write(mesh.Positions);
                writer.Write(mesh.Elements);
                writer.Write(mesh.Indices);
            }

            long meshDataSize = writer.BaseStream.Position - meshDataBegin;

            Debug.Assert(meshDataSize > 0);

            byte[] buffer = (writer.BaseStream as MemoryStream).ToArray();
            hash = ContentHelper.ComputeHash(buffer, (int)meshDataBegin, (int)meshDataSize);
        }

        private MeshLoD BinaryToLOD(BinaryReader reader)
        {
            var lod = new MeshLoD();

            lod.Name = reader.ReadString();
            lod.LoDThreshold = reader.ReadSingle();
            var meshCount = reader.ReadInt32();

            for (int i = 0; i < meshCount; i++)
            {
                var mesh = new Mesh()
                {
                    Name = reader.ReadString(),
                    ElementSize = reader.ReadInt32(),
                    ElementsType = (ElementsType)reader.ReadInt32(),
                    PrimitiveTopology = (PrimitiveTopology)reader.ReadInt32(),
                    VertexCount = reader.ReadInt32(),
                    IndexSize = reader.ReadInt32(),
                    IndexCount = reader.ReadInt32()
                };

                mesh.Positions = reader.ReadBytes(Mesh.PositionSize * mesh.VertexCount);
                mesh.Elements = reader.ReadBytes(mesh.ElementSize * mesh.VertexCount);
                mesh.Indices = reader.ReadBytes(mesh.IndexSize * mesh.IndexCount);

                lod.Meshes.Add(mesh);
            }

            return lod;
        }

        private static List<MeshLoD> ReadMeshLoDs(int numMeshes, BinaryReader reader)
        {
            List<int> lodIDs = new List<int>();
            List<MeshLoD> lodList = new List<MeshLoD>();
            for (int i = 0; i < numMeshes; i++)
            {
                ReadMeshes(reader, lodIDs, lodList);
            }

            return lodList;
        }

        private static void ReadMeshes(BinaryReader reader, List<int> lodIDs, List<MeshLoD> lodList)
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

            Mesh mesh = new Mesh() { Name = meshName };
            int lodID = reader.ReadInt32();
            mesh.ElementSize = reader.ReadInt32();
            mesh.ElementsType = (ElementsType)reader.ReadInt32();
            mesh.PrimitiveTopology = PrimitiveTopology.TriangleList; // ContentTools currently only support triangle list meshes.
            mesh.VertexCount = reader.ReadInt32();
            mesh.IndexSize = reader.ReadInt32();
            mesh.IndexCount = reader.ReadInt32();
            float lodThreshold = reader.ReadSingle();

            int elementBufferSize = mesh.ElementSize * mesh.VertexCount;
            int indexBufferSize = mesh.IndexSize * mesh.IndexCount;

            mesh.Positions = reader.ReadBytes(Mesh.PositionSize * mesh.VertexCount);
            mesh.Elements = reader.ReadBytes(elementBufferSize);
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
