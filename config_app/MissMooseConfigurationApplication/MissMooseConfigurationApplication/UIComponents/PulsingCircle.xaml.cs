using System;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
using MissMooseConfigurationApplication.UIPages;

namespace MissMooseConfigurationApplication
{
    public partial class PulsingCircle : UserControl
    {
        #region Private Members

        Timer timer;

        double minRadius;
        double maxRadius;
        double currentRadius;
        double period;
        bool isGrowing;

        const int timerPeriod = 40;
        const double defaultRadius = 10;

        #endregion

        #region Public Methods

        public PulsingCircle()
        {
            InitializeComponent();

            Configure(defaultRadius, defaultRadius, 1);

            timer = new Timer(new TimerCallback(OnTick), null, timerPeriod, timerPeriod);
        }

        public void Configure(double minRadiusPx, double maxRadiusPx, double periodS)
        {       
            isGrowing = true;
            minRadius = minRadiusPx;
            maxRadius = maxRadiusPx;
            period = periodS;

            SetRadius(minRadius);
        }

        #endregion

        #region Private Methods

        private void OnTick(object state)
        {
            double newRadius = currentRadius;
            double delta = 2 * (maxRadius - minRadius) / (period * 1000 / timerPeriod);


            if (isGrowing)
            {
                newRadius += delta;

                if(newRadius >= maxRadius)
                {
                    newRadius = maxRadius;
                    isGrowing = false;
                }
            }
            else
            {
                newRadius -= delta;

                if(newRadius <= minRadius)
                {
                    newRadius = minRadius;
                    isGrowing = true;
                }
            }

            SetRadius(newRadius);
        }

        private void SetRadius(double radius)
        {
            currentRadius = radius;

            Dispatcher.Invoke(() =>
            {
                Circle.Height = radius;
                Circle.Width = radius;
            });
        }

        #endregion
    }
}
