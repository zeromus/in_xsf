/*
	Copyright (C) 2009-2011 DeSmuME Team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "firmware.h"
#include "NDSSystem.h"
#include "path.h"

//#define DWNUM(i) ((i) >> 2)
static inline uint32_t DWNUM(uint32_t i) { return i >> 2; }

bool CFIRMWARE::getKeyBuf()
{
	FILE *file = fopen(CommonSettings.ARM7BIOS, "rb");
	if (!file) return false;

	fseek(file, 0x30, SEEK_SET);
	size_t res = fread(keyBuf, 4, 0x412, file);
	fclose(file);
	return res == 0x412;
}

void CFIRMWARE::crypt64BitUp(uint32_t *ptr)
{
	uint32_t Y = ptr[0];
	uint32_t X = ptr[1];

	for(uint32_t i = 0x00; i <= 0x0F; i++)
	{
		uint32_t Z = (keyBuf[i] ^ X);
		X = keyBuf[DWNUM(0x048 + (((Z >> 24) & 0xFF) << 2))];
		X = (keyBuf[DWNUM(0x448 + (((Z >> 16) & 0xFF) << 2))] + X);
		X = (keyBuf[DWNUM(0x848 + (((Z >> 8) & 0xFF) << 2))] ^ X);
		X = (keyBuf[DWNUM(0xC48 + ((Z & 0xFF) << 2))] + X);
		X = (Y ^ X);
		Y = Z;
	}

	ptr[0] = (X ^ keyBuf[DWNUM(0x40)]);
	ptr[1] = (Y ^ keyBuf[DWNUM(0x44)]);
}

void CFIRMWARE::crypt64BitDown(uint32_t *ptr)
{
	uint32_t Y = ptr[0];
	uint32_t X = ptr[1];

	for(uint32_t i = 0x11; i >= 0x02; i--)
	{
		uint32_t Z = (keyBuf[i] ^ X);
		X = keyBuf[DWNUM(0x048 + (((Z >> 24) & 0xFF) << 2))];
		X = (keyBuf[DWNUM(0x448 + (((Z >> 16) & 0xFF) << 2))] + X);
		X = (keyBuf[DWNUM(0x848 + (((Z >> 8)  & 0xFF) << 2))] ^ X);
		X = (keyBuf[DWNUM(0xC48 + ((Z & 0xFF) << 2))] + X);
		X = (Y ^ X);
		Y = Z;
	}

	ptr[0] = (X ^ keyBuf[DWNUM(0x04)]);
	ptr[1] = (Y ^ keyBuf[DWNUM(0x00)]);
}

//#define bswap32(val) (((val & 0x000000FF) << 24) | ((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >> 8) | ((val & 0xFF000000) >> 24))
static inline uint32_t bswap32(uint32_t val) { return ((val & 0x000000FF) << 24) | ((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >> 8) | ((val & 0xFF000000) >> 24); }
void CFIRMWARE::applyKeycode(uint32_t modulo)
{
	crypt64BitUp(&keyCode[1]);
	crypt64BitUp(&keyCode[0]);

	uint32_t scratch[2] = {0x00000000, 0x00000000};

	for(uint32_t i = 0; i <= 0x44; i += 4)
	{
		keyBuf[DWNUM(i)] = (keyBuf[DWNUM(i)] ^ bswap32(keyCode[DWNUM(i % modulo)]));
	}

	for(uint32_t i = 0; i <= 0x1040; i += 8)
	{
		crypt64BitUp(scratch);
		keyBuf[DWNUM(i)] = scratch[1];
		keyBuf[DWNUM(i+4)] = scratch[0];
	}
}
//#undef bswap32

bool CFIRMWARE::initKeycode(uint32_t idCode, int level, uint32_t modulo)
{
	if(getKeyBuf() == false)
		return false;

	keyCode[0] = idCode;
	keyCode[1] = (idCode >> 1);
	keyCode[2] = (idCode << 1);

	if(level >= 1) applyKeycode(modulo);
	if(level >= 2) applyKeycode(modulo);

	keyCode[1] <<= 1;
	keyCode[2] >>= 1;

	if(level >= 3) applyKeycode(modulo);

	return true;
}

uint16_t CFIRMWARE::getBootCodeCRC16()
{
	unsigned int i, j;
	uint32_t crc = 0xFFFF;
	const uint16_t val[8] = {0xC0C1, 0xC181, 0xC301, 0xC601, 0xCC01, 0xD801, 0xF001, 0xA001};

	for(i = 0; i < size9; i++)
	{
		crc = (crc ^ tmp_data9[i]);

		for(j = 0; j < 8; j++)
		{
			if(crc & 0x0001)
				crc = ((crc >> 1) ^ (val[j] << (7-j)));
			else
				crc =  (crc >> 1);
		}
	}

	for(i = 0; i < size7; i++)
	{
		crc = (crc ^ tmp_data7[i]);

		for(j = 0; j < 8; j++)
		{
			if(crc & 0x0001)
				crc = ((crc >> 1) ^ (val[j] << (7-j)));
			else
				crc =  (crc >> 1);
		}
	}

	return crc & 0xFFFF;
}

uint32_t CFIRMWARE::decrypt(const uint8_t *in, uint8_t* &out)
{
	uint32_t curBlock[2] = { 0 };
	uint32_t blockSize = 0;
	uint32_t xLen = 0;

	uint32_t i = 0, j = 0;
	uint32_t xIn = 4, xOut = 0;
	uint32_t len = 0;
	uint32_t offset = 0;
	uint32_t windowOffset = 0;
	uint8_t d = 0;

	uint16_t data = 0;

	memcpy(curBlock, in, 8);
	crypt64BitDown(curBlock);
	blockSize = (curBlock[0] >> 8);

	if (blockSize == 0) return 0;

	out = new uint8_t [blockSize];
	if (!out ) return 0;
	memset(out, 0xFF, blockSize);

	xLen = blockSize;
	while(xLen > 0)
	{
		d = T1ReadByte((uint8_t*)curBlock, (xIn % 8));
		xIn++;
		if((xIn % 8) == 0)
		{
			memcpy(curBlock, in + xIn, 8);
			crypt64BitDown(curBlock);
		}

		for(i = 0; i < 8; i++)
		{
			if(d & 0x80)
			{
				data = (T1ReadByte((uint8_t*)curBlock, (xIn % 8)) << 8);
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
					crypt64BitDown(curBlock);
				}
				data |= T1ReadByte((uint8_t*)curBlock, (xIn % 8));
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
					crypt64BitDown(curBlock);
				}

				len = (data >> 12) + 3;
				offset = (data & 0xFFF);
				windowOffset = (xOut - offset - 1);

				for(j = 0; j < len; j++)
				{
					T1WriteByte(out, xOut, T1ReadByte(out, windowOffset));
					xOut++;
					windowOffset++;

					xLen--;
					if(xLen == 0) return blockSize;
				}
			}
			else
			{
				T1WriteByte(out, xOut, T1ReadByte((uint8_t*)curBlock, (xIn % 8)));
				xOut++;
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
					crypt64BitDown(curBlock);
				}

				xLen--;
				if(xLen == 0) return blockSize;
			}

			d = ((d << 1) & 0xFF);
		}
	}

	return blockSize;
}

uint32_t CFIRMWARE::decompress(const uint8_t *in, uint8_t* &out)
{
	uint32_t curBlock[2] = { 0 };
	uint32_t blockSize = 0;
	uint32_t xLen = 0;

	uint32_t i = 0, j = 0;
	uint32_t xIn = 4, xOut = 0;
	uint32_t len = 0;
	uint32_t offset = 0;
	uint32_t windowOffset = 0;
	uint8_t d = 0;
	uint16_t data = 0;

	memcpy(curBlock, in, 8);
	blockSize = (curBlock[0] >> 8);

	if (blockSize == 0) return 0;

	out = new uint8_t [blockSize];
	if (!out ) return 0;
	memset(out, 0xFF, blockSize);

	xLen = blockSize;
	while(xLen > 0)
	{
		d = T1ReadByte((uint8_t*)curBlock, (xIn % 8));
		xIn++;
		if((xIn % 8) == 0)
		{
			memcpy(curBlock, in + xIn, 8);
		}

		for(i = 0; i < 8; i++)
		{
			if(d & 0x80)
			{
				data = (T1ReadByte((uint8_t*)curBlock, (xIn % 8)) << 8);
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
				}
				data |= T1ReadByte((uint8_t*)curBlock, (xIn % 8));
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
				}

				len = (data >> 12) + 3;
				offset = (data & 0xFFF);
				windowOffset = (xOut - offset - 1);

				for(j = 0; j < len; j++)
				{
					T1WriteByte(out, xOut, T1ReadByte(out, windowOffset));
					xOut++;
					windowOffset++;

					xLen--;
					if(xLen == 0) return blockSize;
				}
			}
			else
			{
				T1WriteByte(out, xOut, T1ReadByte((uint8_t*)curBlock, (xIn % 8)));
				xOut++;
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
				}

				xLen--;
				if(xLen == 0) return blockSize;
			}

			d = ((d << 1) & 0xFF);
		}
	}

	return blockSize;
}
//================================================================================
bool CFIRMWARE::load()
{
	uint32_t size = 0;
	uint8_t	*data = NULL;
	uint16_t shift1 = 0, shift2 = 0, shift3 = 0, shift4 = 0;
	uint32_t part1addr = 0, part2addr = 0/*, part3addr = 0, part4addr = 0, part5addr = 0*/;
	uint32_t part1ram = 0, part2ram = 0;

	uint32_t	src = 0;

	if (CommonSettings.UseExtFirmware == false)
		return false;
	if (strlen(CommonSettings.Firmware) == 0)
		return false;

	FILE	*fp = fopen(CommonSettings.Firmware, "rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if( (size != 256*1024) && (size != 512*1024) )
	{
		fclose(fp);
		return false;
	}

#if 1
	if (size == 512*1024)
	{
		//INFO("ERROR: 32Mbit (512Kb) firmware not supported\n");
		fclose(fp);
		return false;
	}
#endif

	data = new uint8_t [size];
	if (!data)
	{
		fclose(fp);
		return false;
	}

	if (fread(data, 1, size, fp) != size)
	{
		delete [] data;
		fclose(fp);
		return false;
	}

	memcpy(&header, data, sizeof(header));
	if ((header.fw_identifier[0] != 'M') ||
			(header.fw_identifier[1] != 'A') ||
				(header.fw_identifier[2] != 'C'))
				{
					delete [] data;
					fclose(fp);
					return false;
				}

	shift1 = ((header.shift_amounts >> 0) & 0x07);
	shift2 = ((header.shift_amounts >> 3) & 0x07);
	shift3 = ((header.shift_amounts >> 6) & 0x07);
	shift4 = ((header.shift_amounts >> 9) & 0x07);

	// todo - add support for 512Kb
	part1addr = (header.part1_rom_boot9_addr << (2 + shift1));
	part1ram = (0x02800000 - (header.part1_ram_boot9_addr << (2+shift2)));
	part2addr = (header.part2_rom_boot7_addr << (2+shift3));
	part2ram = (0x03810000 - (header.part2_ram_boot7_addr << (2+shift4)));
	//part3addr = (header.part3_rom_gui9_addr << 3);
	//part4addr = (header.part4_rom_wifi7_addr << 3);
	//part5addr = (header.part5_data_gfx_addr << 3);

	ARM9bootAddr = part1ram;
	ARM7bootAddr = part2ram;

	if(initKeycode(T1ReadLong(data, 0x08), 1, 0xC) == false)
	{
		delete [] data;
		fclose(fp);
		return false;
	}

#if 0
	crypt64BitDown((uint32_t*)&data[0x18]);
#else
	// fix touch coords
	data[0x18] = 0x00;
	data[0x19] = 0x00;
	data[0x1A] = 0x00;
	data[0x1B] = 0x00;

	data[0x1C] = 0x00;
	data[0x1D] = 0xFF;
	data[0x1E] = 0x00;
	data[0x1F] = 0x00;
#endif

	if(initKeycode(T1ReadLong(data, 0x08), 2, 0xC) == false)
	{
		delete [] data;
		fclose(fp);
		return false;
	}

	size9 = decrypt(data + part1addr, tmp_data9);
	if (!tmp_data9)
	{
		delete [] data;
		fclose(fp);
		return false;
	}

	size7 = decrypt(data + part2addr, tmp_data7);
	if (!tmp_data7)
	{
		delete [] tmp_data9;
		delete [] data;
		fclose(fp);
		return false;
	}

	uint16_t crc16_mine = getBootCodeCRC16();

	if (crc16_mine != header.part12_boot_crc16)
	{
		//INFO("Firmware: ERROR: the boot code CRC16 (0x%04X) doesn't match the value in the firmware header (0x%04X)", crc16_mine, header.part12_boot_crc16);
		delete [] tmp_data9;
		delete [] tmp_data7;
		delete [] data;
		fclose(fp);
		return false;
	}

	// Copy firmware boot codes to their respective locations
	src = 0;
	for(uint32_t i = 0; i < (size9 >> 2); i++)
	{
		_MMU_write32<ARMCPU_ARM9>(part1ram, T1ReadLong(tmp_data9, src));
		src += 4; part1ram += 4;
	}

	src = 0;
	for(uint32_t i = 0; i < (size7 >> 2); i++)
	{
		_MMU_write32<ARMCPU_ARM7>(part2ram, T1ReadLong(tmp_data7, src));
		src += 4; part2ram += 4;
	}
	delete [] tmp_data7;
	delete [] tmp_data9;

	patched = false;
	if (data[0x17C] != 0xFF)
		patched = true;

	//INFO("Firmware:\n");
	//INFO("- path: %s\n", CommonSettings.Firmware);
	//INFO("- size: %i bytes (%i Mbit)\n", size, size/1024/8);
	//INFO("- CRC : 0x%04X\n", header.part12_boot_crc16);
	//INFO("- header: \n");
	//INFO("   * size firmware %i\n", ((header.shift_amounts >> 12) & 0xF) * 128 * 1024);
	//INFO("   * ARM9 boot code address:     0x%08X\n", part1addr);
	//INFO("   * ARM9 boot code RAM address: 0x%08X\n", ARM9bootAddr);
	//INFO("   * ARM9 unpacked size:         0x%08X (%i) bytes\n", size9, size9);
	//INFO("   * ARM9 GUI code address:      0x%08X\n", part3addr);
	//INFO("\n");
	//INFO("   * ARM7 boot code address:     0x%08X\n", part2addr);
	//INFO("   * ARM7 boot code RAM address: 0x%08X\n", ARM7bootAddr);
	//INFO("   * ARM7 WiFi code address:     0x%08X\n", part4addr);
	//INFO("   * ARM7 unpacked size:         0x%08X (%i) bytes\n", size7, size7);
	//INFO("\n");
	//INFO("   * Data/GFX address:           0x%08X\n", part5addr);

	if (patched)
	{
		uint32_t patch_offset = 0x3FC80;
		if (data[0x17C] > 1)
			patch_offset = 0x3F680;

		memcpy(&header, data + patch_offset, sizeof(header));

		shift1 = ((header.shift_amounts >> 0) & 0x07);
		shift2 = ((header.shift_amounts >> 3) & 0x07);
		shift3 = ((header.shift_amounts >> 6) & 0x07);
		shift4 = ((header.shift_amounts >> 9) & 0x07);

		// todo - add support for 512Kb
		part1addr = (header.part1_rom_boot9_addr << (2 + shift1));
		part1ram = (0x02800000 - (header.part1_ram_boot9_addr << (2+shift2)));
		part2addr = (header.part2_rom_boot7_addr << (2+shift3));
		part2ram = (0x03810000 - (header.part2_ram_boot7_addr << (2+shift4)));

		ARM9bootAddr = part1ram;
		ARM7bootAddr = part2ram;

		size9 = decompress(data + part1addr, tmp_data9);
		if (!tmp_data9)
		{
			delete [] data;
			fclose(fp);
			return false;
		}

		size7 = decompress(data + part2addr, tmp_data7);
		if (!tmp_data7)
		{
			delete [] tmp_data9;
			delete [] data;
			fclose(fp);
			return false;
		};
		// Copy firmware boot codes to their respective locations
		src = 0;
		for(uint32_t i = 0; i < (size9 >> 2); i++)
		{
			_MMU_write32<ARMCPU_ARM9>(part1ram, T1ReadLong(tmp_data9, src));
			src += 4; part1ram += 4;
		}

		src = 0;
		for(uint32_t i = 0; i < (size7 >> 2); i++)
		{
			_MMU_write32<ARMCPU_ARM7>(part2ram, T1ReadLong(tmp_data7, src));
			src += 4; part2ram += 4;
		}
		delete [] tmp_data7;
		delete [] tmp_data9;

		//INFO("\nFlashme:\n");
		//INFO("- header: \n");
		//INFO("   * ARM9 boot code address:     0x%08X\n", part1addr);
		//INFO("   * ARM9 boot code RAM address: 0x%08X\n", ARM9bootAddr);
		//INFO("   * ARM9 unpacked size:         0x%08X (%i) bytes\n", size9, size9);
		//INFO("\n");
		//INFO("   * ARM7 boot code address:     0x%08X\n", part2addr);
		//INFO("   * ARM7 boot code RAM address: 0x%08X\n", ARM7bootAddr);
		//INFO("   * ARM7 unpacked size:         0x%08X (%i) bytes\n", size7, size7);
	}

	// Generate the path for the external firmware config file.
	std::string extFilePath = CFIRMWARE::GetExternalFilePath();
	strncpy(MMU.fw.userfile, extFilePath.c_str(), MAX_PATH);

	fclose(fp);
	fp = fopen(MMU.fw.userfile, "rb");
	if (fp)
	{
		char buf[0x300];
		memset(buf, 0, 0x300);
		if (fread(buf, 1, 0x100, fp) == 0x100)
		{
			printf("- loaded from %s:\n", MMU.fw.userfile);
			memcpy(&data[0x3FE00], &buf[0], 0x100);
			memcpy(&data[0x3FF00], &buf[0], 0x100);
			printf("   * User settings\n");
			memset(buf, 0, 0x100);
			if (fread(buf, 1, 0x1D6, fp) == 0x1D6)
			{
				memcpy(&data[0x002A], &buf[0], 0x1D6);
				printf("   * WiFi settings\n");

				memset(buf, 0, 0x1D6);
				if (fread(buf, 1, 0x300, fp) == 0x300)
				{
					memcpy(&data[0x3FA00], &buf[0], 0x300);
					printf("   * WiFi AP settings\n");
				}
			}

		}
		fclose(fp);
	}
	printf("\n");

	// TODO: add 512Kb support
	memcpy(&MMU.fw.data[0], data, 256*1024);
	MMU.fw.fp = NULL;

	delete [] data; data = NULL;
	return true;
}

