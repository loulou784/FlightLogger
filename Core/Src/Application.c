/*
 * Application.c
 *
 *  Created on: May 23, 2021
 *      Author: alexmorin
 */

#include "Application.h"

bool bSDPresent = false;
bool bFileIsOpen = false;

FATFS FatFs;
FIL fp;
FRESULT fResult;
FILINFO fInfo;
DIR dir;

SD_MPU6050 mpu6050;
SD_MPU6050_Result mpuResult;

uint8_t u8LogFolderIndex = 0;
uint8_t u8LogFileIndex = 0;
uint32_t u32LogFileLineCounter = 0;
uint8_t u8Buffer[128];
int iBufferlen = 0;
uint8_t u8FolderPath[10];
int iFolderPathlen = 0;
uint8_t u8BytesWritten = 0;

void unmountSD() {
	bSDPresent = false;
	f_mount(NULL, "", 0);
}

void ApplicationInit() {
	// Mount SD Card
	fResult = f_mount(&FatFs, "", 1);
	if(fResult == FR_OK) {
		bSDPresent = true;
	} else {
		// Error with SD card, cannot continue
		unmountSD();
		return;
	}

	// Check if FlightLog folder exists, otherwise create it
	//fResult = f_opendir(&dir, "/log");
	fResult = f_stat("/log", &fInfo);
	if(fResult == FR_NO_FILE) {
		// Create the directory
		fResult = f_mkdir("/log");
		if(fResult != FR_OK) {
			// Error with SD card, cannot continue
			unmountSD();
			return;
		}
	}

	// Check the highest XX folder for specific flight, create one XX + 1
	while(u8LogFolderIndex < 255) {
		iFolderPathlen = snprintf(u8FolderPath, sizeof(u8FolderPath), "/log/%d", u8LogFolderIndex);
		fResult = f_stat(u8FolderPath, &fInfo);
		// The folder does not exist, we can create it
		if(fResult == FR_NO_FILE) {
			// Create the directory
			fResult = f_mkdir(u8FolderPath);
			if(fResult != FR_OK) {
				// Error with SD card, cannot continue
				unmountSD();
				return;
			} else if(fResult == FR_OK) {
				// We found our new log folder
				break;
			}
		} else {
			u8LogFolderIndex++;
		}
	}
	if(u8LogFolderIndex == 255) {
		// Needs to delete log folder before continuing
		unmountSD();
		return;
	}

	//Everything went right with the SD card, we can init the MPU6050
	mpuResult = SD_MPU6050_Init(&hi2c1, &mpu6050, SD_MPU6050_Device_0, SD_MPU6050_Accelerometer_2G, SD_MPU6050_Gyroscope_250s);
	SD_MPU6050_SetDataRate(&hi2c1, &mpu6050, SD_MPU6050_DataRate_1KHz);
	HAL_Delay(500); // Wait init
	if(mpuResult != SD_MPU6050_Result_Ok) {
		unmountSD();
		return;
	}

}

void ApplicationTask() {
	if(bSDPresent) {

		if(bFileIsOpen == false) {
			if(u8LogFileIndex == 6) {
				while(1) {
					HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
					HAL_Delay(1000);
				}
			}

			iBufferlen = snprintf(u8Buffer, sizeof(u8Buffer), "%s/%d.csv", u8FolderPath, u8LogFileIndex);
			u8LogFileIndex++;
			u32LogFileLineCounter = 0;
			fResult = f_open(&fp, u8Buffer, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			if(fResult != FR_OK) {
				unmountSD();
				return;
			}
			bFileIsOpen = true;
		} else {

			mpuResult = SD_MPU6050_ReadAll(&hi2c1, &mpu6050);
			if(mpuResult == SD_MPU6050_Result_Ok) {
				iBufferlen = snprintf(u8Buffer, sizeof(u8Buffer), "%d,%d,%d,%d,%d,%d\n",mpu6050.Accelerometer_X, mpu6050.Accelerometer_Y, mpu6050.Accelerometer_Z, mpu6050.Gyroscope_X, mpu6050.Gyroscope_Y, mpu6050.Gyroscope_Z);
				fResult = f_write(&fp, u8Buffer, iBufferlen, &u8BytesWritten);
			}

			u32LogFileLineCounter++;
			if(u32LogFileLineCounter == MAX_LINE_PER_FILE) {
				bFileIsOpen = false;
				fResult = f_close(&fp);
			}
		}

	} else {
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
		HAL_Delay(100);
	}
}
