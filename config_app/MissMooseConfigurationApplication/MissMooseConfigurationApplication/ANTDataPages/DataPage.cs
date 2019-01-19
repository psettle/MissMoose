using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    /*
     * Abstract base class for all data pages
     */ 
    public abstract class DataPage : INotifyPropertyChanged
    {       
        #region Public Properties

        public abstract byte DataPageNumber
        {
            get;
        }

        public uint DataPageLength { get; } = 8;

        #endregion

        #region Public Methods

        /* Implement in all subclasses.
         * Encodes the current values of this page's data fields into the given txBuffer.
         */
        public abstract void Encode(byte[] txBuffer);

        /* Implement in all subclasses.
         * Decodes the given rxBuffer into this page's data fields.
         */
        public abstract void Decode(byte[] rxBuffer);

        #endregion

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }
}
