/*
 * W25Qxx.h
 *
 *  Created on: Apr 21, 2025
 *      Author: PC-X2
 */

#ifndef INC_W25QXX_H_
#define INC_W25QXX_H_
uint32_t W25Q_ReadID(void);
void W25Q_Reset(void);
void erase_sector(uint8_t numSector);
void W25Q_Read(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
void W25Q_Write_Page(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);

#endif /* INC_W25QXX_H_ */
