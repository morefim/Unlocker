using System;

namespace Unlocker
{
    public class LockedFileInfo
    {
        public string Process { get; internal set; }
        public int PID { get; internal set; }
        public string Type { get; internal set; }
        public string Handle { get; internal set; }
        public string File { get; internal set; }
        public uint HandleHex => Convert.ToUInt32("0x" + Handle, 16);

        public string Status { get; internal set; }
    }
}