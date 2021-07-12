using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;

namespace HavanaEditor.Content
{
    enum AssetType
    {
        Unknown,
        Animation,
        Audio,
        Material,
        Mesh,
        Skeleton,
        Texture
    }
    
    abstract class Asset : ViewModelBase
    {
        // PROPERTIES
        public static string AssetFileExtension => ".asset";
        public AssetType Type { get; private set; }//
        public byte[] Icon { get; protected set; }
        public string SourcePath { get; protected set; }
        public Guid Guid { get; protected set; } = Guid.NewGuid();//
        public DateTime ImportDate { get; protected set; }//
        public byte[] Hash { get; protected set; }//
        
        // PUBLIC
        public Asset(AssetType type)
        {
            Debug.Assert(type != AssetType.Unknown);
            Type = type;
        }

        public abstract IEnumerable<string> Save(string file);

        // PROTECTED
        protected void WriteAssetFileHeader(BinaryWriter writer)
        {
            byte[] id = Guid.ToByteArray();
            long importDate = DateTime.Now.ToBinary();

            // Explicitly set writer to start of file
            writer.BaseStream.Position = 0;

            writer.Write((int)Type);
            writer.Write(id.Length);
            writer.Write(id);
            writer.Write(importDate);
            
            // Asset hash is optional
            if (Hash?.Length > 0)
            {
                writer.Write(Hash.Length);
                writer.Write(Hash);
            }
            else
            {
                writer.Write(0);
            }

            writer.Write(SourcePath ?? "");
            writer.Write(Icon);
        }
    }
}
