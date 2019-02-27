﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MissMooseConfigurationApplication.UIComponents
{
    /// <summary>
    /// Interaction logic for LineWithBorder.xaml
    /// </summary>
    public partial class LineWithBorder : UserControl
    {
        private Color firstHalfColor;
        private Color secondHalfColor;

        public enum LineHalves
        {
            FirstHalf, // the "first" half is always the rightmost or topmost half of the line
            SecondHalf
        }

        public int X1
        {
            set
            {
                BorderLine.X1 += value;
                ColoredLine.X1 += value;
            }
        }

        public int X2
        {
            set
            {
                BorderLine.X2 += value;
                ColoredLine.X2 += value;
            }
        }

        public int Y1
        {
            set
            {
                BorderLine.Y1 += value;
                ColoredLine.Y1 += value;
            }
        }

        public int Y2
        {
            set
            {
                BorderLine.Y2 += value;
                ColoredLine.Y2 += value;
            }
        }

        public int Thickness
        {
            set
            {
                BorderLine.StrokeThickness = value;
                ColoredLine.StrokeThickness = value - 4;
            }
        }

        public bool isVertical
        {
            set
            {
                if (value)
                {
                    ColoredLine.Y1 -= 3;
                    ColoredLine.Y2 += 3;
                    ColoredLine.UpdateLayout();
                }
                else
                {
                    ColoredLine.X1 -= 3;
                    ColoredLine.X2 += 3;
                    ColoredLine.UpdateLayout();
                }
            }
        }

        public LineWithBorder()
        {
            InitializeComponent();
        }

        public void SetColour(LineHalves half, Color new_color)
        {
            switch(half)
            {
                case LineHalves.FirstHalf:
                    firstHalfColor = new_color;
                    LineSeg1.Color = firstHalfColor;
                    LineSeg2.Color = firstHalfColor;
                    break;
                case LineHalves.SecondHalf:
                    secondHalfColor = new_color;
                    LineSeg3.Color = secondHalfColor;
                    LineSeg4.Color = secondHalfColor;
                    break;
            }
        }

        public void SetVisibility(LineHalves half, bool visible)
        {
            switch (half)
            {
                case LineHalves.FirstHalf:
                    if (visible)
                    {
                        BorderSeg1.Color = Brushes.Black.Color;
                        BorderSeg2.Color = Brushes.Black.Color;
                        LineSeg1.Color = firstHalfColor;
                        LineSeg2.Color = firstHalfColor;
                    }
                    else
                    {
                        BorderSeg1.Color = Brushes.Transparent.Color;
                        BorderSeg2.Color = Brushes.Transparent.Color;
                        LineSeg1.Color = Brushes.Transparent.Color;
                        LineSeg2.Color = Brushes.Transparent.Color;
                    }
                    break;
                case LineHalves.SecondHalf:
                    if (visible)
                    {
                        BorderSeg3.Color = Brushes.Black.Color;
                        BorderSeg4.Color = Brushes.Black.Color;
                        LineSeg3.Color = secondHalfColor;
                        LineSeg4.Color = secondHalfColor;
                    }
                    else
                    {
                        BorderSeg3.Color = Brushes.Transparent.Color;
                        BorderSeg4.Color = Brushes.Transparent.Color;
                        LineSeg3.Color = Brushes.Transparent.Color;
                        LineSeg4.Color = Brushes.Transparent.Color;
                    }
                    break;
            }
        }

        public class LineSegment
        {
            public LineWithBorder line;
            public LineWithBorder.LineHalves half;
            public LineSegment(LineWithBorder line, LineWithBorder.LineHalves half)
            {
                this.line = line;
                this.half = half;
            }
        }
    }
}
