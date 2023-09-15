#include "main.h"
#include "sdram_defines.h"
#include <stdbool.h>

static int SDRAM_SendCommand(uint32_t CommandMode, int32_t Bank, uint32_t RefreshNum, uint32_t RegVal,
                             SDRAM_HandleTypeDef *hsdram1) {
    uint32_t CommandTarget;
    FMC_SDRAM_CommandTypeDef Command;

    if (Bank == 1) {
        CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    } else if (Bank == 2) {
        CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    }

    Command.CommandMode = CommandMode;
    Command.CommandTarget = CommandTarget;
    Command.AutoRefreshNumber = RefreshNum;
    Command.ModeRegisterDefinition = RegVal;

    if (HAL_SDRAM_SendCommand(hsdram1, &Command, 0x1000) != HAL_OK) {
        return -1;
    }
    return 0;
}

void SDRAM_Init(SDRAM_HandleTypeDef *hsdram1) {
    uint32_t temp;

    SDRAM_SendCommand(FMC_SDRAM_CMD_CLK_ENABLE, 1, 1, 0, hsdram1);

    HAL_Delay(1);

    SDRAM_SendCommand(FMC_SDRAM_CMD_PALL, 1, 1, 0, hsdram1);
    SDRAM_SendCommand(FMC_SDRAM_CMD_AUTOREFRESH_MODE, 1, 8, 0, hsdram1);
    temp = (uint32_t) SDRAM_MODEREG_BURST_LENGTH_1 | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | SDRAM_MODEREG_CAS_LATENCY_3 |
           SDRAM_MODEREG_OPERATING_MODE_STANDARD | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
    SDRAM_SendCommand(FMC_SDRAM_CMD_LOAD_MODE, 1, 1, temp, hsdram1);

//    HAL_SDRAM_ProgramRefreshRate(hsdram1, 1668);
    HAL_SDRAM_ProgramRefreshRate(hsdram1, 1105);
}

float SDRAM_Test(long int size_of_ram_in_bytes) {
    volatile uint32_t *ram_address = (uint32_t *)0xC0000000;
    long int successes = 0;
    long int blocks_to_test = size_of_ram_in_bytes / 4;
    for (long int i = 0; i < blocks_to_test; i++) {
        ram_address[i] = 0xFFFF00FF;
    }
    for (int i = 0; i < blocks_to_test; i++) {
        if (ram_address[i] == 0xFFFF00FF) {
            successes++;
        }
//        ram_address[i] = 0x00000000;
    }
    return (double) successes / (double) blocks_to_test;
}
