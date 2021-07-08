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
    }


    [StructLayout(LayoutKind.Sequential)]
    class SceneData : IDisposable
    {
        public IntPtr Data;
        public int DataSize;
        public GeometryImportSettings Settings = new GeometryImportSettings();

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
        private const string toolsDLL = "ContentTools.dll";

        // PUBLIC
        /// <summary>
        /// Create a primitive mesh asset.
        /// </summary>
        /// <param name="geometry"> - The type of primitive mesh to create.</param>
        /// <param name="info"> - Initialization info for the primitive mesh.</param>
        public static void CreatePrimitiveMesh(Content.Geometry geometry, PrimitiveInitInfo info)
        {
            Debug.Assert(geometry != null);

            using SceneData sceneData = new SceneData();
            try
            {
                CreatePrimitiveMesh(sceneData, info);
                
                Debug.Assert(sceneData.Data != IntPtr.Zero && sceneData.DataSize > 0);

                byte[] data = new byte[sceneData.DataSize];
                Marshal.Copy(sceneData.Data, data, 0, sceneData.DataSize);
                geometry.FromRawData(data);
            }
            catch (Exception ex)
            {
                Logger.Log(MessageType.Error, $"Failed to create {info.Type} primitive mesh.");
                Debug.WriteLine(ex.Message);
            }
        }

        // PRIVATE
        [DllImport(toolsDLL)]
        private static extern void CreatePrimitiveMesh([In, Out] SceneData data, PrimitiveInitInfo info);
    }
}
