using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace HavanaEditor.GameProject
{
    /// <summary>
    /// Interaction logic for ProjectBrowserDialogue.xaml
    /// </summary>
    public partial class ProjectBrowserDialogue : Window
    {
        // STATE
        private readonly CubicEase easing = new CubicEase() { EasingMode = EasingMode.EaseInOut };

        // PROPERTIES
        public static bool GotoNewProjectTab { get; set; }

        // PUBLIC
        public ProjectBrowserDialogue()
        {
            InitializeComponent();
            Loaded += OnProjectBrowserDialogueLoaded;
        }

        private void OnProjectBrowserDialogueLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnProjectBrowserDialogueLoaded;
            if (!OpenProject.Projects.Any() || GotoNewProjectTab)
            {
                if (!GotoNewProjectTab)
                {
                    openProjectButton.IsEnabled = false;
                    openProjectView.Visibility = Visibility.Hidden;
                }

                OnToggleButton_Click(newProjectButton, new RoutedEventArgs());
            }

            GotoNewProjectTab = false;
        }

        // PRIVATE
        private void OnToggleButton_Click(object sender, RoutedEventArgs e)
        {
            if (sender == openProjectButton)
            {
                if (newProjectButton.IsChecked == true)
                {
                    newProjectButton.IsChecked = false;
                    AnimateToOpenProject();
                    openProjectView.IsEnabled = true;
                    newProjectView.IsEnabled = false;
                }
                openProjectButton.IsChecked = true;
            }
            else
            {
                if (openProjectButton.IsChecked == true)
                {
                    openProjectButton.IsChecked = false;
                    AnimateToCreateProject();
                    openProjectView.IsEnabled = false;
                    newProjectView.IsEnabled = true;
                }
                newProjectButton.IsChecked = true;
            }
        }

        private void AnimateToCreateProject()
        {
            DoubleAnimation highlightAnimation = new DoubleAnimation(200, 400, new Duration(TimeSpan.FromSeconds(0.2)));
            highlightAnimation.EasingFunction = easing;
            highlightAnimation.Completed += (s, e) =>
            {
                ThicknessAnimation animation = new ThicknessAnimation(new Thickness(0), new Thickness(-1600, 0, 0, 0), new Duration(TimeSpan.FromSeconds(0.5)));
                animation.EasingFunction = easing;
                browserContent.BeginAnimation(MarginProperty, animation);
            };
            highlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }

        private void AnimateToOpenProject()
        {
            DoubleAnimation highlightAnimation = new DoubleAnimation(400, 200, new Duration(TimeSpan.FromSeconds(0.2)));
            highlightAnimation.EasingFunction = easing;
            highlightAnimation.Completed += (s, e) => 
            {
                ThicknessAnimation animation = new ThicknessAnimation(new Thickness(-1600, 0, 0, 0), new Thickness(0), new Duration(TimeSpan.FromSeconds(0.5)));
                animation.EasingFunction = easing;
                browserContent.BeginAnimation(MarginProperty, animation);
            };
            highlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }
    }
}
