using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;

namespace HavanaEditor.GameDev
{
    static class VisualStudio
    {
        // STATE
        private static EnvDTE80.DTE2 vsInstance = null;
        private static readonly string progID = "VisualStudio.DTE.16.0";

        // PUBLIC
        /// <summary>
        /// Creates an instance of Visual Studio 2019, and opens solution
        /// at specified path.
        /// </summary>
        /// <param name="solutionPath">Path of the Visual Studio solution to open.</param>
        public static void OpenVisualStudio(string solutionPath)
        {
            IRunningObjectTable rot = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindCtx = null;

            try
            {
                if (vsInstance == null)
                {
                    // Find an open instance of Visual Studio and check if it has the
                    // game solution already opened - if so, attach instance to it
                    int hResult = GetRunningObjectTable(0, out rot);
                    if (hResult < 0 || rot == null) throw new COMException($"GetRunningObjectTable() returned HRESULT: {hResult:x8}");

                    rot.EnumRunning(out monikerTable);
                    monikerTable.Reset();

                    hResult = CreateBindCtx(0, out bindCtx);
                    if (hResult < 0 || bindCtx == null) throw new COMException($"CreateBindCtx() returned HRESULT: {hResult:x8}");

                    IMoniker[] currentMoniker = new IMoniker[1];
                    while (monikerTable.Next(1, currentMoniker, IntPtr.Zero) == 0)
                    {
                        string name = string.Empty;
                        currentMoniker[0]?.GetDisplayName(bindCtx, null, out name);
                        if (name.Contains(progID))
                        {
                            hResult = rot.GetObject(currentMoniker[0], out object obj);
                            if (hResult < 0 || obj == null) throw new COMException($"GetObject() returned HRESULT: {hResult:x8}");

                            EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;
                            string solutionName = dte.Solution.FullName;
                            if (solutionName == solutionPath)
                            {
                                vsInstance = dte;
                                break;
                            }
                        }
                    }


                    // If we still can't find it, create a new instance of it
                    if (vsInstance == null)
                    {
                        Type visualStudioType = Type.GetTypeFromProgID(progID, true);
                        vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                    }
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                Logger.Log(MessageTypes.Error, "Failed to open Visual Studio.");
            }
            finally
            {
                if (rot != null) Marshal.ReleaseComObject(rot);
                if (monikerTable != null) Marshal.ReleaseComObject(monikerTable);
                if (bindCtx != null) Marshal.ReleaseComObject(bindCtx);
            }
        }

        /// <summary>
        /// Close the instance of Visual Studio that has the game solution opened,
        /// saving all files in the solution, as well as the solution itself, first.
        /// </summary>
        public static void CloseVisualStudio()
        {
            if (vsInstance?.Solution.IsOpen == true)
            {
                vsInstance.ExecuteCommand("File.SaveAll");
                vsInstance.Solution.Close(true);
            }
            vsInstance?.Quit();
        }

        public static bool AddFilesToSolution(string solution, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio(solution);

            try
            {
                if (vsInstance != null)
                {
                    if (!vsInstance.Solution.IsOpen) vsInstance.Solution.Open(solution);
                    else vsInstance.ExecuteCommand("File.SaveAll");

                    foreach (EnvDTE.Project project in vsInstance.Solution.Projects)
                    {
                        if (project.UniqueName.Contains(projectName))
                        {
                            foreach (string file in files)
                            {
                                project.ProjectItems.AddFromFile(file);
                            }
                        }
                    }

                    var cpp = files.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");
                    if (!string.IsNullOrEmpty(cpp))
                    {
                        vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                    }

                    vsInstance.MainWindow.Activate();
                    vsInstance.MainWindow.Visible = true;
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                Logger.Log(MessageTypes.Error, "Could not add files to solution.");
                return false;
            }
            
            return true;
        }

        // PRIVATE
        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);
    }
}
