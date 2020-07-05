using System;
using System.Windows.Forms;

namespace Unlocker
{
    public class CursorWait : IDisposable
    {
        private readonly Control[] _controlsToLock;
        private readonly Control _controlCursor;

        public CursorWait(Control[] controlsToLock, Control controlCursor)
        {
            _controlCursor = controlCursor;
            _controlsToLock = controlsToLock;

            // Wait
            foreach (var controlToLock in _controlsToLock)
                controlToLock.Enabled = false;

            _controlCursor.Cursor = Cursors.WaitCursor;
        }

        public void Dispose()
        {
            // Reset
            foreach (var controlToLock in _controlsToLock)
                controlToLock.Enabled = true;
            _controlCursor.Cursor = Cursors.Default;
        }
    }
}
