using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace HavanaEditor.Utilities.Controls
{
    /// <summary>
    /// Number box to set number values throughout the application.
    /// </summary>
    [TemplatePart(Name = "PART_textBlock", Type = typeof(TextBlock))]
    [TemplatePart(Name = "PART_textBox", Type = typeof(TextBox))]
    class NumberBox : Control
    {
        // STATE
        private double originalValue = 0.0;
        private double mouseXStart = 0.0;
        private double multiplier = 0.01;
        private bool captured = false;
        private bool valueChanged = false;

        // PROPERTIES
        public string Value 
        {
            get => (string)GetValue(ValueProperty);
            set => SetValue(ValueProperty, value);
        }
        public double Multiplier
        {
            get => (double)GetValue(MultiplierProperty);
            set => SetValue(MultiplierProperty, value);
        }

        // CONFIG DATA
        public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(nameof(Value), typeof(string), typeof(NumberBox),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
            new PropertyChangedCallback(OnValueChanged)));

        public static readonly DependencyProperty MultiplierProperty = DependencyProperty.Register(nameof(Multiplier), typeof(double), typeof(NumberBox),
            new FrameworkPropertyMetadata(1.0));

        // EVENTS
        public event RoutedEventHandler ValueChanged
        {
            add => AddHandler(ValueChangedEvent, value);
            remove => RemoveHandler(ValueChangedEvent, value);
        }

        public static readonly RoutedEvent ValueChangedEvent =
            EventManager.RegisterRoutedEvent(nameof(ValueChanged), RoutingStrategy.Bubble, typeof(RoutedEventHandler), typeof(NumberBox));

        // PUBLIC
        static NumberBox()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(NumberBox), 
                new FrameworkPropertyMetadata(typeof(NumberBox)));
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            if (GetTemplateChild("PART_textBlock") is TextBlock textBlock)
            {
                textBlock.MouseLeftButtonDown += OnTextBlock_Mouse_LBD;
                textBlock.MouseLeftButtonUp += OnTextBlock_Mouse_LBU;
                textBlock.MouseMove += OnTextBlock_Mouse_Move;
            }
        }

        // PRIVATE
        private void OnTextBlock_Mouse_Move(object sender, MouseEventArgs e)
        {
            if (captured)
            {
                double mouseX = e.GetPosition(this).X;
                double difference = mouseX - mouseXStart;
                if (Math.Abs(difference) > SystemParameters.MinimumHorizontalDragDistance)
                {
                    if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control)) multiplier = 0.001;
                    else if (Keyboard.Modifiers.HasFlag(ModifierKeys.Shift)) multiplier = 0.1;
                    else multiplier = 0.01;
                    double newValue = originalValue + (difference * multiplier * Multiplier);
                    Value = newValue.ToString("0.#####");
                    valueChanged = true;
                }
            }
        }

        private void OnTextBlock_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            if (captured)
            {
                Mouse.Capture(null);
                captured = false;
                e.Handled = true;
                if (!valueChanged && GetTemplateChild("PART_textBox") is TextBox textBox)
                {
                    textBox.Visibility = Visibility.Visible;
                    textBox.Focus();
                    textBox.SelectAll();
                }
            }
        }

        private void OnTextBlock_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            double.TryParse(Value, out originalValue);

            Mouse.Capture(sender as UIElement);
            captured = true;
            valueChanged = false;
            e.Handled = true;
            multiplier = 0.01;
            mouseXStart = e.GetPosition(this).X;
            Focus();
        }
        
        private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as NumberBox).RaiseEvent(new RoutedEventArgs(ValueChangedEvent));
        }
    }
}
