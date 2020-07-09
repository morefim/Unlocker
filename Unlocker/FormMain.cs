using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Unlocker
{
    public partial class FormMain : Form
    {

        // P/Invoke constants
        private const int WM_SYSCOMMAND = 0x112;
        private const int MF_STRING = 0x0;
        private const int MF_SEPARATOR = 0x800;

        // P/Invoke declarations
        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr GetSystemMenu(IntPtr hWnd, bool bRevert);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool AppendMenu(IntPtr hMenu, int uFlags, int uIDNewItem, string lpNewItem);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool InsertMenu(IntPtr hMenu, int uPosition, int uFlags, int uIDNewItem, string lpNewItem);

        // ID for the About item on the system menu
        private int SYSMENU_ABOUT_ID = 0x1;

        private readonly string _rootPath;
        private List<LockedFileInfo> _files;

        public delegate void UnlockEventHandler(LockedFileInfo lockedFileInfo);
        public event UnlockEventHandler Unlock;

        public delegate List<LockedFileInfo> RefreshEventHandler(string path);
        public event RefreshEventHandler RefreshFilesInfo;

        public FormMain(string rootPath)
        {
            InitializeComponent();

            _rootPath = rootPath;
            Text = "Unlocker - " + _rootPath;
            filesList.Columns.Add("Locked Files", 200);
            filesList.Columns.Add("Locked By", -2);
            filesList.AutoResizeColumns(ColumnHeaderAutoResizeStyle.HeaderSize);
        }

        private void ReloadList()
        {
            filesList.Items.Clear();
            foreach (var file in _files.Where(o => o.File?.Length > 0))
            {
                FileAttributes attr = File.GetAttributes(file.File);
                if (attr.HasFlag(FileAttributes.Directory)) continue;
                filesList.Items.Add(new ListViewItem(new string[] { file.File, file.Process }) { Checked = true, Tag = file });
            }
            filesList.AutoResizeColumns(filesList.Items.Count == 0 ? ColumnHeaderAutoResizeStyle.HeaderSize : ColumnHeaderAutoResizeStyle.ColumnContent);
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        private Control[] ControlsToLock => new Control[] { buttonKill, buttonRefresh, buttonUnlock };

        private void FormMain_HelpButtonClicked(object sender, CancelEventArgs e)
        {

        }

        private void buttonUnlock_Click(object sender, EventArgs e)
        {
            using (new CursorWait(ControlsToLock, this))
            {
                foreach (ListViewItem filesListItem in filesList.CheckedItems)
                {
                    try
                    {
                        Unlock((LockedFileInfo)filesListItem.Tag);
                        filesList.Items.Remove(filesListItem);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.ToString());
                    }
                }
            }
        }

        private void buttonKill_Click(object sender, EventArgs e)
        {
            Process[] processes = Process.GetProcesses();
            using (new CursorWait(ControlsToLock, this))
            {
                var itemsWithProcesses2Kill = filesList.CheckedItems.Cast<ListViewItem>().ToList();
                foreach (var pid2Kill in itemsWithProcesses2Kill.Select(o => (LockedFileInfo)o.Tag).Select(o => o.PID).Distinct())
                {
                    try
                    {
                        var process2Kill = processes.SingleOrDefault(o => o.Id == pid2Kill);
                        process2Kill?.Kill();
                        foreach(var itemWithProcesses2Kill in itemsWithProcesses2Kill)
                            filesList.Items.Remove(itemWithProcesses2Kill);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.ToString());
                    }
                }
            }
        }

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            QueryStateAndReload();
        }

        public void QueryStateAndReload()
        {
            using (new CursorWait(ControlsToLock, this))
            {
                filesList.Items.Clear();
                var filesInfoList = RefreshFilesInfo(_rootPath);
                _files = filesInfoList;
                ReloadList();
            }
        }

        protected override void OnHandleCreated(EventArgs e)
        {
            base.OnHandleCreated(e);

            // Get a handle to a copy of this form's system (window) menu
            IntPtr hSysMenu = GetSystemMenu(this.Handle, false);

            // Add a separator
            AppendMenu(hSysMenu, MF_SEPARATOR, 0, string.Empty);

            // Add the About menu item
            AppendMenu(hSysMenu, MF_STRING, SYSMENU_ABOUT_ID, "&About…");
        }

        protected override void WndProc(ref Message m)
        {
            base.WndProc(ref m);

            // Test if the About item was selected from the system menu
            if ((m.Msg == WM_SYSCOMMAND) && ((int)m.WParam == SYSMENU_ABOUT_ID))
            {
                var aboutBox = new AboutBox();
                aboutBox.ShowDialog();
            }
        }
    }
}
