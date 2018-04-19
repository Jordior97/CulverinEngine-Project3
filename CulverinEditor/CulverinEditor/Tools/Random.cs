﻿using System.Runtime.CompilerServices;

namespace CulverinEditor
{
    /// <summary>  
    /// Class for generating random data.
    /// </summary>  
    public sealed class Random
    {
        /// <summary>  
        ///     Returns a random integer number between and min [inclusive] and max [inclusive]
        ///     (Read Only).
        /// </summary>  
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Range(int min, int max);

        /// <summary>  
        ///     Returns a random float number between and min [inclusive] and max [exclusive]
        ///     (Read Only).
        /// </summary>  
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float Range(float min, float max);

        /// <summary>  
        ///     Returns a random integer number between and min [inclusive] and max [inclusive] and Specify number [exclusive] (this number must be between min-max).
        ///     (Read Only).
        /// </summary>  
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int Range(int min, int max, int norepeat);

    }
}
