/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_QSPI_H
#define MBED_QSPI_H

#include "platform/platform.h"

#if defined (DEVICE_QSPI) || defined(DOXYGEN_ONLY)

#include "hal/qspi_api.h"
#include "platform/PlatformMutex.h"
#include "platform/SingletonPtr.h"
#include "platform/NonCopyable.h"

#define ONE_MHZ     1000000

namespace mbed {
    
/** \addtogroup drivers */

/** A QSPI Driver, used for communicating with QSPI slave devices
 *
 * The default format is set to Quad-SPI(4-4-4), and a clock frequency of 1MHz
 * Most QSPI devices will also require Chip Select which is indicated by ssel. 
 *
 * @note Synchronization level: Thread safe
 *
 * Example:
 * @code
 * // Write 4 byte array to a QSPI slave, and read the response, note that each device will have its specific read/write/alt values defined
 *
 * #include "mbed.h"
 *
 * // hardware ssel (where applicable)
 * QSPI qspi_device(p5, p6, p7, p8, p9, p10); // io0, io1, io2, io3, sclk, ssel
 *
 *
 * int main() {
 *     char tx_buf[] = { 0x11, 0x22, 0x33, 0x44 };    
 *     char rx_buf[4];    
 *     int buf_len = sizeof(tx_buf); 
 *     
 *     int result = qspi_device.write( 0x12 , 0x100000 , 0 , tx_buf, &buf_len );
 *     if( !result ) printf("Write failed");
 *     int result = qspi_device.read( 0x13 , 0x100000 , 0 , rx_buf, &buf_len );
 *     if( !result ) printf("Read failed");
 *
 * }
 * @endcode
 * @ingroup drivers
 */
class QSPI : private NonCopyable<QSPI> {

public:

    /** Create a QSPI master connected to the specified pins
     *
     *  io0-io3 is used to specify the Pins used for Quad SPI mode
     *
     *  @param io0 1st IO pin used for sending/receiving data during data phase of a transaction
     *  @param io1 2nd IO pin used for sending/receiving data during data phase of a transaction
     *  @param io2 3rd IO pin used for sending/receiving data during data phase of a transaction
     *  @param io3 4th IO pin used for sending/receiving data during data phase of a transaction
     *  @param sclk QSPI Clock pin
     *  @param ssel QSPI chip select pin
     *  @param mode Mode specifies the SPI mode(Mode=0 uses CPOL=0, CPHA=0, Mode=1 uses CPOL=1, CPHA=1)
     *         default value = 0
     *
     */
    QSPI(PinName io0, PinName io1, PinName io2, PinName io3, PinName sclk, PinName ssel=NC, int mode=0);

    /** Configure the data transmission format
     *
     *  @param inst_width Bus width used by instruction phase(Valid values are 1,2,4)
     *  @param address_width Bus width used by address phase(Valid values are 1,2,4)
     *  @param address_size Size in bits used by address phase(Valid values are 8,16,24,32)
     *  @param alt_width Bus width used by alt phase(Valid values are 1,2,4)
     *  @param alt_size Size in bits used by alt phase(Valid values are 8,16,24,32)
     *  @param data_width Bus width used by data phase(Valid values are 1,2,4)
     *  @param dummy_cycles Number of dummy clock cycles to be used after alt phase
     *
     */
    qspi_status_t configure_format(qspi_bus_width_t inst_width, 
                   qspi_bus_width_t address_width, 
                   qspi_address_size_t address_size,
                   qspi_bus_width_t alt_width, 
                   qspi_alt_size_t alt_size,   
                   qspi_bus_width_t data_width,
                   int dummy_cycles);

    /** Set the qspi bus clock frequency
     *
     *  @param hz SCLK frequency in hz (default = 1MHz)
     *  @returns
     *    Returns QSPI_STATUS_SUCCESS on successful, fails if the interface is already init-ed
     */
    qspi_status_t set_frequency(int hz = ONE_MHZ);

    /** Read from QSPI peripheral with the preset read_instruction and alt_value
     *
     *  @param address Address to be accessed in QSPI peripheral
     *  @param rx_buffer Buffer for data to be read from the peripheral                          
     *  @param rx_length Pointer to a variable containing the length of rx_buffer, and on return this variable will be updated with the actual number of bytes read
     *
     *  @returns
     *    Returns QSPI_STATUS_SUCCESS on successful reads and QSPI_STATUS_ERROR on failed reads.
     */
    qspi_status_t read(unsigned int address, char *rx_buffer, size_t *rx_length);

