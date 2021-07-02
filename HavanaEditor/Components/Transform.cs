using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.IO;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Components
{
    [DataContract]
    class Transform : Component
    {
        // STATE
        private Vector3 position;
        private Vector3 rotation;
        private Vector3 scale;

        // PROPERTIES
        [DataMember]
        public Vector3 Position
        {
            get => position;
            set
            {
                if (position != value)
                {
                    position = value;
                    OnPropertyChanged(nameof(Position));
                }
            }
        }
        [DataMember]
        public Vector3 Rotation
        {
            get => rotation;
            set
            {
                if (rotation != value)
                {
                    rotation = value;
                    OnPropertyChanged(nameof(Rotation));
                }
            }
        }
        [DataMember]
        public Vector3 Scale
        {
            get => scale;
            set
            {
                if (scale != value)
                {
                    scale = value;
                    OnPropertyChanged(nameof(Scale));
                }
            }
        }

        // PUBLIC
        public Transform(GameEntity owner) : base(owner) { }

        public override IMSComponent GetMSComponent(MSEntity msEntity) => new MSTransform(msEntity);

        public override void WriteToBinary(BinaryWriter bw)
        {
            bw.Write(position.X); bw.Write(position.Y); bw.Write(position.Z);
            bw.Write(rotation.X); bw.Write(rotation.Y); bw.Write(rotation.Z);
            bw.Write(scale.X); bw.Write(scale.Y); bw.Write(scale.Z);
        }
    }

    sealed class MSTransform : MSComponent<Transform>
    {
        // STATE
        private float? posX;
        private float? posY;
        private float? posZ;
        private float? rotX;
        private float? rotY;
        private float? rotZ;
        private float? scaleX;
        private float? scaleY;
        private float? scaleZ;

        // PROPERTIES
        public float? PosX
        {
            get => posX;
            set
            {
                if (!posX.IsTheSameAs(value))
                {
                    posX = value;
                    OnPropertyChanged(nameof(PosX));
                }
            }
        }
        public float? PosY
        {
            get => posY;
            set
            {
                if (!posY.IsTheSameAs(value))
                {
                    posY = value;
                    OnPropertyChanged(nameof(PosY));
                }
            }
        }
        public float? PosZ
        {
            get => posZ;
            set
            {
                if (!posZ.IsTheSameAs(value))
                {
                    posZ = value;
                    OnPropertyChanged(nameof(PosZ));
                }
            }
        }
        public float? RotX
        {
            get => rotX;
            set
            {
                if (!rotX.IsTheSameAs(value))
                {
                    rotX = value;
                    OnPropertyChanged(nameof(RotX));
                }
            }
        }
        public float? RotY
        {
            get => rotY;
            set
            {
                if (!rotY.IsTheSameAs(value))
                {
                    rotY = value;
                    OnPropertyChanged(nameof(RotY));
                }
            }
        }
        public float? RotZ
        {
            get => rotZ;
            set
            {
                if (!rotZ.IsTheSameAs(value))
                {
                    rotZ = value;
                    OnPropertyChanged(nameof(RotZ));
                }
            }
        }
        public float? ScaleX
        {
            get => scaleX;
            set
            {
                if (!scaleX.IsTheSameAs(value))
                {
                    scaleX = value;
                    OnPropertyChanged(nameof(ScaleX));
                }
            }
        }
        public float? ScaleY
        {
            get => scaleY;
            set
            {
                if (!scaleY.IsTheSameAs(value))
                {
                    scaleY = value;
                    OnPropertyChanged(nameof(ScaleY));
                }
            }
        }
        public float? ScaleZ
        {
            get => scaleZ;
            set
            {
                if (!scaleZ.IsTheSameAs(value))
                {
                    scaleZ = value;
                    OnPropertyChanged(nameof(ScaleZ));
                }
            }
        }

        // PUBLIC
        public MSTransform(MSEntity msEntity) : base(msEntity)
        {
            Refresh();
        }

        // PROTECTED
        protected override bool UpdateComponents(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(PosX):
                case nameof(PosY):
                case nameof(PosZ):
                    SelectedComponents.ForEach(c => c.Position = new Vector3(posX ?? c.Position.X, posY ?? c.Position.Y, posZ ?? c.Position.Z));
                    return true;

                case nameof(RotX):
                case nameof(RotY):
                case nameof(RotZ):
                    SelectedComponents.ForEach(c => c.Rotation = new Vector3(rotX ?? c.Rotation.X, rotY ?? c.Rotation.Y, rotZ ?? c.Rotation.Z));
                    return true;

                case nameof(ScaleX):
                case nameof(ScaleY):
                case nameof(ScaleZ):
                    SelectedComponents.ForEach(c => c.Scale = new Vector3(scaleX ?? c.Scale.X, scaleY ?? c.Scale.Y, scaleZ ?? c.Scale.Z));
                    return true;
            }
            return false;
        }

        protected override bool UpdateMSComponent()
        {
            PosX = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Position.X));
            PosY = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Position.Y));
            PosZ = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Position.Z));

            RotX = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Rotation.X));
            RotY = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Rotation.Y));
            RotZ = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Rotation.Z));

            ScaleX = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Scale.X));
            ScaleY = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Scale.Y));
            ScaleZ = MSEntity.GetMixedValue(SelectedComponents, new Func<Transform, float>(x => x.Scale.Z));

            return true;
        }
    }
}
