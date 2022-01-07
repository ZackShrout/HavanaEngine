using HavanaEditor.Common;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace HavanaEditor.Utilities
{
    /// <summary>
    /// Interaction logic for RenderSurfaceView.xaml
    /// </summary>
    public partial class RenderSurfaceView : UserControl, IDisposable
    {
        // STATE
        private RenderSurfaceHost _host = null;

        private enum Win32Msg
        {
            WM_SIZE = 0x0005,
            WM_ENTERSIZEMOVE = 0x0231,
            WM_EXITSIZEMOVE = 0x0232,
            WM_SIZING =0x0214
        }

        // PUBLIC
        public RenderSurfaceView()
        {
            InitializeComponent();
            Loaded += OnRenderSurfaceViewLoaded;
        }

        // PRIVATE
        private void OnRenderSurfaceViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnRenderSurfaceViewLoaded;

            _host = new RenderSurfaceHost(ActualWidth, ActualHeight);
            _host.MessageHook += new HwndSourceHook(HostMsgFilter);
            Content = _host;
        }

        private IntPtr HostMsgFilter(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch ((Win32Msg)msg)
            {
                case Win32Msg.WM_SIZE:
                    break;
                case Win32Msg.WM_ENTERSIZEMOVE:
                    break;
                case Win32Msg.WM_EXITSIZEMOVE:
                    break;
                case Win32Msg.WM_SIZING:
                    break;
                default:
                    break;
            }

            return IntPtr.Zero;
        }

        #region IDisposable Support
        private bool disposedValue;
        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    _host.Dispose();
                }
                disposedValue = true;
            }
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}
