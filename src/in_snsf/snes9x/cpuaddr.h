/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/


#ifndef _CPUADDR_H_
#define _CPUADDR_H_

typedef enum
{
	NONE   = 0,
	READ   = 1,
	WRITE  = 2,
	MODIFY = 3,
	JUMP   = 5,
	JSR    = 8
}	AccessMode;

static inline uint8_t Immediate8Slow (AccessMode a)
{
	uint8_t	val = S9xGetByte(Registers.PBPC);
	if (a & READ)
		OpenBus = val;
	Registers.PCw++;

	return val;
}

static inline uint8_t Immediate8 (AccessMode a)
{
	uint8_t	val = CPU.PCBase[Registers.PCw];
	if (a & READ)
		OpenBus = val;
	AddCycles(CPU.MemSpeed);
	Registers.PCw++;

	return val;
}

static inline uint16_t Immediate16Slow (AccessMode a)
{
	uint16_t	val = S9xGetWord(Registers.PBPC, WRAP_BANK);
	if (a & READ)
		OpenBus = (uint8_t) (val >> 8);
	Registers.PCw += 2;

	return val;
}

static inline uint16_t Immediate16 (AccessMode a)
{
	uint16_t	val = READ_WORD(CPU.PCBase + Registers.PCw);
	if (a & READ)
		OpenBus = (uint8_t) (val >> 8);
	AddCycles(CPU.MemSpeedx2);
	Registers.PCw += 2;

	return val;
}

static inline uint32_t RelativeSlow (AccessMode a)						// branch $xx
{
	int8_t	offset = Immediate8Slow(a);

	return ((int16_t) Registers.PCw + offset) & 0xffff;
}

static inline uint32_t Relative (AccessMode a)							// branch $xx
{
	int8_t	offset = Immediate8(a);

	return ((int16_t) Registers.PCw + offset) & 0xffff;
}

static inline uint32_t RelativeLongSlow (AccessMode a)					// BRL $xxxx
{
	int16_t	offset = Immediate16Slow(a);

	return ((int32_t) Registers.PCw + offset) & 0xffff;
}

static inline uint32_t RelativeLong (AccessMode a)						// BRL $xxxx
{
	int16_t	offset = Immediate16(a);

	return ((int32_t) Registers.PCw + offset) & 0xffff;
}

static inline uint32_t AbsoluteIndexedIndirectSlow (AccessMode a)			// (a,X)
{
	uint16_t	addr;

	if (a & JSR)
	{
		// JSR (a,X) pushes the old address in the middle of loading the new.
		// OpenBus needs to be set to account for this.
		addr = Immediate8Slow(READ);
		if (a == JSR)
			OpenBus = Registers.PCl;
		addr |= Immediate8Slow(READ) << 8;
	}
	else
		addr = Immediate16Slow(READ);

	AddCycles(ONE_CYCLE);
	addr += Registers.X.W;

	// Address load wraps within the bank
	uint16_t	addr2 = S9xGetWord(ICPU.ShiftedPB | addr, WRAP_BANK);
	OpenBus = addr2 >> 8;

	return addr2;
}

static inline uint32_t AbsoluteIndexedIndirect (AccessMode a)				// (a,X)
{
	uint16_t	addr = Immediate16Slow(READ);
	addr += Registers.X.W;

	// Address load wraps within the bank
	uint16_t	addr2 = S9xGetWord(ICPU.ShiftedPB | addr, WRAP_BANK);
	OpenBus = addr2 >> 8;

	return addr2;
}

static inline uint32_t AbsoluteIndirectLongSlow (AccessMode a)			// [a]
{
	uint16_t	addr = Immediate16Slow(READ);

	// No info on wrapping, but it doesn't matter anyway due to mirroring
	uint32_t	addr2 = S9xGetWord(addr);
	OpenBus = addr2 >> 8;
	addr2 |= (OpenBus = S9xGetByte(addr + 2)) << 16;

	return addr2;
}

