using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace Unlocker
{
    internal static class Program
    {
        private static FormMain _mainForm;
        private static HandleUnlocker _handleUnlocker = new HandleUnlocker(ProcessorType.x86, 50);

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            try
            {
                if (args.Length == 0) return;

                var path = args.First();
                _mainForm = new FormMain(path);
                _mainForm.Unlock += MainForm_Unlock;
                _mainForm.RefreshFilesInfo += MainForm_RefreshFilesInfo;
                _mainForm.Show();
                Application.DoEvents();

                _mainForm.QueryStateAndReload();

                while(_mainForm.Visible)
                {
                    Application.DoEvents();
                }
            }
            catch(Exception e)
            {
                MessageBox.Show(e.ToString());
            }
        }

        private static System.Collections.Generic.List<LockedFileInfo> MainForm_RefreshFilesInfo(string path)
        {
            return _handleUnlocker.QueryLockedFileInfo(path);
        }

        private static void MainForm_Unlock(LockedFileInfo lockedFileInfo)
        {
            _handleUnlocker.UnlockFile(lockedFileInfo);
        }
    }
}