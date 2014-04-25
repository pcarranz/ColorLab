// IO pin definitions
#define RFID_CONTROL_PIN 9  // RFID ENable pin
#define RED_PIN 5
#define GREEN_PIN 6
#define BLUE_PIN 3
#define READY_LED_PIN 7
#define RX_PIN 8    // Read pin
#define TX_PIN 1   // Write pin - Unconnected

// Serial protocol control bytes
#define SERIAL_OPCODE 10
#define SERIAL_ENDCODE 13
#define KEY_LENGTH 10
#define ACTIVATE_RFID_WAIT 5 // milliseconds

// Queue metrics
#define QUEUE_LENGTH 16

#define SLOW_FADE 10
#define FAST_FADE 1
#define MAX_NO_DISK_COUNT 200
#define NUM_TAGS 4

enum QMessage
{
  NONE,
  RED,
  GREEN,
  BLUE,
  YELLOW,
  CYAN,
  MAGENTA,
  RANDOM,
  RAINBOW,
  WHITE,
  PURPLE,
  EMPTY
};

void postMessage(QMessage msg);
QMessage getMessage();
