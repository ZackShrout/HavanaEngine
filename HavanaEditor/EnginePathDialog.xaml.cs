using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace HavanaEditor
{
    /// <summary>
    /// Interaction logic for EnginePathDialog.xaml
    /// </summary>
    public partial class EnginePathDialog : Window
    {
        // PROPERTIES
        public string HavanaPath { get; private set; }

        // PUBLIC
        public EnginePathDialog()
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;
        }

        // PRIVATE
        private void OnOK_Button_Click(object sender, RoutedEventArgs e)
        {
            string path = pathTextBox.Text.Trim();
            messageTextBlock.Text = string.Empty;
            
            if (string.IsNullOrEmpty(path))
            {
                messageTextBlock.Text = "Invalid path.";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                messageTextBlock.Text = "Invalid character(s) in path.";
            }
            else if (!Directory.Exists(Path.Combine(path + @"Engine\EngineAPI")))
            {
                messageTextBlock.Text = "Unable to find Havana at the specified location.";
            }

            if (string.IsNullOrEmpty(messageTextBlock.Text))
            {
                if (!Path.EndsInDirectorySeparator(path)) path += @"\";
                HavanaPath = path;
                DialogResult = true;
                Close();
            }
        }
    }
}