std::string CFIRMWARE::GetExternalFilePath()
{
	std::string fwPath = CommonSettings.Firmware;
	std::string fwFileName = Path::GetFileNameFromPathWithoutExt(fwPath);
	std::string configPath = path.pathToBattery;
	std::string finalPath = configPath + DIRECTORY_DELIMITER_CHAR + fwFileName + FILE_EXT_DELIMITER_CHAR + FW_CONFIG_FILE_EXT;

	return finalPath;
}

//=====================================================================================================
static uint32_t
calc_CRC16( uint32_t start, const uint8_t *data, int count) {
	int i,j;
	uint32_t crc = start & 0xffff;
	const uint16_t val[8] = { 0xC0C1,0xC181,0xC301,0xC601,0xCC01,0xD801,0xF001,0xA001 };
	for(i = 0; i < count; i++)
	{
		crc = crc ^ data[i];

		for(j = 0; j < 8; j++) {
			int do_bit = 0;

			if ( crc & 0x1)
				do_bit = 1;

			crc = crc >> 1;

			if ( do_bit) {
				crc = crc ^ (val[j] << (7-j));
			}
		}
	}
	return crc;
}

int copy_firmware_user_data( uint8_t *dest_buffer, const uint8_t *fw_data)
{
	/*
	* Determine which of the two user settings in the firmware is the current
	* and valid one and then copy this into the destination buffer.
	*
	* The current setting will have a greater count.
	* Settings are only valid if its CRC16 is correct.
	*/
	int user1_valid = 0;
	int user2_valid = 0;
	uint32_t user_settings_offset;
	uint32_t fw_crc;
	uint32_t crc;
	int copy_good = 0;

	user_settings_offset = fw_data[0x20];
	user_settings_offset |= fw_data[0x21] << 8;
	user_settings_offset <<= 3;

	if ( user_settings_offset <= 0x3FE00) {
		int32_t copy_settings_offset = -1;

		crc = calc_CRC16( 0xffff, &fw_data[user_settings_offset],
			NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT);
		fw_crc = fw_data[user_settings_offset + 0x72];
		fw_crc |= fw_data[user_settings_offset + 0x73] << 8;
		if ( crc == fw_crc) {
			user1_valid = 1;
		}

		crc = calc_CRC16( 0xffff, &fw_data[user_settings_offset + 0x100],
			NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT);
		fw_crc = fw_data[user_settings_offset + 0x100 + 0x72];
		fw_crc |= fw_data[user_settings_offset + 0x100 + 0x73] << 8;
		if ( crc == fw_crc) {
			user2_valid = 1;
		}

		if ( user1_valid) {
			if ( user2_valid) {
				uint16_t count1, count2;

				count1 = fw_data[user_settings_offset + 0x70];
				count1 |= fw_data[user_settings_offset + 0x71] << 8;

				count2 = fw_data[user_settings_offset + 0x100 + 0x70];
				count2 |= fw_data[user_settings_offset + 0x100 + 0x71] << 8;

				if ( count2 > count1) {
					copy_settings_offset = user_settings_offset + 0x100;
				}
				else {
					copy_settings_offset = user_settings_offset;
				}
			}
			else {
				copy_settings_offset = user_settings_offset;
			}
		}
		else if ( user2_valid) {
			/* copy the second user settings */
			copy_settings_offset = user_settings_offset + 0x100;
		}

		if ( copy_settings_offset > 0) {
			memcpy( dest_buffer, &fw_data[copy_settings_offset],
				NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT);
			copy_good = 1;
		}
	}

	return copy_good;
}

