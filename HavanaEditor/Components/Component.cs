using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Components
{
    interface IMSComponent
    { }

    [DataContract]
    abstract class Component : ViewModelBase
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

    abstract class MSComponent<T> : ViewModelBase, IMSComponent where T : Component
    {

    }
}
