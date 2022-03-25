using HavanaEditor.ContentToolsAPIStructs;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;

namespace HavanaEditor.ContentToolsAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class GeometryImportSettings
    {
        public float SmoothingAngle = 178.0f;
		public byte CalculateNormals = 0;
		public byte CalculateTangents = 1;
		public byte ReverseHandedness = 0;
		public byte ImportEmbededTextures = 1;
		public byte ImportAnimations = 1;

        // PUBLIC
        public void FromContentSettings(Content.Geometry geometry)
        {
            var settings = geometry.ImportSettings;
            SmoothingAngle = settings.SmoothingAngle;
            CalculateNormals = ToByte(settings.CalculateNormals);
            CalculateTangents = ToByte(settings.CalculateTangents);
            ReverseHandedness = ToByte(settings.ReverseHandedness);
            ImportEmbededTextures = ToByte(settings.ImportEmbededTextures);
            ImportAnimations = ToByte(settings.ImportAnimations);
        }

        // PRIVATE
        private byte ToByte(bool value) => (byte)(value ? 1 : 0);
    }


    [StructLayout(LayoutKind.Sequential)]
    class SceneData : IDisposable
    {
        public IntPtr Data;
        public int DataSize;
        public GeometryImportSettings ImportSettings = new GeometryImportSettings();

        ~SceneData()
        {
            Dispose();
        }

        public void Dispose()
        {
            Marshal.FreeCoTaskMem(Data);
            GC.SuppressFinalize(this);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    class PrimitiveInitInfo
    {
        public Content.PrimitiveMeshType Type;
        public int SegmentX = 1;
        public int SegmentY = 1;
        public int SegmentZ = 1;
        public Vector3 Size = new Vector3(1.0f);
        public int LoD = 0;
    }
}

namespace HavanaEditor.DllWrapper
{
    static class ContentToolsAPI
    {
        // STATE
        private const string _toolsDLL = "ContentTools.dll";

        // PUBLIC
        /// <summary>
        /// Create a primitive mesh asset.
        /// </summary>
        /// <param name="geometry"> - The type of primitive mesh to create.</param>
        /// <param name="info"> - Initialization info for the primitive mesh.</param>
        public static void CreatePrimitiveMesh(Content.Geometry geometry, PrimitiveInitInfo info)
        {
            GeometryFromSceneData(geometry, (sceneData) => CreatePrimitiveMesh(sceneData, info), $"Failed to create {info.Type} primitive mesh.");
        }

        public static void ImportFbx(string file, Content.Geometry geometry)
        {
            GeometryFromSceneData(geometry, (sceneData) => ImportFbx(file, sceneData), $"Failed to import from FBX file: {file}");
        }

        // PRIVATE
        private static void GeometryFromSceneData(Content.Geometry geometry, Action<SceneData> sceneDataGenerator, string failureMessage)
        {
            Debug.Assert(geometry != null);

            using SceneData sceneData = new SceneData();
            try
            {
                sceneData.ImportSettings.FromContentSettings(geometry);

                sceneDataGenerator(sceneData);

                Debug.Assert(sceneData.Data != IntPtr.Zero && sceneData.DataSize > 0);

                byte[] data = new byte[sceneData.DataSize];
                Marshal.Copy(sceneData.Data, data, 0, sceneData.DataSize);
                geometry.FromRawData(data);
            }
            catch (Exception ex)
            {
                Logger.Log(MessageType.Error, failureMessage);
                Debug.WriteLine(ex.Message);
            }
        }

        [DllImport(_toolsDLL)]
        private static extern void CreatePrimitiveMesh([In, Out] SceneData data, PrimitiveInitInfo info);

        [DllImport(_toolsDLL)]
        private static extern void ImportFbx(string file, [In, Out] SceneData data);
    }
}
