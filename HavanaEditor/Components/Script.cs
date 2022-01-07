using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Components
{
    [DataContract]
    class Script : Component
    {
        // STATE
        private string _name;
        
        // PROPERTIES
        [DataMember]
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
        
        // PUBLIC
        public Script(GameEntity owner) : base(owner) {}

        public override IMSComponent GetMSComponent(MSEntity msEntity) => new MSScript(msEntity);

        public override void WriteToBinary(BinaryWriter bw)
        {
            byte[] nameBytes = Encoding.UTF8.GetBytes(Name);
            bw.Write(nameBytes.Length);
            bw.Write(nameBytes);
        }
    }

    sealed class MSScript : MSComponent<Script>
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
                SelectedComponents.ForEach(c => c.Name = _name);
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
