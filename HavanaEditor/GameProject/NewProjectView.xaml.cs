using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace HavanaEditor.GameProject
{
    /// <summary>
    /// Interaction logic for NewProjectView.xaml
    /// </summary>
    public partial class NewProjectView : UserControl
    {
        // PUBLIC
        public NewProjectView()
        {
            InitializeComponent();
        }

        // PRIVATE
        private void OnCreate_Button_Click(object sender, RoutedEventArgs e)
        {
            NewProject viewModel = DataContext as NewProject;
            string projectPath = viewModel.CreateProject(templateListBox.SelectedItem as ProjectTemplate);
            bool dialogResult = false;
            Window window = Window.GetWindow(this);
            if (!string.IsNullOrEmpty(projectPath))
            {
                dialogResult = true;
                Project project = OpenProject.Open(new ProjectData() { ProjectName = viewModel.ProjectName, ProjectPath = projectPath });
                window.DataContext = project;
            }
            window.DialogResult = dialogResult;
            window.Close();
        }
    }
}