/*static void fill_user_data_area( struct NDS_fw_config_data *user_settings,uint8_t *data, int count)
{
	uint32_t crc;
	int i;
	//uint8_t *ts_cal_data_area;

	memset( data, 0, 0x100);

	// version
	data[0x00] = 5;
	data[0x01] = 0;

	// colour
	data[0x02] = user_settings->fav_colour;

	// birthday month and day
	data[0x03] = user_settings->birth_month;
	data[0x04] = user_settings->birth_day;

	//nickname and length
	for ( i = 0; i < MAX_FW_NICKNAME_LENGTH; i++) {
		data[0x06 + (i * 2)] = user_settings->nickname[i] & 0xff;
		data[0x06 + (i * 2) + 1] = (user_settings->nickname[i] >> 8) & 0xff;
	}

	data[0x1a] = user_settings->nickname_len;

	//Message
	for ( i = 0; i < MAX_FW_MESSAGE_LENGTH; i++) {
		data[0x1c + (i * 2)] = user_settings->message[i] & 0xff;
		data[0x1c + (i * 2) + 1] = (user_settings->message[i] >> 8) & 0xff;
	}

	data[0x50] = user_settings->message_len;*/

	//touch screen calibration
	/*ts_cal_data_area = &data[0x58];
	for ( i = 0; i < 2; i++) {
		// ADC x y
		*ts_cal_data_area++ = user_settings->touch_cal[i].adc_x & 0xff;
		*ts_cal_data_area++ = (user_settings->touch_cal[i].adc_x >> 8) & 0xff;
		*ts_cal_data_area++ = user_settings->touch_cal[i].adc_y & 0xff;
		*ts_cal_data_area++ = (user_settings->touch_cal[i].adc_y >> 8) & 0xff;

		//screen x y
		*ts_cal_data_area++ = user_settings->touch_cal[i].screen_x;
		*ts_cal_data_area++ = user_settings->touch_cal[i].screen_y;
	}*/

	//language and flags
	/*data[0x64] = user_settings->language;
	data[0x65] = 0xfc;

	//update count and crc
	data[0x70] = count & 0xff;
	data[0x71] = (count >> 8) & 0xff;

	crc = calc_CRC16( 0xffff, data, 0x70);
	data[0x72] = crc & 0xff;
	data[0x73] = (crc >> 8) & 0xff;

	memset( &data[0x74], 0xff, 0x100 - 0x74);
}*/

