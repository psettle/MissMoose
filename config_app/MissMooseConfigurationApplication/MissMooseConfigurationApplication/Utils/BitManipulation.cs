using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MissMooseConfigurationApplication
{
    class BitManipulation
    {
        public static readonly byte ReservedOnes = 0xFF;
        public static readonly byte ReservedZeros = 0x00;

        public static byte GetByte0(UInt16 field)
        {
            return (byte)(field & 0x00FF);
        }

        public static byte GetByte1(UInt16 field)
        {
            return (byte)(field & 0xFF00);
        }

        public static byte GetByte0(UInt32 field)
        {
            return (byte)(field & 0x000000FF);
        }

        public static byte GetByte1(UInt32 field)
        {
            return (byte)(field & 0x0000FF00);
        }

        public static byte GetByte2(UInt32 field)
        {
            return (byte)(field & 0x00FF0000);
        }

        public static byte GetByte3(UInt32 field)
        {
            return (byte)(field & 0xFF000000);
        }

        public static void SetByte0(UInt16 field, byte value)
        {
            field &= 0xFF00;
            field |= value;
        }

        public static void SetByte1(UInt16 field, byte value)
        {
            field &= 0x00FF;
            field |= (UInt16)(value << 8);
        }

        public static void SetByte0(UInt32 field, byte value)
        {
            field &= 0xFFFFFF00;
            field |= value;
        }

        public static void SetByte1(UInt32 field, byte value)
        {
            field &= 0xFFFF00FF;
            field |= (UInt32)(value << 8);
        }

        public static void SetByte2(UInt32 field, byte value)
        {
            field &= 0xFF00FFFF;
            field |= (UInt32)(value << 16);
        }

        public static void SetByte3(UInt32 field, byte value)
        {
            field &= 0x00FFFFFF;
            field |= (UInt32)(value << 24);
        }
    }
}