static inline uint32_t AbsoluteIndirectLong (AccessMode a)				// [a]
{
	uint16_t	addr = Immediate16(READ);

	// No info on wrapping, but it doesn't matter anyway due to mirroring
	uint32_t	addr2 = S9xGetWord(addr);
	OpenBus = addr2 >> 8;
	addr2 |= (OpenBus = S9xGetByte(addr + 2)) << 16;

	return addr2;
}

static inline uint32_t AbsoluteIndirectSlow (AccessMode a)				// (a)
{
	// No info on wrapping, but it doesn't matter anyway due to mirroring
	uint16_t	addr2 = S9xGetWord(Immediate16Slow(READ));
	OpenBus = addr2 >> 8;

	return addr2;
}

static inline uint32_t AbsoluteIndirect (AccessMode a)					// (a)
{
	// No info on wrapping, but it doesn't matter anyway due to mirroring
	uint16_t	addr2 = S9xGetWord(Immediate16(READ));
	OpenBus = addr2 >> 8;

	return addr2;
}

static inline uint32_t AbsoluteSlow (AccessMode a)						// a
{
	return ICPU.ShiftedDB | Immediate16Slow(a);
}

static inline uint32_t Absolute (AccessMode a)							// a
{
	return ICPU.ShiftedDB | Immediate16(a);
}

static inline uint32_t AbsoluteLongSlow (AccessMode a)					// l
{
	uint32_t	addr = Immediate16Slow(READ);

	// JSR l pushes the old bank in the middle of loading the new.
	// OpenBus needs to be set to account for this.
	if (a == JSR)
		OpenBus = Registers.PB;

	addr |= Immediate8Slow(a) << 16;

	return addr;
}

static inline uint32_t AbsoluteLong (AccessMode a)						// l
{
	uint32_t	addr = READ_3WORD(CPU.PCBase + Registers.PCw);
	AddCycles(CPU.MemSpeedx2 + CPU.MemSpeed);
	if (a & READ)
		OpenBus = addr >> 16;
	Registers.PCw += 3;

	return addr;
}

static inline uint32_t DirectSlow (AccessMode a)							// d
{
	uint16_t	addr = Immediate8Slow(a) + Registers.D.W;
	if (Registers.DL != 0)
		AddCycles(ONE_CYCLE);

	return addr;
}

static inline uint32_t Direct (AccessMode a)								// d
{
	uint16_t	addr = Immediate8(a) + Registers.D.W;
	if (Registers.DL != 0)
		AddCycles(ONE_CYCLE);

	return addr;
}

static inline uint32_t DirectIndirectSlow (AccessMode a)					// (d)
{
	uint32_t	addr = S9xGetWord(DirectSlow(READ), (!CheckEmulation() || Registers.DL) ? WRAP_BANK : WRAP_PAGE);
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);
	addr |= ICPU.ShiftedDB;

	return addr;
}

static inline uint32_t DirectIndirectE0 (AccessMode a)					// (d)
{
	uint32_t	addr = S9xGetWord(Direct(READ));
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);
	addr |= ICPU.ShiftedDB;

	return addr;
}

static inline uint32_t DirectIndirectE1 (AccessMode a)					// (d)
{
	uint32_t	addr = S9xGetWord(DirectSlow(READ), Registers.DL ? WRAP_BANK : WRAP_PAGE);
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);
	addr |= ICPU.ShiftedDB;

	return addr;
}

static inline uint32_t DirectIndirectIndexedSlow (AccessMode a)			// (d),Y
{
	uint32_t	addr = DirectIndirectSlow(a);
	if (a & WRITE || !CheckIndex() || (addr & 0xff) + Registers.YL >= 0x100)
		AddCycles(ONE_CYCLE);

	return addr + Registers.Y.W;
}

static inline uint32_t DirectIndirectIndexedE0X0 (AccessMode a)			// (d),Y
{
	uint32_t	addr = DirectIndirectE0(a);
	AddCycles(ONE_CYCLE);

	return addr + Registers.Y.W;
}

