#define PRU0_ARM_INTERRUPT 19

#define ADC_BASE 0x44e0d000

#define CONTROL 0x0040
#define SPEED   0x004c
#define STEP1   0x0064
#define DELAY1  0x0068
#define STATUS  0x0044
#define STEPCONFIG  0x0054
#define FIFO0COUNT  0x00e4

#define ADC_FIFO0DATA   (ADC_BASE + 0x0100)

// Register allocations
#define ticks     r5 // Number of times thru the capture loop
#define adc_      r6 // Offset to ADC_
#define fifo0data r7 // Offset to ADC_ FIFO
#define locals    r8 // Offset to local shared memory

// Constant configurations
#define ema_pow   r9  // EMA Power
#define enc_thrsh r10 // Encoder Threshold

// Used to store encoder values
#define enc_value r11
#define enc_min   r12
#define enc_max   r13
#define enc_ticks r14
#define enc_speed r15
#define enc_acc   r16
#define enc_delay r17
#define enc_up    r18
#define enc_down  r19

#define tmp0  r1
#define tmp1  r2
#define tmp2  r3
#define tmp3  r4

.origin 0
.entrypoint START

START:
	LBCO r0, C4, 4, 4  // Load Bytes Constant Offset (?)
	CLR  r0, r0, 4     // Clear bit 4 in reg 0
	SBCO r0, C4, 4, 4  // Store Bytes Constant Offset

	MOV adc_, ADC_BASE
	MOV fifo0data, ADC_FIFO0DATA
	MOV locals, 0
	MOV ticks, 0
	MOV enc_value, 0
	MOV enc_min, 0xffff
	MOV enc_max, 0
	MOV enc_ticks, 0
	MOV enc_speed, 0
	MOV enc_acc, 0
	MOV enc_delay, 0
	MOV enc_up, 0
	MOV enc_down, 0

	LBBO tmp0, locals, 0, 4   // check eyecatcher
	MOV tmp1, 0xbeef1965
	QBNE QUIT, tmp0, tmp1     // bail out if does not match

        // Load configuration variables
	LBBO ema_pow, locals, 0x1c, 4
	LBBO enc_thrsh, locals, 0x44, 4
	LBBO enc_delay, locals, 0x60, 4

	// Disable ADC_
	LBBO tmp0, adc_, CONTROL, 4
	MOV  tmp1, 0x1
	NOT  tmp1, tmp1
	AND  tmp0, tmp0, tmp1
	SBBO tmp0, adc_, CONTROL, 4

	// Put ADC capture to its full speed
	MOV tmp0, 0
	SBBO tmp0, adc_, SPEED, 4

	// Configure STEP1 to read from configured pin
	LBBO tmp0, locals, 0x40, 1
	LSL tmp0, tmp0, 19
	ADD tmp0, tmp0, 1 // 1 & 0x80 for 16 avg samples
	SBBO tmp0, adc_, STEP1, 4   // Open pin in continuous mode
	MOV tmp0, 0
	SBBO tmp0, adc_, DELAY1, 4  // Open pin w/o delay

	// Enable ADC_ with the desired mode (make STEPCONFIG registers writable, use tags, enable)
	LBBO tmp0, adc_, CONTROL, 4
	OR   tmp0, tmp0, 0x7
	SBBO tmp0, adc_, CONTROL, 4

	MOV tmp0, 0x2                    // Only enable step 1
	SBBO tmp0, adc_, STEPCONFIG, 4   // write STEPCONFIG register (this triggers capture)

WAIT_FOR_FIFO0:
	LBBO tmp0, adc_, FIFO0COUNT, 4
	QBNE WAIT_FOR_FIFO0, tmp0, 1

CAPTURE:
	// check for exit flag
	LBBO tmp0, locals, 0x08, 4   // read runtime flags
	QBNE QUIT, tmp0.b0, 0

	// increment ticks
	ADD  ticks, ticks, 1
	SBBO ticks, locals, 0x04, 4

	// Disabled for the moment to keep speed up
	// increment encoder ticks
	// ADD enc_acc, enc_acc, 1
	// MAX enc_speed, enc_speed, enc_acc

	// Read current ADC value from fifo
	LBBO enc_value, fifo0data, 0, 4

	// Disabled for the moment to keep speed up
	// Perform moving average and store in enc_value
	// LBBO tmp0, fifo0data, 0, 4
	// LBBO ema_pow, locals, 0x1c, 4
	// LSR tmp1, enc_value, ema_pow
	// SUB tmp1, tmp0, tmp1
	// ADD enc_value, enc_value, tmp1

	// Update min and max
	MIN enc_min, enc_value, enc_min
	MAX enc_max, enc_value, enc_max

	CALL PROCESS

	SBBO &enc_value, locals, 0x48, 16

	JMP CAPTURE

QUIT:
	MOV R31.b0, PRU0_ARM_INTERRUPT+16   // Send notification to Host for program completion
	HALT

PROCESS:
	ADD tmp2, enc_min, enc_thrsh        // tmp2 = min + threshold
	QBLT MAYBE_TOHIGH, enc_value, tmp2  // if ((min + thresh) < val)
	ADD tmp2, enc_value, enc_thrsh      // tmp2 = value + threshold
	QBLT MAYBE_TOLOW, enc_max, tmp2     // if ((value + thresh) < max)

	MOV enc_up, 0
	MOV enc_down, 0

	RET

MAYBE_TOHIGH:
	ADD enc_up, enc_up, 1           // enc_up++
	MOV enc_down, 0                 // enc_down == 0
	QBLT TOHIGH, enc_up, enc_delay  // if (enc_delay < enc_up)

	RET

MAYBE_TOLOW:
	ADD enc_down, enc_down, 1       // enc_down++
	MOV enc_up, 0                   // enc_up == 0
	QBLT TOLOW, enc_down, enc_delay // if (enc_delay < enc_down)

	RET

TOLOW:
	MOV enc_up, 0
	MOV enc_down, 0

	MOV enc_min, enc_value
	MOV enc_max, enc_value

	ADD enc_ticks, enc_ticks, 1

	// Disabled for the moment to keep speed up
	// MOV enc_speed, enc_acc
	// MOV enc_acc, 0

	RET
	
TOHIGH:
	MOV enc_up, 0
	MOV enc_down, 0

	MOV enc_min, enc_value
	MOV enc_max, enc_value

	RET
