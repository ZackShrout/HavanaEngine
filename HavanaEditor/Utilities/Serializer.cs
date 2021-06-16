using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.Serialization;
using System.Text;

namespace HavanaEditor.Utilities
{
    /// <summary>
    /// Serializes and deserializes specified data type to and from an .xml file.
    /// </summary>
    public static class Serializer
    {
        /// <summary>
        /// Serializes data of specified type to an .xml file.
        /// </summary>
        /// <typeparam name="T">Type of data to be serialized.</typeparam>
        /// <param name="instance">Instance of the data to be serialized.</param>
        /// <param name="path">Path to which to serialize the data.</param>
        public static void ToFile<T>(T instance, string path)
        {
            try
            {
                using var fileStream = new FileStream(path, FileMode.Create);
                var serializer = new DataContractSerializer(typeof(T));
                serializer.WriteObject(fileStream, instance);
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                // TODO: log error
            }
        }

        /// <summary>
        /// Deserializes an .xml file to specified data type.
        /// </summary>
        /// <typeparam name="T">Type of data to deserialize to.</typeparam>
        /// <param name="path">Path to deserialize from.</param>
        /// <returns>Data of specified type.</returns>
        public static T FromFile<T>(string path)
        {
            try
            {
                using var fileStream = new FileStream(path, FileMode.Open);
                var serializer = new DataContractSerializer(typeof(T));
                T instance = (T)serializer.ReadObject(fileStream);
                return instance;
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                // TODO: log error
                return default(T);
            }
        }
    }
}
