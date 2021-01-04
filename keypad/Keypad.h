/* mbed Keypad library, using user-defined interrupt callback
 * Copyright (c) 2012 Yoong Hor Meng
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
 */

#ifndef KEYPAD_H
#define KEYPAD_H

#include "mbed.h"
#include "FPointer.h"

/**
 * An interrupt-based interface to 4x4 keypad.
 *
 * On each key pressed on a keypad, the index of the key is passed to a
 * user-defined function. User is free to define what to be done with the
 * input.
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "Keypad.h"
 *
 * // Define your own keypad values
 * char Keytable[] = { '1', '2', '3', 'A',   // r0
 *                     '4', '5', '6', 'B',   // r1
 *                     '7', '8', '9', 'C',   // r2
 *                     '*', '0', '#', 'D'    // r3
 *                   };
 *                  // c0   c1   c2   c3
 *
 * uint32_t Index;
 *
 * uint32_t cbAfterInput(uint32_t index) {
 *     Index = index;
 *     return 0;
 * }
 *
 * int main() {
 *                 // r0   r1   r2   r3   c0   c1   c2   c3
 *     Keypad keypad(p21, p22, p23, p24, p25, p26, p27, p28);
 *     keypad.attach(&cbAfterInput);
 *     keypad.start();  // energize the keypad via c0-c3
 *
 *     while (1) {
 *         __wfi();
 *         printf("Interrupted\r\n");
 *         printf("Index:%d => Key:%c\r\n", Index, Keytable[Index]);
 *     }
 * }
 * @endcode
 */

class Keypad {
public:
    /** Create a 4x4 (row, col) or 4x3 keypad interface:
     *
     *          | Col0 | Col1 | Col2 | Col3
     *   -------+------+------+------+-----
     *   Row 0  |   x  |   x  |   x  |  x
     *   Row 1  |   x  |   x  |   x  |  x
     *   Row 2  |   x  |   x  |   x  |  x
     *   Row 3  |   x  |   x  |   x  |  x
     *
     *  @param row<0..3>     Row data lines
     *  @param col<0..3>     Column data lines
     *  @param debounce_ms   Debounce in ms (Default to 20ms)
     */
    Keypad(PinName r0, PinName r1, PinName r2, PinName r3,
           PinName c0, PinName c1, PinName c2, PinName c3,
           int debounce_ms = 20);

    /** Destructor
     */
    ~Keypad();
                    
    /** Start the keypad interrupt routines
     */
    void start(void);

    /** Stop the keypad interrupt routines
     */
    void stop(void);

    /** User-defined function that to be called when a key is pressed
     *  @param fptr           A function pointer takes a uint32_t and
     *                        returns uint32_t
     */
    void attach(uint32_t (*fptr)(uint32_t));

protected:
    InterruptIn     *_rows[4];
    DigitalOut      *_cols[4];
    int              _debounce;  // miliseconds
    int              _nRow;
    int              _nCol;
    FPointer         _callback; // Called after each input

    void _checkIndex(int row, InterruptIn *therow);
    void _cbRow0Rise(void);
    void _cbRow1Rise(void);
    void _cbRow2Rise(void);
    void _cbRow3Rise(void);
    void _setupRiseTrigger(void);
};

#endif // KEYPAD_H