// creates an firmware flash image, which contains all needed info to initiate a wifi connection
/*int NDS_CreateDummyFirmware( struct NDS_fw_config_data *user_settings)
{
	//Create the firmware header

	memset(&MMU.fw.data[0], 0, 0x40000);

	//firmware identifier
	MMU.fw.data[0x8] = 'M';
	MMU.fw.data[0x8 + 1] = 'A';
	MMU.fw.data[0x8 + 2] = 'C';
	MMU.fw.data[0x8 + 3] = 'P';

	// DS type
	if ( user_settings->ds_type == NDS_FW_DS_TYPE_LITE)
		MMU.fw.data[0x1d] = 0x20;
	else
		MMU.fw.data[0x1d] = 0xff;

	//User Settings offset 0x3fe00 / 8
	MMU.fw.data[0x20] = 0xc0;
	MMU.fw.data[0x21] = 0x7f;


	//User settings (at 0x3FE00 and 0x3FF00)

	fill_user_data_area( user_settings, &MMU.fw.data[ 0x3FE00], 0);
	fill_user_data_area( user_settings, &MMU.fw.data[ 0x3FF00], 1);

	// Wifi config length
	//MMU.fw.data[0x2C] = 0x38;
	//MMU.fw.data[0x2D] = 0x01;

	//MMU.fw.data[0x2E] = 0x00;

	//Wifi version
	//MMU.fw.data[0x2F] = 0x00;

	//MAC address
	//memcpy((MMU.fw.data + 0x36), FW_Mac, sizeof(FW_Mac));

	//Enabled channels
	MMU.fw.data[0x3C] = 0xFE;
	MMU.fw.data[0x3D] = 0x3F;

	MMU.fw.data[0x3E] = 0xFF;
	MMU.fw.data[0x3F] = 0xFF;

	//RF related
	MMU.fw.data[0x40] = 0x02;
	MMU.fw.data[0x41] = 0x18;
	MMU.fw.data[0x42] = 0x0C;

	MMU.fw.data[0x43] = 0x01;

	//Wifi I/O init values
	//memcpy((MMU.fw.data + 0x44), FW_WIFIInit, sizeof(FW_WIFIInit));

	//Wifi BB init values
	//memcpy((MMU.fw.data + 0x64), FW_BBInit, sizeof(FW_BBInit));

	//Wifi RF init values
	//memcpy((MMU.fw.data + 0xCE), FW_RFInit, sizeof(FW_RFInit));

	//Wifi channel-related init values
	//memcpy((MMU.fw.data + 0xF2), FW_RFChannel, sizeof(FW_RFChannel));
	//memcpy((MMU.fw.data + 0x146), FW_BBChannel, sizeof(FW_BBChannel));
	memset(&MMU.fw.data[0x154], 0x10, 0xE);*/

	//WFC profiles
	/*memcpy((MMU.fw.data + 0x3FA40), &FW_WFCProfile1, sizeof(FW_WFCProfile));
	memcpy((MMU.fw.data + 0x3FB40), &FW_WFCProfile2, sizeof(FW_WFCProfile));
	memcpy((MMU.fw.data + 0x3FC40), &FW_WFCProfile3, sizeof(FW_WFCProfile));
	(*(uint16_t*)(MMU.fw.data + 0x3FAFE)) = (uint16_t)calc_CRC16(0, (MMU.fw.data + 0x3FA00), 0xFE);
	(*(uint16_t*)(MMU.fw.data + 0x3FBFE)) = (uint16_t)calc_CRC16(0, (MMU.fw.data + 0x3FB00), 0xFE);
	(*(uint16_t*)(MMU.fw.data + 0x3FCFE)) = (uint16_t)calc_CRC16(0, (MMU.fw.data + 0x3FC00), 0xFE);*/


	/*MMU.fw.data[0x162] = 0x19;
	memset(&MMU.fw.data[0x163], 0xFF, 0x9D);

	//Wifi settings CRC16
	(*(uint16_t*)(&MMU.fw.data[0x2A])) = calc_CRC16(0, &MMU.fw.data[0x2C], 0x138);

	if (&CommonSettings.InternalFirmConf != user_settings)
		memcpy(&CommonSettings.InternalFirmConf, user_settings, sizeof(struct NDS_fw_config_data));

	return true ;
}*/