    /** Write to QSPI peripheral using custom write instruction
     *
     *  @param address Address to be accessed in QSPI peripheral
     *  @param tx_buffer Buffer containing data to be sent to peripheral                          
     *  @param tx_length Pointer to a variable containing the length of data to be transmitted, and on return this variable will be updated with the actual number of bytes written
     *
     *  @returns
     *    Returns QSPI_STATUS_SUCCESS on successful reads and QSPI_STATUS_ERROR on failed reads.
     */
    qspi_status_t write(unsigned int address, const char *tx_buffer, size_t *tx_length);

    /** Read from QSPI peripheral using custom read instruction, alt values
     *
     *  @param instruction Instruction value to be used in instruction phase
     *  @param alt Alt value to be used in instruction phase
     *  @param dummy_cnt Amount of dummy cycles to be sent after instruction phase
     *  @param address Address to be accessed in QSPI peripheral
     *  @param rx_buffer Buffer for data to be read from the peripheral                          
     *  @param rx_length Pointer to a variable containing the length of rx_buffer, and on return this variable will be updated with the actual number of bytes read
     *
     *  @returns
     *    Returns QSPI_STATUS_SUCCESS on successful reads and QSPI_STATUS_ERROR on failed reads.
     */
    qspi_status_t read(unsigned int instruction, unsigned int alt, unsigned int dummy_cnt, unsigned int address, char *rx_buffer, size_t *rx_length);

    /** Write to QSPI peripheral using custom write instruction, alt values
     *
     *  @param instruction Instruction value to be used in instruction phase
     *  @param alt Alt value to be used in instruction phase
     *  @param dummy_cnt Amount of dummy cycles to be sent after instruction phase
     *  @param address Address to be accessed in QSPI peripheral
     *  @param tx_buffer Buffer containing data to be sent to peripheral                          
     *  @param tx_length Pointer to a variable containing the length of data to be transmitted, and on return this variable will be updated with the actual number of bytes written
     *
     *  @returns
     *    Returns QSPI_STATUS_SUCCESS on successful reads and QSPI_STATUS_ERROR on failed reads.
     */
    qspi_status_t write(unsigned int instruction, unsigned int alt, unsigned int dummy_cnt, unsigned int address, const char *tx_buffer, size_t *tx_length);

    /** Perform a transaction to write to an address(a control register) and get the status results
     *
     *  @param instruction Instruction value to be used in instruction phase
     *  @param address Some instruction might require address. Use -1 for ignoring the address value
     *  @param tx_buffer Buffer containing data to be sent to peripheral                          
     *  @param tx_length Pointer to a variable containing the length of data to be transmitted, and on return this variable will be updated with the actual number of bytes written
     *  @param rx_buffer Buffer for data to be read from the peripheral                          
     *  @param rx_length Pointer to a variable containing the length of rx_buffer, and on return this variable will be updated with the actual number of bytes read
     *
     *  @returns
     *    Returns QSPI_STATUS_SUCCESS on successful reads and QSPI_STATUS_ERROR on failed reads.
     */
    qspi_status_t command_transfer(unsigned int instruction, int address, const char *tx_buffer, size_t tx_length, const char *rx_buffer, size_t rx_length);

    /** Acquire exclusive access to this SPI bus
     */
    virtual void lock(void);

    /** Release exclusive access to this SPI bus
     */
    virtual void unlock(void);

public:
    virtual ~QSPI() {
    }

protected:
    qspi_t _qspi;

    bool acquire(void);
    static QSPI *_owner;
    static SingletonPtr<PlatformMutex> _mutex;
    qspi_bus_width_t _inst_width; //Bus width for Instruction phase
    qspi_bus_width_t _address_width; //Bus width for Address phase
    qspi_address_size_t _address_size;
    qspi_bus_width_t _alt_width; //Bus width for Alt phase
    qspi_alt_size_t _alt_size;
    qspi_bus_width_t _data_width; //Bus width for Data phase
    qspi_command_t _qspi_command; //QSPI Hal command struct
    int _hz; //Bus Frequency
    int _mode; //SPI mode
    bool _initialized;
    PinName _qspi_io0, _qspi_io1, _qspi_io2, _qspi_io3, _qspi_clk, _qspi_cs; //IO lines, clock and chip select

private:
    /* Private acquire function without locking/unlocking
     * Implemented in order to avoid duplicate locking and boost performance
     */
    bool _acquire(void);
    bool _initialize();

    /* 
     * This function builds the qspi command struct to be send to Hal
     */
    inline void _build_qspi_command(int instruction, int address, int alt, int dummy_cnt);
};

} // namespace mbed

#endif

#endif
