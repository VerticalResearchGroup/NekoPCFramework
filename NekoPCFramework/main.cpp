/*
* Copyright (c) 2015, Ziliang Guo
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the distribution.
* * Neither the name of Wisconsin Robotics nor the
*   names of its contributors may be used to endorse or promote products
*   derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL WISCONSIN ROBOTICS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <wincodec.h>
#define COM_PORT L"COM3"

#define LD_INSTR_STATE 1

#define RESET_INSTR_CNT 2
#define EXEC_KERNEL_STATE 3
#define CHECK_EXEC_STATUS 4
#define READ_PC_CNT 5

#define LD_MEM_DATA 6
#define RD_MEM_DATA 7
#define RESET_MEM_CNT 8
#define MEM_CNT_INC 9

#define MEM_WR_ADDR_OUT 20
#define KERNEL_DONE_OUT 50

HANDLE serialPort;

static char receiveBuffer[MAX_PATH + 1];

extern uint32_t instr_mem[];
extern uint32_t LENTH_OF_INSTR_MEM;
extern uint32_t data_mem[];
extern uint32_t LENTH_OF_DATA_MEM;

BYTE *imageBuffer = nullptr;

char cmdBuffer;
char sendBuffer[4];

int main()
{
    DWORD bytesRead;
    DWORD bytesWritten;
    DCB dcb;
    COMMTIMEOUTS commTimeouts;
    BOOL status;
    uint32_t index;
    uint32_t returnedData;

    serialPort = CreateFile(
        COM_PORT,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        0,
        NULL
        );

    if (serialPort == INVALID_HANDLE_VALUE)
        return -1;

    GetCommTimeouts(serialPort, &commTimeouts);
    commTimeouts.ReadIntervalTimeout = 0;
    SetCommTimeouts(serialPort, &commTimeouts);

    dcb.DCBlength = sizeof(DCB);
    dcb.BaudRate = CBR_115200;
    dcb.fBinary = 1;
    dcb.fParity = 0;
    dcb.fOutxCtsFlow = 0;
    dcb.fOutxDsrFlow = 0;
    dcb.fDtrControl = 0;
    dcb.fDsrSensitivity = 0;
    dcb.fTXContinueOnXoff = 0;
    dcb.fOutX = 0;
    dcb.fInX = 0;
    dcb.fErrorChar = 0;
    dcb.fNull = 0;
    dcb.fRtsControl = 0;
    dcb.fAbortOnError = 0;
    dcb.fDummy2 = 0;
    dcb.wReserved = 0;
    dcb.wReserved1 = 0;
    dcb.XonLim = 32768;
    dcb.XoffLim = 8192;
    dcb.ByteSize = 8;
    dcb.Parity = 0;
    dcb.StopBits = 0;
    dcb.XonChar = '\0';
    dcb.XoffChar = '\0';
    dcb.ErrorChar = '\0';
    dcb.EofChar = '\x4';
    dcb.EvtChar = '\n';

    if (!SetCommState(serialPort, &dcb))
        goto Cleanup;

    cmdBuffer = RESET_INSTR_CNT;
    status = WriteFile(serialPort, &cmdBuffer, 1, &bytesWritten, nullptr);

    for (index = 0; index < LENTH_OF_INSTR_MEM; ++index)
    {
        cmdBuffer = LD_INSTR_STATE;
        sendBuffer[0] = (instr_mem[index] >> 24) & 0xFF;
        sendBuffer[1] = (instr_mem[index] >> 16) & 0xFF;
        sendBuffer[2] = (instr_mem[index] >> 8) & 0xFF;
        sendBuffer[3] = instr_mem[index] & 0xFF;
        status = WriteFile(serialPort, &cmdBuffer, 1, &bytesWritten, nullptr);
        status = WriteFile(serialPort, (LPCVOID)sendBuffer, 4, &bytesWritten, nullptr);
        status = ReadFile(serialPort, receiveBuffer, 4, &bytesRead, NULL);
        if (!bytesRead)
        {
            wprintf_s(L"Problem\n");
        }
        else
        {
            returnedData = ((uint32_t)receiveBuffer[3] & 0xFF) + (((uint32_t)receiveBuffer[2] << 8) & 0xFF00) + (((uint32_t)receiveBuffer[1] << 16) & 0xFF0000) + (((uint32_t)receiveBuffer[0] << 24) & 0xFF000000);
            //wprintf_s(L"Written instruction: %x Returned data: %x\n", instr_mem[index], returnedData);
            if (returnedData != instr_mem[index])
            {
                wprintf_s(L"Problem with instruction write #%u\n", index);
                goto Cleanup;
            }
        }
    }
    
    cmdBuffer = RESET_MEM_CNT;
    status = WriteFile(serialPort, &cmdBuffer, 1, &bytesWritten, nullptr);

    for (index = 0; index < LENTH_OF_DATA_MEM; ++index)
    {
        if (!data_mem[index])
        {
            cmdBuffer = MEM_CNT_INC;
            status = WriteFile(serialPort, &cmdBuffer, 1, &bytesWritten, nullptr);
            continue;
        }

        cmdBuffer = LD_MEM_DATA;
        sendBuffer[0] = (data_mem[index] >> 24) & 0xFF;
        sendBuffer[1] = (data_mem[index] >> 16) & 0xFF;
        sendBuffer[2] = (data_mem[index] >> 8) & 0xFF;
        sendBuffer[3] = data_mem[index] & 0xFF;
        status = WriteFile(serialPort, &cmdBuffer, 1, &bytesWritten, nullptr);
        status = WriteFile(serialPort, (LPCVOID)sendBuffer, 4, &bytesWritten, nullptr);
        status = ReadFile(serialPort, receiveBuffer, 4, &bytesRead, NULL);
        if (!bytesRead)
        {
            wprintf_s(L"Problem\n");
        }
        else
        {
            returnedData = ((uint32_t)receiveBuffer[3] & 0xFF) + (((uint32_t)receiveBuffer[2] << 8) & 0xFF00) + (((uint32_t)receiveBuffer[1] << 16) & 0xFF0000) + (((uint32_t)receiveBuffer[0] << 24) & 0xFF000000);
            //wprintf_s(L"Written instruction: %x Returned data: %x\n", data_mem[index], returnedData);
            if (returnedData != data_mem[index])
            {
                wprintf_s(L"Problem with instruction write #%u\n", index);
                goto Cleanup;
            }
        }
    }
    
    cmdBuffer = EXEC_KERNEL_STATE;
    status = WriteFile(serialPort, &cmdBuffer, 1, &bytesWritten, nullptr);

    while (true)
    {
        status = ReadFile(serialPort, receiveBuffer, 1, &bytesRead, NULL);
        if (receiveBuffer[0] == MEM_WR_ADDR_OUT)
        {
            status = ReadFile(serialPort, receiveBuffer, 4, &bytesRead, NULL);
            returnedData = ((uint32_t)receiveBuffer[3] & 0xFF) + (((uint32_t)receiveBuffer[2] << 8) & 0xFF00) + (((uint32_t)receiveBuffer[1] << 16) & 0xFF0000) + (((uint32_t)receiveBuffer[0] << 24) & 0xFF000000);
            wprintf_s(L"Write address: 0x%x\n", returnedData);
            continue;
        }
        if (receiveBuffer[0] == KERNEL_DONE_OUT)
            break;
    }

    cmdBuffer = READ_PC_CNT;
    status = WriteFile(serialPort, &cmdBuffer, 1, &bytesWritten, nullptr);
    status = ReadFile(serialPort, receiveBuffer, 4, &bytesRead, NULL);
    returnedData = ((uint32_t)receiveBuffer[3] & 0xFF) + (((uint32_t)receiveBuffer[2] << 8) & 0xFF00) + (((uint32_t)receiveBuffer[1] << 16) & 0xFF0000) + (((uint32_t)receiveBuffer[0] << 24) & 0xFF000000);
    wprintf_s(L"PC count: %d", returnedData);
Cleanup:
    CloseHandle(serialPort);

    return 0;
}