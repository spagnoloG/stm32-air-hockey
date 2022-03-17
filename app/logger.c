#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
// *nix headers
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <fcntl.h> 
#include <errno.h>
#include <termios.h> 
#include <unistd.h> 
#include <poll.h>

// defines
#define BUFF_SIZE 64

// function declarations
void setup_tty_structure(struct termios *tty);
void setup_interrupt_structure(struct sigaction *saio);
int recv_data(char *buff, int serial_port);
void send_data(unsigned char *buff, int buff_size);

// interrup declaration
void signal_handler_IO (int status);

int serial_port_descriptor;
FILE *log_file_descriptor;

int main() {

  // define tty and interrupt structures
  struct termios tty;
  struct sigaction saio;
  
  serial_port_descriptor = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_SYNC);
  log_file_descriptor = fopen("play_activity.log", "a");

  // check if serial port is opened succesfully
  if(tcgetattr(serial_port_descriptor, &tty) != 0) {
      fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  // check if file is opened
  if(log_file_descriptor == NULL) {
      fprintf(stderr, "Error %i file: %s\n", errno, strerror(errno));
      return 1; 
  }  

  // setup tty and interrupt handler
  setup_interrupt_structure(&saio); 
  setup_tty_structure(&tty);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port_descriptor, TCSANOW, &tty) != 0) {
      fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  // flush serial port before reading data
  if(tcflush(serial_port_descriptor, TCIFLUSH) != 0) {
      fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  while(1) {
    sleep(1);
  }

  close(serial_port_descriptor);
  return 0;
}

/*
* this function initializes UART connection with same parameters as on STM32
**/
void setup_tty_structure(struct termios *tty) {
    tty->c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty->c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty->c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty->c_cflag |= CS8; // 8 bits per byte (most common)
    tty->c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty->c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty->c_lflag &= ~ICANON;
    tty->c_lflag &= ~ECHO; // Disable echo
    tty->c_lflag &= ~ECHOE; // Disable erasure
    tty->c_lflag &= ~ECHONL; // Disable new-line echo
    tty->c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty->c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty->c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty->c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
                            // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
                            // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty->c_cc[VTIME] = 200;  // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty->c_cc[VMIN] =  11;  // Define how many bytes are buffered before recieving / sending

    // Set in/out baud rate to be 115200
    cfsetispeed(tty, B115200);
    cfsetospeed(tty, B115200);
}

/*
* this function setups interrupt for tty
**/
void setup_interrupt_structure(struct sigaction *saio) {
  saio->sa_handler = signal_handler_IO;
  saio->sa_flags = 0;
  saio->sa_restorer = NULL;
  sigaction(SIGIO, saio, NULL);

  fcntl(serial_port_descriptor, F_SETFL, FNDELAY);
  fcntl(serial_port_descriptor, F_SETOWN, getpid());
  fcntl(serial_port_descriptor, F_SETFL,  O_ASYNC ); 
}

/*
* this function reads data from tty and returns number of bytes read
**/
int recv_data(char *buff, int serial_port) {
    // lets set all bytes to null terminator
    memset(buff, '\0', BUFF_SIZE);

    int num_bytes = read(serial_port, buff, BUFF_SIZE);
    
    if (num_bytes < 0) {
        fprintf(stderr, "Error reading: %s", strerror(errno));
        return -1;
    }
    return num_bytes;
}

/*
* function that sends buffer to STM
**/
void send_data(unsigned char *buff, int buff_size) {
  // flush output buffer 
  if(tcflush(serial_port_descriptor, TCOFLUSH) != 0)
      fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
  write(serial_port_descriptor, buff, buff_size);
}

/*
* interrupt handler, which is called when new data appears on buffer
**/
void signal_handler_IO (int status) {
    // get current time
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char read_buf[BUFF_SIZE];    
    int bytes_recieved = recv_data(read_buf, serial_port_descriptor);
        
    // form a log message
    char log[150];
    sprintf(log, "[Message | %d:%d.%d] ---> %s (size: %dB)  \n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, read_buf, bytes_recieved);
   
    printf("%s", log);
    fprintf(log_file_descriptor, "%s", log);
    fflush(log_file_descriptor);
}
