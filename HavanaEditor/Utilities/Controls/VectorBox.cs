using System.Windows;
using System.Windows.Controls;

namespace HavanaEditor.Utilities.Controls
{
    public enum VectorType
    {
        Vector2,
        Vector3,
        Vector4
    }
    
    class VectorBox : Control
    {
        // PROPERTIES
        public string X
        {
            get => (string)GetValue(XProperty);
            set => SetValue(XProperty, value);
        }
        public string Y
        {
            get => (string)GetValue(YProperty);
            set => SetValue(YProperty, value);
        }
        public string Z
        {
            get => (string)GetValue(ZProperty);
            set => SetValue(ZProperty, value);
        }
        public string W
        {
            get => (string)GetValue(WProperty);
            set => SetValue(WProperty, value);
        }
        public double Multiplier
        {
            get => (double)GetValue(MultiplierProperty);
            set => SetValue(MultiplierProperty, value);
        }
        public VectorType VectorType
        {
            get => (VectorType)GetValue(VectorTypeProperty);
            set => SetValue(VectorTypeProperty, value);
        }
        public Orientation Orientation
        {
            get => (Orientation)GetValue(OrientationProperty);
            set => SetValue(OrientationProperty, value);
        }

        // CONFIG DATA
        public static readonly DependencyProperty XProperty = DependencyProperty.Register(nameof(X), typeof(string), typeof(VectorBox),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));
        public static readonly DependencyProperty YProperty = DependencyProperty.Register(nameof(Y), typeof(string), typeof(VectorBox),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));
        public static readonly DependencyProperty ZProperty = DependencyProperty.Register(nameof(Z), typeof(string), typeof(VectorBox),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));
        public static readonly DependencyProperty WProperty = DependencyProperty.Register(nameof(W), typeof(string), typeof(VectorBox),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));
        public static readonly DependencyProperty MultiplierProperty = DependencyProperty.Register(nameof(Multiplier), typeof(double), typeof(VectorBox),
            new FrameworkPropertyMetadata(1.0));
        public static readonly DependencyProperty VectorTypeProperty = DependencyProperty.Register(nameof(VectorType), typeof(VectorType), typeof(VectorBox),
            new FrameworkPropertyMetadata(VectorType.Vector3));
        public static readonly DependencyProperty OrientationProperty = DependencyProperty.Register(nameof(Orientation), typeof(Orientation), typeof(VectorBox),
            new FrameworkPropertyMetadata(Orientation.Horizontal));

        // PUBLIC
        static VectorBox()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(VectorBox),
                new FrameworkPropertyMetadata(typeof(VectorBox)));
        }
    }
}
