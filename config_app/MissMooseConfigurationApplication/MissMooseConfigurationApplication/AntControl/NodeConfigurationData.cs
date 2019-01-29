using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    class NodeConfigurationData
    {
        public HardwareConfiguration nodeType;
        public Rotation nodeRotation;
        public int xpos;
        public int ypos;
        public int xoffset;
        public int yoffset;
        public bool isSent = false;

        public NodeConfigurationData(HardwareConfiguration nodeType, Rotation nodeRotation, int xpos, int ypos, int xoffset, int yoffset)
        {
            this.nodeType = nodeType;
            this.nodeRotation = nodeRotation;
            this.xpos = xpos;
            this.ypos = ypos;
            this.xoffset = xoffset;
            this.yoffset = yoffset;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is NodeConfigurationData nodeConfigData))
            {
                return false;
            }

            if ((nodeConfigData.xpos != xpos) ||
                (nodeConfigData.ypos != ypos) ||
                (nodeConfigData.xoffset != xoffset) ||
                (nodeConfigData.yoffset != yoffset))
            {
                return false;
            }

            return (this.nodeType == nodeConfigData.nodeType
                && this.nodeRotation.ToEnum() == nodeConfigData.nodeRotation.ToEnum());
        }

        public override int GetHashCode()
        {
            throw new InvalidOperationException("Shouldn't be called");
        }
    }
}
