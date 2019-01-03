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

        public static bool GetBit(byte field, byte bitIndex)
        {
            return (field & (0x01 << bitIndex)) != 0;
        }

        public static void SetBit(ref byte field, byte bitIndex, bool value)
        {
            field &= (byte) (~(0x01 << bitIndex));
            field |= (byte) (value? 0x00 : 0x01 << bitIndex);
        }

        public static byte GetByte0(UInt16 field)
        {
            return (byte)(field & 0x00FF);
        }

        public static byte GetByte1(UInt16 field)
        {
            return (byte)((field & 0xFF00) >> 8);
        }

        public static byte GetByte0(UInt32 field)
        {
            return (byte)(field & 0x000000FF);
        }

        public static byte GetByte1(UInt32 field)
        {
            return (byte)((field & 0x0000FF00) >> 8);
        }

        public static byte GetByte2(UInt32 field)
        {
            return (byte)((field & 0x00FF0000) >> 16);
        }

        public static byte GetByte3(UInt32 field)
        {
            return (byte)((field & 0xFF000000) >> 24);
        }

        public static void SetByte0(ref UInt16 field, byte value)
        {
            field &= 0xFF00;
            field |= value;
        }

        public static void SetByte1(ref UInt16 field, byte value)
        {
            field &= 0x00FF;
            field |= (UInt16)(value << 8);
        }

        public static void SetByte0(ref UInt32 field, byte value)
        {
            field &= 0xFFFFFF00;
            field |= value;
        }

        public static void SetByte1(ref UInt32 field, byte value)
        {
            field &= 0xFFFF00FF;
            field |= (UInt32)(value << 8);
        }

        public static void SetByte2(ref UInt32 field, byte value)
        {
            field &= 0xFF00FFFF;
            field |= (UInt32)(value << 16);
        }

        public static void SetByte3(ref UInt32 field, byte value)
        {
            field &= 0x00FFFFFF;
            field |= (UInt32)(value << 24);
        }
    }
}
