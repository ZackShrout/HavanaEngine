using HavanaEditor.GameProject;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
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
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;


namespace HavanaEditor.GameDev
{
    /// <summary>
    /// Interaction logic for NewScriptDialog.xaml
    /// </summary>
    public partial class NewScriptDialog : Window
    {
        // STATE
        private static readonly string _namespace = GetNamespaceFromProjectName();
        private static readonly string cppCode = @"#include ""{0}.h""

namespace {1}
{{
	REGISTER_SCRIPT({0});
    void {0}::BeginPlay()
    {{
    
    }}

    void {0}::Update(float dt)
    {{
    
    }}
}} // namespace {1}";

        private static readonly string hCode = @"#pragma once

namespace {1}
{{
	class {0} : public Havana::Script::EntityScript
	{{
	public:
		constexpr explicit {0}(Havana::Entity::Entity entity) : Havana::Script::EntityScript{{entity}} {{}}
        void BeginPlay() override;
		void Update(float dt) override;
    private:
	}};
}} // namespace {1}" ;
        
        // PUBLIC
        public NewScriptDialog()
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;
            scriptPath.Text = @"Scripts\";
        }

        // PRIVATE
        private bool Validate()
        {
            bool isValid = false;
            string name = scriptName.Text.Trim();
            string path = scriptPath.Text.Trim();
            string errorMessage = string.Empty;
            
            if (string.IsNullOrEmpty(name))
            {
                errorMessage = "Please enter a script name.";
            }
            else if (name.IndexOfAny(Path.GetInvalidFileNameChars()) != -1 || name.Any(x => char.IsWhiteSpace(x)))
            {
                errorMessage = "One or more invalid characters or white spaces in script name.";
            }
            else if (string.IsNullOrEmpty(path))
            {
                errorMessage = "Please enter a script path.";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                errorMessage = "One or more invalid characters in script path.";
            }
            else if (!Path.GetFullPath(Path.Combine(Project.Current.Path, path)).Contains(Path.Combine(Project.Current.Path, @"Scripts\")))
            {
                errorMessage = "Scripts must be added to the Scripts folder, or a sub folder in the Scripts folder.";
            }
            else if (File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, path), $"{name}.cpp"))) || 
                File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, path), $"{name}.h"))))
            {
                errorMessage = $"Script {name} already exists in this folder.";
            }
            else
            {
                isValid = true;
            }

            if (!isValid)
            {
                messageTextBlock.Foreground = FindResource("Editor.RedBrush") as Brush;
            }
            else
            {
                messageTextBlock.Foreground = FindResource("Editor.FontBrush") as Brush;
            }
            
            messageTextBlock.Text = errorMessage;
            return isValid;
        }

        private void OnScriptName_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!Validate()) return;

            string name = scriptName.Text.Trim();
            messageTextBlock.Text = $"{name}.h and {name}.cpp will be added to {Project.Current.Name}";
        }

        private void OnScriptPath_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            Validate();
        }

        private async void OnCreate_Button_Click(object sender, RoutedEventArgs e)
        {
            if (!Validate()) return;
            IsEnabled = false;
            
            // Fade the busy animation in
            busyAnimation.Opacity = 0;
            busyAnimation.Visibility = Visibility.Visible;
            DoubleAnimation fadeIn = new DoubleAnimation(0, 1, new Duration(TimeSpan.FromMilliseconds(500)));
            busyAnimation.BeginAnimation(OpacityProperty, fadeIn);

            try
            {
                string name = scriptName.Text.Trim();
                string path = Path.GetFullPath(Path.Combine(Project.Current.Path, scriptPath.Text.Trim()));
                string solution = Project.Current.Solution;
                string projectName = Project.Current.Name;

                await Task.Run(() => CreateScript(name, path, solution, projectName));
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to create sript {scriptName.Text}");
            }
            finally
            {
                // Fade the busy animation out
                DoubleAnimation fadeOut = new DoubleAnimation(1, 0, new Duration(TimeSpan.FromMilliseconds(200)));
                fadeOut.Completed += (s, e) => 
                {
                    busyAnimation.Opacity = 0;
                    busyAnimation.Visibility = Visibility.Hidden;
                    Close();
                };
                busyAnimation.BeginAnimation(OpacityProperty, fadeOut);
            }
        }

        private void CreateScript(string name, string path, string solution, string projectName)
        { 
            if (!Directory.Exists(path)) Directory.CreateDirectory(path);

            string cpp = Path.GetFullPath(Path.Combine(path, $"{name}.cpp"));
            string h = Path.GetFullPath(Path.Combine(path, $"{name}.h"));

            using (StreamWriter streamWriter = File.CreateText(cpp))
            {
                streamWriter.Write(string.Format(cppCode, name, _namespace));
            }

            using (StreamWriter streamWriter = File.CreateText(h))
            {
                streamWriter.Write(string.Format(hCode, name, _namespace));
            }

            string[] files = new string[] { cpp, h };
            
            for (int i = 0; i < 3; i++)
            {
                // try to add files to Visual Studio solution up to three times every second, in case VS is still opening,
                // or there is some other issue.
                if (!VisualStudio.AddFilesToSolution(solution, projectName, files)) System.Threading.Thread.Sleep(1000);
                else break;
            }
        }

        private static string GetNamespaceFromProjectName()
        {
            string projectName = Project.Current.Name;
            projectName = string.Concat(projectName.Where(c => !char.IsWhiteSpace(c)));
            return projectName;
        }
    }
}
