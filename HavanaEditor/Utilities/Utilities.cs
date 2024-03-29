﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Windows.Threading;

namespace HavanaEditor.Utilities
{
    public static class ID
    {
        public static int INVALID_ID => -1;
        public static bool IsValid(int id) => id != INVALID_ID;
    }
    
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

        /// <summary>
        /// Align by rounding up. Will result in a multiple of 'alignment'
        /// that is greater than or equal to 'size'
        /// </summary>
        public static long AlignSizeUp(long size, long alignment)
        {
            Debug.Assert(alignment > 0, "Alignment must be non-zero");
            long mask = alignment - 1;
            Debug.Assert((alignment & mask) == 0, "Alignment must be a power of 2.");
            return ((size + mask) & ~mask);
        }

        /// <summary>
        /// Align by rounding down. Will result in a multiple of 'alignment'
        /// that is less than or equal to 'size'
        /// </summary>
        /// <param name="size"></param>
        /// <returns></returns>
        public static long AlignSizeDown(long size, long alignment)
        {
            Debug.Assert(alignment > 0, "Alignment must be non-zero");
            long mask = alignment - 1;
            Debug.Assert((alignment & mask) == 0, "Alignment must be a power of 2.");
            return (size & ~mask);
        }
    }

    class DelayEventTimerArgs : EventArgs
    {
        // PROPERTIES
        public bool RepeatEvent { get; set; }
        public IEnumerable<object> Data { get; set; }

        // PUBLIC
        public DelayEventTimerArgs(IEnumerable<object> data)
        {
            Data = data;
        }
    }

    class DelayEventTimer
    {
        // STATE
        private readonly DispatcherTimer _timer;
        private readonly TimeSpan _delay;
        private readonly List<object> _data = new List<object>();
        private DateTime _lastEventTime = DateTime.Now;

        // EVENT HANDLERS
        public event EventHandler<DelayEventTimerArgs> Triggered;

        // PUBLIC
        public DelayEventTimer(TimeSpan delay, DispatcherPriority priority = DispatcherPriority.Normal)
        {
            _delay = delay;
            _timer = new DispatcherTimer(priority)
            {
                Interval = TimeSpan.FromMilliseconds(delay.TotalMilliseconds * 0.5)
            };
            _timer.Tick += OnTimerTick;
        }

        public void Trigger (object data = null)
        {
            if (data != null)
            {
                _data.Add(data);
            }

            _lastEventTime = DateTime.Now;
            _timer.IsEnabled = true;
        }

        public void Disable()
        {
            _timer.IsEnabled = false;
        }

        // PRIVATE
        private void OnTimerTick(object sender, EventArgs e)
        {
            if ((DateTime.Now - _lastEventTime) < _delay) return;

            var eventArgs = new DelayEventTimerArgs(_data);
            Triggered?.Invoke(this, eventArgs);
            if (!eventArgs.RepeatEvent)
            {
                _data.Clear();
            }
            _timer.IsEnabled = eventArgs.RepeatEvent;
        }
    }
}