static inline uint32_t DirectIndirectIndexedE0X1 (AccessMode a)			// (d),Y
{
	uint32_t	addr = DirectIndirectE0(a);
	if (a & WRITE || (addr & 0xff) + Registers.YL >= 0x100)
		AddCycles(ONE_CYCLE);

	return addr + Registers.Y.W;
}

static inline uint32_t DirectIndirectIndexedE1 (AccessMode a)				// (d),Y
{
	uint32_t	addr = DirectIndirectE1(a);
	if (a & WRITE || (addr & 0xff) + Registers.YL >= 0x100)
		AddCycles(ONE_CYCLE);

	return addr + Registers.Y.W;
}

static inline uint32_t DirectIndirectLongSlow (AccessMode a)				// [d]
{
	uint16_t	addr = DirectSlow(READ);
	uint32_t	addr2 = S9xGetWord(addr);
	OpenBus = addr2 >> 8;
	addr2 |= (OpenBus = S9xGetByte(addr + 2)) << 16;

	return addr2;
}

static inline uint32_t DirectIndirectLong (AccessMode a)					// [d]
{
	uint16_t	addr = Direct(READ);
	uint32_t	addr2 = S9xGetWord(addr);
	OpenBus = addr2 >> 8;
	addr2 |= (OpenBus = S9xGetByte(addr + 2)) << 16;

	return addr2;
}

static inline uint32_t DirectIndirectIndexedLongSlow (AccessMode a)		// [d],Y
{
	return DirectIndirectLongSlow(a) + Registers.Y.W;
}

static inline uint32_t DirectIndirectIndexedLong (AccessMode a)			// [d],Y
{
	return DirectIndirectLong(a) + Registers.Y.W;
}

static inline uint32_t DirectIndexedXSlow (AccessMode a)					// d,X
{
	pair	addr;
	addr.W = DirectSlow(a);
	if (!CheckEmulation() || Registers.DL)
		addr.W += Registers.X.W;
	else
		addr.B.l += Registers.XL;

	AddCycles(ONE_CYCLE);

	return addr.W;
}

static inline uint32_t DirectIndexedXE0 (AccessMode a)					// d,X
{
	uint16_t	addr = Direct(a) + Registers.X.W;
	AddCycles(ONE_CYCLE);

	return addr;
}

static inline uint32_t DirectIndexedXE1 (AccessMode a)					// d,X
{
	if (Registers.DL)
		return DirectIndexedXE0(a);
	else
	{
		pair	addr;
		addr.W = Direct(a);
		addr.B.l += Registers.XL;
		AddCycles(ONE_CYCLE);

		return addr.W;
	}
}

static inline uint32_t DirectIndexedYSlow (AccessMode a)					// d,Y
{
	pair	addr;
	addr.W = DirectSlow(a);
	if (!CheckEmulation() || Registers.DL)
		addr.W += Registers.Y.W;
	else
		addr.B.l += Registers.YL;

	AddCycles(ONE_CYCLE);

	return addr.W;
}

static inline uint32_t DirectIndexedYE0 (AccessMode a)					// d,Y
{
	uint16_t	addr = Direct(a) + Registers.Y.W;
	AddCycles(ONE_CYCLE);

	return addr;
}

static inline uint32_t DirectIndexedYE1 (AccessMode a)					// d,Y
{
	if (Registers.DL)
		return DirectIndexedYE0(a);
	else
	{
		pair	addr;
		addr.W = Direct(a);
		addr.B.l += Registers.YL;
		AddCycles(ONE_CYCLE);

		return addr.W;
	}
}

static inline uint32_t DirectIndexedIndirectSlow (AccessMode a)			// (d,X)
{
	uint32_t	addr = S9xGetWord(DirectIndexedXSlow(READ), (!CheckEmulation() || Registers.DL) ? WRAP_BANK : WRAP_PAGE);
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);

	return ICPU.ShiftedDB | addr;
}

