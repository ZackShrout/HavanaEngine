using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Components
{
    [DataContract]
    public class Component : ViewModelBase
    {
        // PROPERTIES
        [DataMember]
        public GameEntity Owner { get; set; }

        // PUBLIC
        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }
}
