using HavanaEditor.Components;
using HavanaEditor.GameProject;
using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
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
    /// Used to convert nullable boolean types to non-nullable
    /// boolean types.
    /// </summary>
    public class NullableBoolToBoolConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            // If value is null or false, return false.
            // If value is true, return true.
            return value is bool b && b == true;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is bool b && b == true;
        }
    }

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
            propertyName = string.Empty;
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

        private void OnAddScriptComponent(object sender, RoutedEventArgs e)
        {
            AddComponent(ComponentType.Script, (sender as MenuItem).Header.ToString());
        }
        
        private void OnAddComponent_Button_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            ContextMenu menu = FindResource("addComponentMenu") as ContextMenu;
            ToggleButton button = sender as ToggleButton;
            button.IsChecked = true;
            menu.Placement = PlacementMode.Bottom;
            menu.PlacementTarget = button;
            menu.MinWidth = button.ActualWidth;
            menu.IsOpen = true;
        }

        private Action GetRenameAction()
        {
            MSEntity viewModel = DataContext as MSEntity;
            var selection = viewModel.SelectedEntities.Select(entity => (entity, entity.Name)).ToList();
           
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

        private void AddComponent(ComponentType componentType, object data)
        {
            var creationFunction = ComponentFactory.GetCreationFunction(componentType);
            var changedEntities = new List<(GameEntity entity, Component component)>();
            MSEntity viewModel = DataContext as MSEntity;
            foreach (var entity in viewModel.SelectedEntities)
            {
                Component component = creationFunction(entity, data);
                if (entity.AddComponent(component))
                {
                    changedEntities.Add((entity, component));
                }
            }

            if (changedEntities.Any())
            {
                viewModel.Refresh();

                Project.UndoRedo.Add(new UndoRedoAction(
                () =>
                {
                    changedEntities.ForEach(x => x.entity.RemoveComponent(x.component));
                    (DataContext as MSEntity).Refresh();
                },
                () =>
                {
                    changedEntities.ForEach(x => x.entity.AddComponent(x.component));
                    (DataContext as MSEntity).Refresh();
                },
                $"Add {componentType} component."));
            }
        }

    }
}
