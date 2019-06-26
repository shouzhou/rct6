

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_EEPROM_24XX_H
#define __BSP_EEPROM_24XX_H

/* Includes ------------------------------------------------------------------*/
#include "bsp_i2c_gpio.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
//#define   WP    P12       

 #define AT24C02 
/* #define AT24C04 */
/* #define AT24C08 */
//#define AT24C16////
/* #define AT24C32 */
/* #define AT24C64 */

#ifdef AT24C02
	#define EE_MODEL_NAME		"AT24C02"
	#define EE_DEV_ADDR			0xA0			/* �豸��ַ */
	#define EE_PAGE_SIZE		8					/* ҳ���С(�ֽ�) */
	#define EE_SIZE				256				/* ������(�ֽ�) */
	#define EE_ADDR_BYTES		1					/* ��ַ�ֽڸ��� */
	#define EE_ADDR_A8			0					/* ��ַ�ֽڵĸ�8bit�������ֽ� */
#endif

#ifdef AT24C04
	#define EE_MODEL_NAME		"AT24C04"
	#define EE_DEV_ADDR			0xA0			/* �豸��ַ */
	#define EE_PAGE_SIZE		16				/* ҳ���С(�ֽ�) */
	#define EE_SIZE				  512				/* ������(�ֽ�) */
	#define EE_ADDR_BYTES		1					/* ��ַ�ֽڸ��� */
	#define EE_ADDR_A8			1					/* ��ַ�ֽڵĸ�8bit�����ֽ� */
#endif

#ifdef AT24C08
	#define EE_MODEL_NAME		"AT24C08"
	#define EE_DEV_ADDR			0xA0	    /* �豸��ַ */
	#define EE_PAGE_SIZE		16		   	/* ҳ���С(�ֽ�) */
	#define EE_SIZE				  (1*1024)	/* ������(�ֽ�) */
	#define EE_ADDR_BYTES		1		    	/* ��ַ�ֽڸ��� */
	#define EE_ADDR_A8			1			    /* ��ַ�ֽڵĸ�8bit�����ֽ� */
#endif

#ifdef AT24C16
	#define EE_MODEL_NAME		"AT24C16"
	#define EE_DEV_ADDR			0xA0	    /* �豸��ַ */
	#define EE_PAGE_SIZE		16		   	/* ҳ���С(�ֽ�) */
	#define EE_SIZE				  (2*1024) 	/* ������(�ֽ�) */
	#define EE_ADDR_BYTES		1			    /* ��ַ�ֽڸ��� */
	#define EE_ADDR_A8			1			    /* ��ַ�ֽڵĸ�8bit�����ֽ� */
#endif

#ifdef AT24C32
	#define EE_MODEL_NAME		"AT24C32"
	#define EE_DEV_ADDR			0xA0	    /* �豸��ַ */
	#define EE_PAGE_SIZE		32		   	/* ҳ���С(�ֽ�) */
	#define EE_SIZE				  (4*1024)  /* ������(�ֽ�) */
	#define EE_ADDR_BYTES		2			    /* ��ַ�ֽڸ��� */
	#define EE_ADDR_A8			0			    /* ��ַ�ֽڵĸ�8bit�����ֽ� */
#endif

#ifdef AT24C64
	#define EE_MODEL_NAME		"AT24C64"
	#define EE_DEV_ADDR			0xA0	    /* �豸��ַ */
	#define EE_PAGE_SIZE		32			  /* ҳ���С(�ֽ�) */
	#define EE_SIZE				  (8*1024)	/* ������(�ֽ�) */
	#define EE_ADDR_BYTES		2			    /* ��ַ�ֽڸ��� */
	#define EE_ADDR_A8			0			    /* ��ַ�ֽڵĸ�8bit�����ֽ� */
#endif

#ifdef AT24C128
	#define EE_MODEL_NAME		"AT24C128"
	#define EE_DEV_ADDR			0xA0	  	/* �豸��ַ */
	#define EE_PAGE_SIZE		64			  /* ҳ���С(�ֽ�) */
	#define EE_SIZE				  (16*1024)	/* ������(�ֽ�) */
	#define EE_ADDR_BYTES		2			    /* ��ַ�ֽڸ��� */
	#define EE_ADDR_A8			0			    /* ��ַ�ֽڵĸ�8bit�������ֽ� */
#endif

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
/* EEPROM���    ���������� */
uint8_t ee_CheckOk(void);
/* EEPROM������  ���������� */
uint8_t ee_ReadBytes(uint8_t * /*_pReadBuf*/, uint16_t /*_usAddress*/, uint16_t /*_usSize*/);
/* EEPROM����д  ���������� */
uint8_t ee_WriteBytes(uint8_t /*_rease*/, uint8_t * /*_pWriteBuf*/, uint16_t /*_usAddress*/, uint16_t /*_usSize*/);

/* Internal variables --------------------------------------------------------*/

#endif /* __BSP_EEPROM_24XX_H */


