using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace Unlocker
{
    public partial class FormMain : Form
    {
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
    }
}
