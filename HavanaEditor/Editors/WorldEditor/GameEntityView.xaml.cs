using HavanaEditor.Components;
using HavanaEditor.GameProject;
using HavanaEditor.Utilities;
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
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace HavanaEditor.Editors
{
    /// <summary>
    /// Interaction logic for GameEntityView.xaml
    /// </summary>
    public partial class GameEntityView : UserControl
    {
        // STATE
        Action undoAction;
        string propertyName;
        
        // PROPERTIES
        public static GameEntityView Instance { get; private set; }

        // PUBLIC
        public GameEntityView()
        {
            InitializeComponent();
            DataContext = null;
            Instance = this;
            DataContextChanged += (_, __) => 
            {
                if (DataContext != null)
                {
                    (DataContext as MSEntity).PropertyChanged += (s, e) => propertyName = e.PropertyName;
                }
            };
        }

        // PRIVATE
        private void OnName_TextBox_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            undoAction = GetRenameAction();
        }

        private void OnName_TextBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (propertyName == nameof(MSEntity.Name) && undoAction != null)
            {
                Action redoAction = GetRenameAction();
                Project.UndoRedo.Add(new UndoRedoAction(undoAction, redoAction, "Rename game entity"));
                propertyName = null;
            }
            undoAction = null;
        }

        private void OnIsEnabled_CheckBox_Click(object sender, RoutedEventArgs e)
        {
            Action undoAction = GetIsEnabledAction();
            MSEntity viewModel = DataContext as MSEntity;
            viewModel.IsEnabled = (sender as CheckBox).IsChecked == true;
            Action redoAction = GetIsEnabledAction();
            Project.UndoRedo.Add(new UndoRedoAction(undoAction, redoAction,
                viewModel.IsEnabled == true ? "Enabled Game Entity" : "Disabled Game Entity"));
        }

        private Action GetRenameAction()
        {
            MSEntity viewModel = DataContext as MSEntity;
            var selection = viewModel.SelectedEntities.Select(entity => (entity, entity.Name)).ToList();
            
            foreach (var entity in selection)
            {
                Logger.Log(MessageTypes.Info, $"{entity.Name}");
            }
           
            return new Action(() =>
            {
                selection.ForEach(item => item.entity.Name = item.Name);
                (DataContext as MSEntity).Refresh();
            });
        }

        private Action GetIsEnabledAction()
        {
            MSEntity viewModel = DataContext as MSEntity;
            var selection = viewModel.SelectedEntities.Select(entity => (entity, entity.IsEnabled)).ToList();
            return new Action(() =>
            {
                selection.ForEach(item => item.entity.IsEnabled = item.IsEnabled);
                (DataContext as MSEntity).Refresh();
            });
        }
    }
}
