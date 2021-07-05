using System;
using System.Collections.Generic;
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
    }

    class DelayEventTimerArgs : EventArgs
    {
        // PROPERTIES
        public bool RepeatEvent { get; set; }
        public object Data { get; set; }

        // PUBLIC
        public DelayEventTimerArgs(object data)
        {
            Data = data;
        }
    }

    class DelayEventTimer
    {
        // STATE
        private readonly DispatcherTimer timer;
        private readonly TimeSpan delay;
        private DateTime lastEventTime = DateTime.Now;
        private object data;

        // EVENT HANDLERS
        public event EventHandler<DelayEventTimerArgs> Triggered;

        // PUBLIC
        public DelayEventTimer(TimeSpan delay, DispatcherPriority priority = DispatcherPriority.Normal)
        {
            this.delay = delay;
            timer = new DispatcherTimer(priority)
            {
                Interval = TimeSpan.FromMilliseconds(delay.TotalMilliseconds * 0.5)
            };
            timer.Tick += OnTimerTick;
        }

        public void Trigger (object data = null)
        {
            this.data = data;
            lastEventTime = DateTime.Now;
            timer.IsEnabled = true;
        }

        public void Disable()
        {
            timer.IsEnabled = false;
        }

        // PRIVATE
        private void OnTimerTick(object sender, EventArgs e)
        {
            if ((DateTime.Now - lastEventTime) < delay) return;

            var eventArgs = new DelayEventTimerArgs(data);
            Triggered?.Invoke(this, eventArgs);
            timer.IsEnabled = eventArgs.RepeatEvent;
        }
    }
}
