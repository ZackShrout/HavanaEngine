using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Components
{
    [DataContract]
    class Script : Component
    {
        // STATE
        private string name;
        
        // PROPERTIES
        [DataMember]
        public string Name
        {
            get => name;
            set
            {
                if (name != value)
                {
                    name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        
        // PUBLIC
        public Script(GameEntity owner) : base(owner) {}

        public override IMSComponent GetMSComponent(MSEntity msEntity) => new MSScript(msEntity);
    }

    sealed class MSScript : MSComponent<Script>
    {
        // STATE
        private string name;

        

        // PROPERTIES
        public string Name
        {
            get => name;
            set
            {
                if (name != value)
                {
                    name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        // PUBLIC
        public MSScript(MSEntity msEntity) : base(msEntity)
        {
            Refresh();
        }

        // PROTECTED
        protected override bool UpdateComponents(string propertyName)
        {
            if (propertyName == nameof(Name))
            {
                SelectedComponents.ForEach(c => c.Name = name);
                return true;
            }
            return false;
        }

        protected override bool UpdateMSComponent()
        {
            Name = MSEntity.GetMixedValue(SelectedComponents, new Func<Script, string>(x => x.Name));
            return true;
        }
    }
}
