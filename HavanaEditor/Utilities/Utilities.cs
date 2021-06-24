using System;
using System.Collections.Generic;
using System.Text;

namespace HavanaEditor.Utilities
{
    public static class MathU
    {
        // PROPERTIES
        public static float Epsilon => 0.00001f;

        // PUBLIC
        /// <summary>
        /// Checks to see if two floats are the same, given a specified
        /// epsilon value of 0.00001.
        /// </summary>
        /// <param name="value">First float to check against.</param>
        /// <param name="other">Second float to check against.</param>
        /// <returns>True or false.</returns>
        public static bool IsTheSameAs(this float value, float other)
        {
            return Math.Abs(value - other) < Epsilon;
        }

        /// <summary>
        /// Checks to see if two nullable floats are the same, given a specified
        /// epsilon value of 0.00001.
        /// </summary>
        /// <param name="value">First nullable float to check against.</param>
        /// <param name="other">Second nullable float to check against.</param>
        /// <returns>True or false.</returns>
        public static bool IsTheSameAs(this float? value, float? other)
        {
            if (!value.HasValue || !other.HasValue) return false;
            return Math.Abs(value.Value - other.Value) < Epsilon;
        }
    }
}
