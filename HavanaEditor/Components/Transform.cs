using System;
using System.Collections.Generic;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Components
{
    [DataContract]
    public class Transform : Component
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
        public Transform(GameEntity owner) : base(owner)
        {
        }
    }
}
