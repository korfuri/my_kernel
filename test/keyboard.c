#include <keyboard.h>
#include <mutex.h>

char	keyboard_buffer[256];
char*	keyboard_readPtr = keyboard_buffer;
char*	keyboard_writePtr = keyboard_buffer;

static char scancode_to_char(unsigned int n);

void keyboard_write(unsigned int n) {
  char c = scancode_to_char(n);
  if (c != 0) {
    *keyboard_writePtr++ = c;
    if (keyboard_writePtr > keyboard_buffer + KEYBOARD_BUFFER_SIZE)
      keyboard_writePtr = keyboard_buffer;
  }
}

size_t keyboard_read(char* buf, size_t len) {
  // FIXME UGLY
  static mutex_t keyboard_read_m = MUTEX_INIT;
  size_t i;

  if (len > KEYBOARD_BUFFER_SIZE)
    len = KEYBOARD_BUFFER_SIZE;
  if (len == 0)
    return 0;
  
  mutex_lock(&keyboard_read_m);
  for (i = 0; i < len && keyboard_readPtr != keyboard_writePtr; i++) {
    buf[i] = *keyboard_readPtr++;
    if (keyboard_readPtr > keyboard_buffer + KEYBOARD_BUFFER_SIZE)
      keyboard_readPtr = keyboard_buffer;
  }
  mutex_unlock(&keyboard_read_m);
  
  return i;
}



static char scancode_to_char(unsigned int n) {
  static unsigned int shift = 0; // 0 means not shifted. >0 means
				 // `shift` shiftkeys are being
				 // pressed
  
  // Is it a "released key" scancode ?
  int release = n & 0x80; // 0 means it's not a release
  // Filter out the "release" bit
  n &= (~0x80);

  if (!release) {

    switch (n) {
      // Shift keys
    case 0x2a: //lshift
    case 0x36: // rshift
      shift++;
      return 0;

      // These keys are not affected by the shift status
    case 0x1c: return '\n';
    case 0x39: return ' ';
    case 0x0f: return '\t';
    
      // Non-alphabetic keys that are affected by the shift status
    case 0x02: return shift == 0 ? '1' : '!';
    case 0x03: return shift == 0 ? '2' : '@';
    case 0x04: return shift == 0 ? '3' : '#';
    case 0x05: return shift == 0 ? '4' : '$';
    case 0x06: return shift == 0 ? '5' : '%';
    case 0x07: return shift == 0 ? '6' : '^';
    case 0x08: return shift == 0 ? '7' : '&';
    case 0x09: return shift == 0 ? '8' : '*';
    case 0x0a: return shift == 0 ? '9' : '(';
    case 0x0b: return shift == 0 ? '0' : ')';
    case 0x0c: return shift == 0 ? '-' : '_';
    case 0x0d: return shift == 0 ? '=' : '+';
    case 0x27: return shift == 0 ? ';' : ':';
    case 0x28: return shift == 0 ? '\'' : '\"';
    case 0x29: return shift == 0 ? '`' : '~';
    case 0x2b: return shift == 0 ? '\\' : '|';
    case 0x33: return shift == 0 ? '<' : ')';
    case 0x34: return shift == 0 ? '.' : '>';
    case 0x35: return shift == 0 ? '/' : '?';
    case 0x1a: return shift == 0 ? '[' : '{';
    case 0x1b: return shift == 0 ? ']' : '}';

      // Alphabetic keys
#define SHIFTED(x) (shift ? (x) : ((x) + 'a' - 'A'))
    case 0x10: return SHIFTED('Q');
    case 0x11: return SHIFTED('W');
    case 0x12: return SHIFTED('E');
    case 0x13: return SHIFTED('R');
    case 0x14: return SHIFTED('T');
    case 0x15: return SHIFTED('Y');
    case 0x16: return SHIFTED('U');
    case 0x17: return SHIFTED('I');
    case 0x18: return SHIFTED('O');
    case 0x19: return SHIFTED('P');
    case 0x1e: return SHIFTED('A');
    case 0x1f: return SHIFTED('S');
    case 0x20: return SHIFTED('D');
    case 0x21: return SHIFTED('F');
    case 0x22: return SHIFTED('G');
    case 0x23: return SHIFTED('H');
    case 0x24: return SHIFTED('J');
    case 0x25: return SHIFTED('K');
    case 0x26: return SHIFTED('L');
    case 0x2c: return SHIFTED('Z');
    case 0x2d: return SHIFTED('X');
    case 0x2e: return SHIFTED('C');
    case 0x2f: return SHIFTED('V');
    case 0x30: return SHIFTED('B');
    case 0x31: return SHIFTED('N');
    case 0x32: return SHIFTED('M');

      // Ignored keys
    case 0x00: // error code FIXME handle this, maybe ?
    case 0x01: // escape
    case 0x0e: // backspace
    case 0x1d: // Lctrl
    case 0x38: // Lalt
    case 0x3a: // capslock
    case 0x45: // numlock
    case 0x46: // scrolllock
    case 0x3b: // F1
    case 0x3c: // F2
    case 0x3d: // F3
    case 0x3e: // F4
    case 0x3f: // F5
    case 0x40: // F6
    case 0x41: // F7
    case 0x42: // F8
    case 0x43: // F9
    case 0x44: // F10
    case 0x57: // F11
    case 0x58: // F12
      return 0;

      // keypad keys - ignored for now
    case 0x37: // Keypad-* or * / PrtScn
    case 0x47: // Keypad-7/Home
    case 0x48: // Keypad-8/Up
    case 0x49: // Keypad-9/PgUp
    case 0x4a: // Keypad--
    case 0x4b: // Keypad-4/Left
    case 0x4c: // Keypad-5
    case 0x4d: // Keypad-6/Right
    case 0x4e: // Keypad-+
    case 0x4f: // Keypad-1/End
    case 0x50: // Keypad-2/Down
    case 0x51: // Keypad-3/PgDn
    case 0x52: // Keypad-0/Ins
    case 0x53: // Keypad-./Del
    case 0x54: // Alt-SysRq
      return 0;    
    }
  } else {
    if (n == 0x2a || n == 0x36)
      shift--;
  }

  return 0;
}
