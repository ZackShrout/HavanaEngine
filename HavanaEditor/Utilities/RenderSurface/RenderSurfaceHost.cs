using HavanaEditor.DllWrapper;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Input;
using System.Windows.Interop;

namespace HavanaEditor.Utilities
{
    class RenderSurfaceHost : HwndHost
    {
        // STATE
        private readonly int width = 800;
        private readonly int height = 600;
        private IntPtr renderWindowHandle = IntPtr.Zero;
        private DelayEventTimer resizeTimer;

        // PROPERTIES
        public int SurfaceId { get; private set; } = ID.INVALID_ID;

        // PUBLIC
        public RenderSurfaceHost(double width, double height)
        {
            this.width = (int)width;
            this.height = (int)height;
            resizeTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));
            resizeTimer.Triggered += Resize;
        }

        /// <summary>
        /// Resize the render surfaces in the host.
        /// </summary>
        public void Resize()
        {
            resizeTimer.Trigger();
        }

        // PROTECTED
        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            // Create in instance of the window to host
            SurfaceId = EngineAPI.CreateRenderSurface(hwndParent.Handle, width, height);
            Debug.Assert(ID.IsValid(SurfaceId));
            renderWindowHandle = EngineAPI.GetWindowHandle(SurfaceId);
            Debug.Assert(renderWindowHandle != IntPtr.Zero);
            
            return new HandleRef(this, renderWindowHandle);
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            // Destroy instance of window currently being hosted
            EngineAPI.RemoveRenderSurface(SurfaceId);
            SurfaceId = ID.INVALID_ID;
            renderWindowHandle = IntPtr.Zero;
        }

        // PRIVATE
        private void Resize(object sender, DelayEventTimerArgs e)
        {
            e.RepeatEvent = Mouse.LeftButton == MouseButtonState.Pressed;
            if (!e.RepeatEvent)
            {
                Logger.Log(MessageTypes.Info, "Resized");
            }
        }
    }
}