static inline uint32_t DirectIndexedIndirectE0 (AccessMode a)				// (d,X)
{
	uint32_t	addr = S9xGetWord(DirectIndexedXE0(READ));
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);

	return ICPU.ShiftedDB | addr;
}

static inline uint32_t DirectIndexedIndirectE1 (AccessMode a)				// (d,X)
{
	uint32_t	addr = S9xGetWord(DirectIndexedXE1(READ), Registers.DL ? WRAP_BANK : WRAP_PAGE);
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);

	return ICPU.ShiftedDB | addr;
}

static inline uint32_t AbsoluteIndexedXSlow (AccessMode a)				// a,X
{
	uint32_t	addr = AbsoluteSlow(a);
	if (a & WRITE || !CheckIndex() || (addr & 0xff) + Registers.XL >= 0x100)
		AddCycles(ONE_CYCLE);

	return addr + Registers.X.W;
}

static inline uint32_t AbsoluteIndexedXX0 (AccessMode a)					// a,X
{
	uint32_t	addr = Absolute(a);
	AddCycles(ONE_CYCLE);

	return addr + Registers.X.W;
}

static inline uint32_t AbsoluteIndexedXX1 (AccessMode a)					// a,X
{
	uint32_t	addr = Absolute(a);
	if (a & WRITE || (addr & 0xff) + Registers.XL >= 0x100)
		AddCycles(ONE_CYCLE);

	return addr + Registers.X.W;
}

static inline uint32_t AbsoluteIndexedYSlow (AccessMode a)				// a,Y
{
	uint32_t	addr = AbsoluteSlow(a);
	if (a & WRITE || !CheckIndex() || (addr & 0xff) + Registers.YL >= 0x100)
		AddCycles(ONE_CYCLE);

	return addr + Registers.Y.W;
}

static inline uint32_t AbsoluteIndexedYX0 (AccessMode a)					// a,Y
{
	uint32_t	addr = Absolute(a);
	AddCycles(ONE_CYCLE);

	return addr + Registers.Y.W;
}

static inline uint32_t AbsoluteIndexedYX1 (AccessMode a)					// a,Y
{
	uint32_t	addr = Absolute(a);
	if (a & WRITE || (addr & 0xff) + Registers.YL >= 0x100)
		AddCycles(ONE_CYCLE);

	return addr + Registers.Y.W;
}

static inline uint32_t AbsoluteLongIndexedXSlow (AccessMode a)			// l,X
{
	return AbsoluteLongSlow(a) + Registers.X.W;
}

static inline uint32_t AbsoluteLongIndexedX (AccessMode a)				// l,X
{
	return AbsoluteLong(a) + Registers.X.W;
}

static inline uint32_t StackRelativeSlow (AccessMode a)					// d,S
{
	uint16_t	addr = Immediate8Slow(a) + Registers.S.W;
	AddCycles(ONE_CYCLE);

	return addr;
}

static inline uint32_t StackRelative (AccessMode a)						// d,S
{
	uint16_t	addr = Immediate8(a) + Registers.S.W;
	AddCycles(ONE_CYCLE);

	return addr;
}

static inline uint32_t StackRelativeIndirectIndexedSlow (AccessMode a)	// (d,S),Y
{
	uint32_t	addr = S9xGetWord(StackRelativeSlow(READ));
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);
	addr = (addr + Registers.Y.W + ICPU.ShiftedDB) & 0xffffff;
	AddCycles(ONE_CYCLE);

	return addr;
}

static inline uint32_t StackRelativeIndirectIndexed (AccessMode a)		// (d,S),Y
{
	uint32_t	addr = S9xGetWord(StackRelative(READ));
	if (a & READ)
		OpenBus = (uint8_t) (addr >> 8);
	addr = (addr + Registers.Y.W + ICPU.ShiftedDB) & 0xffffff;
	AddCycles(ONE_CYCLE);

	return addr;
}

#endif
