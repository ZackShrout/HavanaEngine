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
    class ScriptComponent
    {
        public IntPtr ScriptCreator;
    }

    [StructLayout(LayoutKind.Sequential)]
    class GameEntityDescriptor
    {
        public TransformComponent transform = new TransformComponent();
        public ScriptComponent script = new ScriptComponent();
    }
}

namespace HavanaEditor.DllWrapper
{
    static class EngineAPI
    {
        // CONST
        private const string engineDll = "EngineDLL.dll";

        /// <summary>
        /// Class to handle the coordination of game entity creation and destruction
        /// between the editor and the engine.
        /// </summary>
        internal static class EntityAPI
        {
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

                // Script Component
                {
                    //Script component = entity.GetComponent<Script>();
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
            [DllImport(engineDll)]
            private static extern int CreateGameEntity(GameEntityDescriptor descriptor);

            [DllImport(engineDll)]
            private static extern void RemoveGameEntity(int id);
        }

        // PUBLIC
        /// <summary>
        /// Load the game code DLL into the Havana Editor.
        /// </summary>
        /// <param name="dllPath">Path to the DLL file.</param>
        /// <returns>1 if successful, 0 if unsuccessful</returns>
        [DllImport(engineDll, CharSet = CharSet.Ansi)]
        public static extern int LoadGameCodeDll(string dllPath);

        /// <summary>
        /// Unload the game code DLL from the Havana Editor.
        /// </summary>
        /// <returns>1 if successful, 0 if unsuccessful</returns>
        [DllImport(engineDll)]
        public static extern int UnloadGameCodeDll();

        /// <summary>
        /// Gets an array of script names.
        /// </summary>
        /// <returns>Array of script names.</returns>
        [DllImport(engineDll)]
        [return: MarshalAs(UnmanagedType.SafeArray)]
        public static extern string[] GetScriptNames();

        /// <summary>
        /// Return a pointer to the script creator function.
        /// </summary>
        /// <param name="name">Name of script.</param>
        /// <returns>Pointer to script creator.</returns>
        [DllImport(engineDll)]
        public static extern IntPtr GetScriptCreator(string name);
    }
}
