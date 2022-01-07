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
        private double _originalValue = 0.0;
        private double _mouseXStart = 0.0;
        private double _multiplier = 0.01;
        private bool _captured = false;
        private bool _valueChanged = false;

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
            if (_captured)
            {
                double mouseX = e.GetPosition(this).X;
                double difference = mouseX - _mouseXStart;
                if (Math.Abs(difference) > SystemParameters.MinimumHorizontalDragDistance)
                {
                    if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control)) _multiplier = 0.001;
                    else if (Keyboard.Modifiers.HasFlag(ModifierKeys.Shift)) _multiplier = 0.1;
                    else _multiplier = 0.01;
                    double newValue = _originalValue + (difference * _multiplier * Multiplier);
                    Value = newValue.ToString("G5");
                    _valueChanged = true;
                }
            }
        }

        private void OnTextBlock_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            if (_captured)
            {
                Mouse.Capture(null);
                _captured = false;
                e.Handled = true;
                if (!_valueChanged && GetTemplateChild("PART_textBox") is TextBox textBox)
                {
                    textBox.Visibility = Visibility.Visible;
                    textBox.Focus();
                    textBox.SelectAll();
                }
            }
        }

        private void OnTextBlock_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            double.TryParse(Value, out _originalValue);

            Mouse.Capture(sender as UIElement);
            _captured = true;
            _valueChanged = false;
            e.Handled = true;
            _multiplier = 0.01;
            _mouseXStart = e.GetPosition(this).X;
            Focus();
        }
        
        private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as NumberBox).RaiseEvent(new RoutedEventArgs(ValueChangedEvent));
        }
    }
}
