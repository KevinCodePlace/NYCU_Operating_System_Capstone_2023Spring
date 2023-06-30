// C++ library headers
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <vector>

using namespace std;
// Linux headers
#include <string.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#define block_size 128

void port_setting(int serial_port, struct termios tty){
    
    //general setting
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    tty.c_cflag |= CS8; // 8 bits per byte
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON; //Disable canonial mode. Canonial mode is sending message line by line.
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    //setting input
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    
    //setting output
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    cfsetspeed(&tty, B115200); //Set baud rate

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
            printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
            exit(errno);
    }
}

int main(int argc, char* argv[]){
    if(argc!=3){
        cout<<"Error! Need the port number and file name!"<<endl;
        return 1;
    }


    int serial_port = open(argv[1], O_RDWR); //oepn port
    struct termios tty;
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno)); //if not found, return
        return errno;
    }
    port_setting(serial_port,tty);


    fstream  file; 
    file.open(argv[2],ios::in | ios::binary);
    if(!file){
        printf("Error %i from open: %s\n", errno, strerror(errno)); //if not found, return
        return errno;
    }
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    uint32_t size=buffer.size();
    unsigned char par_bit=0;
    unsigned char SIZE[5];
    //unsigned char command[]={'l','o','a','d','i','m','g','\n'};
    //write(serial_port,command,sizeof(command));

    for(int i=0;i<4;i++){
        SIZE[3-i] = size%256;
        size/=256;
        par_bit ^= SIZE[3-i];
    }
    SIZE[4]=par_bit;
    size=buffer.size();
    write(serial_port, SIZE, sizeof(SIZE));
    
    uint32_t now=0;
    char check_byte=0;
    for(int i=0;i<size;i++){
        char out[1];
        out[0]=buffer[i];
        write(serial_port, out, 1);
        check_byte ^= out[0];
        usleep(10);
        if(i%8 == 7){
            write(serial_port, &check_byte, 1);
            check_byte &= 0;
            read( serial_port, &out, 1);
            if(out[0] == 2 ) exit(1);
            else if(out[1] == 1) i-=8;
            usleep(10);
        }
    }

    file.close();
    close(serial_port);
    return 0; // success
}