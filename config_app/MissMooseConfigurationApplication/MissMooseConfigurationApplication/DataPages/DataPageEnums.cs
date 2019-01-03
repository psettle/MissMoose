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

    public enum NodeRotation : byte
    {
        Rotation0Degrees = 0,
        Rotation45Degrees = 1,
        Rotation90Degrees = 2,
        Rotation135Degrees = 3,
        Rotation180Degrees = 4,
        Rotation225Degrees = 5,
        Rotation270Degrees = 6,
        Rotation315Degrees = 7
    }
}
