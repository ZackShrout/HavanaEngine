using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace HavanaEditor.Common
{
    static class VisualExtensions
    {
        public static T FindVisualParent<T>(this DependencyObject depObject) where T : DependencyObject
        {
            if (!(depObject is Visual)) return null;

            DependencyObject parent = VisualTreeHelper.GetParent(depObject);

            while (parent != null)
            {
                if (parent is T type)
                {
                    return type;
                }
                parent = VisualTreeHelper.GetParent(parent);
            }

            return null;
        }
    }
}
