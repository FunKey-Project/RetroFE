﻿using System;
using System.Windows.Data;

namespace Configuration.Converter
{
    public class CollectionExistsConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
//            if (targetType != typeof(bool))
//                throw new InvalidOperationException("The target is not a bool");
            return true;
         //   return !(bool)value;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
