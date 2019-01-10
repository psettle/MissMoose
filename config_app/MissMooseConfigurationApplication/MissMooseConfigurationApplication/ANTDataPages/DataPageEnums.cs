using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    public enum NodeType : byte
    {
        Unknown = 0,
        TwoPir = 1,
        OneLidarOnePir = 2,
        OneLidarOnePirWithLed = 3,
    }
}
