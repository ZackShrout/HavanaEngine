using HavanaEditor.Components;
using HavanaEditor.EngineAPIStructs;
using System;
using System.Collections.Generic;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;

namespace HavanaEditor.EngineAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class TransformComponent
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale = new Vector3(1, 1, 1);
    }

    [StructLayout(LayoutKind.Sequential)]
    class GameEntityDescriptor
    {
        public TransformComponent transform = new TransformComponent();
    }
}

namespace HavanaEditor.DllWrapper
{
    static class EngineAPI
    {
        // CONST
        private const string dllName = "EngineDLL.dll";

        // PUBLIC
        /// <summary>
        /// Create a game entity in the engine.
        /// </summary>
        /// <param name="entity">- Entity to create.</param>
        /// <returns>ID number.</returns>
        public static int CreateGameEntity(GameEntity entity)
        {
            GameEntityDescriptor descriptor = new GameEntityDescriptor();

            // Transform Component
            {
                Transform component = entity.GetComponent<Transform>();
                descriptor.transform.Position = component.Position;
                descriptor.transform.Rotation = component.Rotation;
                descriptor.transform.Scale = component.Scale;
            }

            return CreateGameEntity(descriptor);
        }

        /// <summary>
        /// Remove a game entity in the engine.
        /// </summary>
        /// <param name="entity">- Entity to remove.</param>
        public static void RemoveGameEntity(GameEntity entity)
        {
            RemoveGameEntity(entity.EntityID);
        }

        // PRIVATE
        [DllImport(dllName)]
        private static extern int CreateGameEntity(GameEntityDescriptor descriptor);

        [DllImport(dllName)]
        private static extern void RemoveGameEntity(int id);
    }
}
