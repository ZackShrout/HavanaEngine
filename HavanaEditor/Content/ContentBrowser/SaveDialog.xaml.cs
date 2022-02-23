﻿using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.IO;
using System.ComponentModel;

namespace HavanaEditor.Content
{
    /// <summary>
    /// Interaction logic for SaveDialog.xaml
    /// </summary>
    public partial class SaveDialog : Window
    {
        // PROPERTIES
        public string SaveFilePath { get; private set; }

        // PUBLIC
        public SaveDialog()
        {
            InitializeComponent();
            Closing += OnSaveDialogClosing;
        }

        // PRIVATE
        private bool ValidateFileName(out string saveFilePath)
        {
            ContentBrowser contentBrowser = contentBrowserView.DataContext as ContentBrowser;
            string path = contentBrowser.SelectedFolder;
            
            if (!Path.EndsInDirectorySeparator(path)) path += @"\";
            
            string fileName = fileNameTextBox.Text.Trim();
            if (string.IsNullOrEmpty(fileName))
            {
                saveFilePath = string.Empty;
                return false;
            }

            if (!fileName.EndsWith(Asset.AssetFileExtension))
                fileName += Asset.AssetFileExtension;

            path += $@"{fileName}";
            bool isValid = false;
            string errorMsg = string.Empty;

            if (fileName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                errorMsg = "Invalid character(s) used in asset file name.";
            }
            else if (File.Exists(path) &&
                MessageBox.Show("File already exists. Overwrite?", "Overwrite File", MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.No)
            {
                // Do nothing... just return false
            }
            else
            {
                isValid = true;
            }

            if (!string.IsNullOrEmpty(errorMsg))
            {
                MessageBox.Show(errorMsg, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            saveFilePath = path;
            return isValid;
        }

        private void OnSave_Button_Click(object sender, RoutedEventArgs e)
        {
            if (ValidateFileName(out var saveFilePath))
            {
                SaveFilePath = saveFilePath;
                DialogResult = true;
                Close();
            }
        }

        private void OnContentBrowser_Mouse_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            if ((e.OriginalSource as FrameworkElement).DataContext == contentBrowserView.SelectedItem &&
                contentBrowserView.SelectedItem.FileName == fileNameTextBox.Text)
            {
                OnSave_Button_Click(sender, null);
            }
        }
        private void OnSaveDialogClosing(object sender, CancelEventArgs e)
        {
            contentBrowserView.Dispose();
        }

    }
}
