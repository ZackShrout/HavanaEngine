using HavanaEditor.GameProject;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;

namespace HavanaEditor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        // PROPERTIES
        public static string HavanaPath { get; private set; } = @"H:\dev\HavanaEngine\Havana";

        // PUBLIC
        public MainWindow()
        {
            InitializeComponent();
            Loaded += OnMainWindowLoaded;
            Closing += OnMainWindowClosing;
        }

        // PRIVATE
        private void OnMainWindowClosing(object sender, CancelEventArgs e)
        {
            Closing -= OnMainWindowClosing;
            Project.Current?.Unload();
        }

        private void OnMainWindowLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnMainWindowLoaded;
            GetEnginePath();
            OpenProjectBrowserDialogue();
        }

        private void GetEnginePath()
        {
            string havanaPath = Environment.GetEnvironmentVariable("HAVANA_ENGINE", EnvironmentVariableTarget.User);
            if (havanaPath == null || !Directory.Exists(Path.Combine(havanaPath + @"Engine\EngineAPI")))
            {
                var dialog = new EnginePathDialog();
                if (dialog.ShowDialog() == true)
                {
                    HavanaPath = dialog.HavanaPath;
                    Environment.SetEnvironmentVariable("HAVANA_ENGINE", HavanaPath.ToUpper(), EnvironmentVariableTarget.User);
                }
                else
                {
                    Application.Current.Shutdown();
                }
            }
            else
            {
                HavanaPath = havanaPath;
            }
        }

        private void OpenProjectBrowserDialogue()
        {
            var projectBrowser = new ProjectBrowserDialogue();
            if (projectBrowser.ShowDialog() == false || projectBrowser.DataContext == null)
            {
                Application.Current.Shutdown();
            }
            else
            {
                Project.Current?.Unload();
                DataContext = projectBrowser.DataContext;
            }
        }
    }
}