void NDS_FillDefaultFirmwareConfigData( struct NDS_fw_config_data *fw_config) {
	const char *default_nickname = "DeSmuME";
	const char *default_message = "DeSmuME makes you happy!";
	int i;
	int str_length;

	memset( fw_config, 0, sizeof( struct NDS_fw_config_data));
	fw_config->ds_type = NDS_CONSOLE_TYPE_FAT;

	fw_config->fav_colour = 7;

	fw_config->birth_day = 23;
	fw_config->birth_month = 6;

	str_length = strlen( default_nickname);
	for ( i = 0; i < str_length; i++) {
		fw_config->nickname[i] = default_nickname[i];
	}
	fw_config->nickname_len = str_length;

	str_length = strlen( default_message);
	for ( i = 0; i < str_length; i++) {
		fw_config->message[i] = default_message[i];
	}
	fw_config->message_len = str_length;

	/* default to English */
	fw_config->language = 1;

	/* default touchscreen calibration */
	//fw_config->touch_cal[0].adc_x = 0x200;
	//fw_config->touch_cal[0].adc_y = 0x200;
	//fw_config->touch_cal[0].screen_x = 0x20 + 1; // calibration screen coords are 1-based,
	//fw_config->touch_cal[0].screen_y = 0x20 + 1; // either that or NDS_getADCTouchPosX/Y are wrong.

	//fw_config->touch_cal[1].adc_x = 0xe00;
	//fw_config->touch_cal[1].adc_y = 0x800;
	//fw_config->touch_cal[1].screen_x = 0xe0 + 1;
	//fw_config->touch_cal[1].screen_y = 0x80 + 1;
}

/*void NDS_PatchFirmwareMAC()
{
	//memcpy((MMU.fw.data + 0x36), FW_Mac, sizeof(FW_Mac));
	//(*(uint16_t*)(MMU.fw.data + 0x2A)) = calc_CRC16(0, (MMU.fw.data + 0x2C), 0x138);
}*/
