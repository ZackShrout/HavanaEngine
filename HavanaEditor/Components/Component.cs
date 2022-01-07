using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
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

        /// <summary>
        /// Get a MultiSelect variant of a given component
        /// </summary>
        /// <param name="msEntity">Entity to whom the component belongs.</param>
        /// <returns>MultiSelect component object.</returns>
        public abstract IMSComponent GetMSComponent(MSEntity msEntity);

        /// <summary>
        /// Writes the component to a binary file.
        /// </summary>
        /// <param name="bw">The binary writer to use.</param>
        public abstract void WriteToBinary(BinaryWriter bw);
    }

    abstract class MSComponent<T> : ViewModelBase, IMSComponent where T : Component
    {
        // STATE
        private bool _enableUpdates = true;
        
        // PROPERTIES
        public List<T> SelectedComponents { get; }

        // PUBLIC
        public MSComponent(MSEntity msEntity)
        {
            Debug.Assert(msEntity?.SelectedEntities?.Any() == true);
            SelectedComponents = msEntity.SelectedEntities.Select(entity => entity.GetComponent<T>()).ToList();
            PropertyChanged += (s, e) => { if (_enableUpdates) UpdateComponents(e.PropertyName); };
        }

        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSComponent();
            _enableUpdates = true;
        }

        // PROTECTED
        protected abstract bool UpdateComponents(string propertyName);

        protected abstract bool UpdateMSComponent();
    }
}
