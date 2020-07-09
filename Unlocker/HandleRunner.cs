using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Unlocker
{
    public enum ProcessorType
    {
        x86,
        x64,
    }

    /// <summary>
    /// Nthandle v4.22 - Handle viewer
    ///  Copyright(C) 1997-2019 Mark Russinovich
    ///  Sysinternals - www.sysinternals.com
    ///  
    ///  usage: handle[[-a[-l]][-u] | [-c<handle> [-y]] | [-s]] [-p<process>|<pid>] [name] [-nobanner]
    ///    -a Dump all handle information.
    ///    -l Just show pagefile-backed section handles.
    ///    -c Closes the specified handle(interpreted as a hexadecimal number).
    ///               You must specify the process by its PID.
    ///               WARNING: Closing handles can cause application or system instability.
    ///    -y Don't prompt for close handle confirmation.
    ///    -s Print count of each type of handle open.
    ///    -u Show the owning user name when searching for handles.
    ///    -p Dump handles belonging to process (partial name accepted).
    ///    name Search for handles to objects with<name>(fragment accepted).
    ///    -nobanner Do not display the startup banner and copyright message.
    ///
    /// No arguments will dump all file references.
    /// </summary>
    class HandleUnlocker
    {
        private readonly ProcessorType _processorType;
        private readonly int _timeout;

        public HandleUnlocker(ProcessorType processorType, int timeout)
        {
            _timeout = timeout;
            _processorType = processorType;
        }

        public void Run(List<string> files)        
        {
            foreach (var file in files)
            {
                var lockedFileInfo = QueryLockedFileInfo(file);
                if (lockedFileInfo.First().PID != 0)
                    UnlockFile(lockedFileInfo.First());
                else
                    MessageBox.Show(lockedFileInfo.First().Status);
            }
        }

        string HandleExePath => Path.GetFullPath((_processorType == ProcessorType.x64 ? "Handle64.exe" : "Handle.exe"));

        int Timeout => (int)TimeSpan.FromSeconds(_timeout).TotalMilliseconds;

        public List<LockedFileInfo> QueryLockedFileInfo(string fileOrFolder)
        {
            var retVal = new List<LockedFileInfo>();
            fileOrFolder = fileOrFolder.TrimEnd('\\');
            Debug.WriteLine("Querying handle...");
            var outputString = RunProcessAndReadOutput(HandleExePath, $"\"{fileOrFolder}\" -nobanner", Timeout);
            Debug.WriteLine(outputString);
            // WINWORD.EXE pid: 18600  type: File C3C: C:\Unlocker\UnlockerTest\test.docx
            var outputLines = outputString.Split(new[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
            if (outputLines.Length < 4) return retVal;

            foreach (var outputLine in outputLines.Skip(3))
            {
                var outputParts = outputLine.Split(new[] { "pid:", "type:", "  ", ": " }, StringSplitOptions.RemoveEmptyEntries)
                    .Select(o => o.Trim()).Where(o => o.Length > 0).ToArray();
                if (outputParts.Length < 5)
                {
                    retVal.Add(new LockedFileInfo { Status = outputLine });
                    continue;
                }
                var lockedFileInfo = new LockedFileInfo
                {
                    Process = outputParts[0],
                    PID = int.Parse(outputParts[1]),
                    Type = outputParts[2],
                    Handle = outputParts[3],
                    File = outputParts[4],
                };
                retVal.Add(lockedFileInfo);
            }
            return retVal;
        }

        public void UnlockFile(LockedFileInfo lockedFileInfo)
        {
            Debug.WriteLine("Unlocking handle...");
            // -c C3C -p 18600 -y
            var args = $"-c {lockedFileInfo.Handle} -p {lockedFileInfo.PID} -y -nobanner";
            var outputString = RunProcessAndReadOutput(HandleExePath, args, Timeout);
            Debug.WriteLine(outputString);
        }

        string RunProcessAndReadOutput(string processPath, string args, int processTimeout)
        {
            var outputStringBuilder = new StringBuilder();
            var process = new Process();
            try
            {
                process.StartInfo.FileName = Path.GetFileName(processPath);
                process.StartInfo.WorkingDirectory = Path.GetDirectoryName(processPath);
                process.StartInfo.Arguments = args;
                process.StartInfo.RedirectStandardError = true;
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.UseShellExecute = false;
                process.EnableRaisingEvents = false;
                process.OutputDataReceived += (sender, eventArgs) => outputStringBuilder.AppendLine(eventArgs.Data);
                process.ErrorDataReceived += (sender, eventArgs) => outputStringBuilder.AppendLine(eventArgs.Data);

                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                var processExited = process.WaitForExit(processTimeout);

                if (processExited == false) // we timed out...
                {
                    process.Kill();
                    throw new Exception("ERROR: Process took too long to finish");
                }
                else if (process.ExitCode != 0)
                {
                    var output = outputStringBuilder.ToString();
                    Debug.WriteLine("Process exited with non-zero exit code of: " + process.ExitCode + Environment.NewLine +
                    "Output from process: " + output);
                    return output;
                }
                return outputStringBuilder.ToString();
            }
            finally
            {
                process.Close();
            }
        }
    }
}
