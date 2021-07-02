using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace HavanaEditor.Components
{
    enum ComponentType
    {
        Transform,
        Script
    }

    class ComponentFactory
    {
        // STATE
        private static readonly Func<GameEntity, object, Component>[] function = new Func<GameEntity, object, Component>[]
        {
            (entity, data) => new Transform(entity),
            (entity, data) => new Script(entity) { Name = (string)data }
        };

        // PUBLIC
        public static Func<GameEntity, object, Component> GetCreationFunction(ComponentType componentType)
        {
            Debug.Assert((int)componentType < function.Length);
            return function[(int)componentType];
        }
    }
}
