/* 
 * Comms Test
 * American University Particle Detector
 * Washington, DC, United States of America
 * Part of the VASpace ThinSat Program
 * 
 * Author: AU ThinSat Team
 * Date: 2019 May 26
 * 
 * This code shows an example of working comms between the NSL board and the 'MEDO' (xinabox) boards.
 * No meaningful payload information is included, other than the number of requests from the NSL board.
 * Tested on Teensy 3.2 connected via Serial pins 0 and 1 to the xBus connector on an MN01.
 * 
 */

// ------------------------------------------------------------------------------------------------
// globals
// ------------------------------------------------------------------------------------------------

// payload struct: code-convenient linearly-stored block of information
// total size: 38 bytes
// NSL-specified structure for payload: 3-byte header plus 35 bytes of info
struct payload_struct {
    uint8_t header[4] = {0x50, 0x50, 0x50, 0};                              // header plus payload request count
    uint16_t int_01;                                                        // 17 currently unspecified integers (this is a comms test!)
    uint16_t int_02;
    uint16_t int_03;
    uint16_t int_04;
    uint16_t int_05;
    uint16_t int_06;
    uint16_t int_07;
    uint16_t int_08;
    uint16_t int_09;
    uint16_t int_10;
    uint16_t int_11;
    uint16_t int_12;
    uint16_t int_13;
    uint16_t int_14;
    uint16_t int_15;
    uint16_t int_16;
    uint16_t int_17;
};

#define         serial_computer         Serial                              // connection to the computer for debugging
#define         serial_nsl              Serial1                             // connection to the NSL board for payload delivery
#define         serial_computer_baud    115200                              // baud for talking to computer
#define         serial_nsl_baud         9600                                // baud for talking to NSL board

elapsedMillis   current_timer;                                              // timer used to check serial NSL line regularly for requests 
uint32_t        window                  = 10;                               // how regularly to check for a request from NSL board. this can probably be upped
payload_struct  payload;                                                    // all payload information
char            *buff                   = (char*)malloc(sizeof(payload));   // buffer to drop payload into, which is then sent to NSL board
uint8_t         pin_led                 = 13;                               // used for visual of NSL board request

// ------------------------------------------------------------------------------------------------
// one-time stuff
// ------------------------------------------------------------------------------------------------
void setup() {
    // serial_nsl.setTX(5);                                                 // for teensy's alt Serial pins
    // serial_nsl.setRX(21);
    pinMode(pin_led, OUTPUT);
    serial_nsl.begin(serial_nsl_baud);
    serial_computer.begin(serial_computer_baud);
    // while(!serial_nsl);
    // while(!serial_computer);
    serial_computer.println("----------------------------------------------------------------");
    serial_computer.println("-                          Comms Test                          -");
    serial_computer.println("-            American University Particle Detector             -");
    serial_computer.println("-           Washington, DC, United States of America           -");
    serial_computer.println("----------------------------------------------------------------");
}

// ------------------------------------------------------------------------------------------------
// regularly-recurring stuff: check to see if NSL requested information; if it did, send it.
// ------------------------------------------------------------------------------------------------
void loop() {
    bool requested = false;
    if (current_timer > window) {
        requested = check_serial_NSL();
        payload.int_17++;
        current_timer = 0;
    }
    if (requested) {
        payload.header[3]++;
        send_payload();
        blink_led(1);
    }
}

// ------------------------------------------------------------------------------------------------
// checks NSL serial line for information; only cares about payload requests
// parameters: none
// returns:    true if NSL board requested a payload
//             false otherwise
// todo:       implement other requests
// ------------------------------------------------------------------------------------------------
bool check_serial_NSL() {
    if (serial_nsl.available()) {
        serial_computer.println("Received msg from NSL board.");
        bool r = true;
        for (byte j = 0; j < 3; j++) {
            byte in = serial_nsl.read();
            serial_computer.print("\treceived: ");
            serial_computer.println(in, HEX);
            r = (in == 0x49) & r;
        }
        while (serial_nsl.available()) {
            serial_nsl.read();
        }
        serial_nsl.flush();
        if (r) {
            serial_computer.println("Time to send payload!");
            return true;
        } else return false;
    }
}

// ------------------------------------------------------------------------------------------------
// sends payload to NSL board
// parameters: none
// returns:    true always
//             false never
// todo:       return needs to truly represent the status of the sending message!
// ------------------------------------------------------------------------------------------------
bool send_payload() {
    serial_computer.print("Sending payload of size ");
    serial_computer.println(sizeof(payload));
    memcpy(buff, (char*)&payload, sizeof(payload));
    for (uint8_t i = 0 ; i < sizeof(payload) ; i++) {
        serial_nsl.print(buff[i]);
    }
    serial_computer.print("\tbuff presend:\t");
    print_buff();
    memset(buff, 0, sizeof(payload));
    serial_computer.println("----------------------------------------------------------------");
    return true; // needs fixed: truly check for size of message sent
}
// ------------------------------------------------------------------------------------------------
// blinks onboard LED
// parameters: n, the number of times the LED will blink
// returns:    none
// ------------------------------------------------------------------------------------------------
void blink_led(uint16_t n) {
    for (uint16_t i = 0 ; i < n ; i++) {
        digitalWrite(pin_led, HIGH);
        delay(200);
        digitalWrite(pin_led, LOW);
        delay(200);
    }
}

// ------------------------------------------------------------------------------------------------
// prints the newly-filled buffer in its entirety, with pipes between values
// parameters: none
// returns:    none
// ------------------------------------------------------------------------------------------------
void print_buff() {
    for (uint8_t i = 0 ; i < sizeof(payload) ; i++) {
        serial_computer.print(buff[i], HEX);
        serial_computer.print(" | ");
    }
    serial_computer.println();
}
