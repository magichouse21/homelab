#include <stdio.h>
#include <string.h>      // memset, strncpy
#include <fcntl.h>       // open, O_RDWR
#include <unistd.h>      // read, write, close
#include <sys/ioctl.h>   // ioctl
#include <net/if.h>      // struct ifreq, IFNAMSIZ
#include <linux/if_tun.h> // IFF_TAP, IFF_NO_PI, TUNSETIFF

int fd;
int tapif_init() {
    struct ifreq ifr;
    int err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) { //assigns fd to process, if not found, err
        printf("can't open /dev/net/tun");
        return fd;
    }
    memset(&ifr, 0, sizeof(ifr)); //now that we have space, we need to make sure it aint used already
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, "tap0", IFNAMSIZ); //copies data from parameter 2 into parameter 1 with n characters (3rd param)
    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) { //
        printf("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }
    return fd;
}

int tap_read(char *buf, size_t bufsize) {
    int len = read(fd, buf, bufsize);
    if (len < 0) {
        perror("read from TAP");
        return -1;
    }
    printf("Read %d bytes from TAP\n", len);
    return len;
}

int tap_write(const char *buf, size_t len) {
    int written = write(fd, buf, len);
    if (written < 0) {
        perror("write to TAP");
        return -1;
    }
    printf("Wrote %d bytes to TAP\n", written);
    return written;
}

int main()
{
    fd = tapif_init();
    char arp_request[] = {
    // Ethernet Header (14 bytes)
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,       // Destination MAC: Broadcast
    0x00, 0x0c, 0x29, 0xab, 0xcd, 0xef,       // Source MAC: Your MAC
    0x08, 0x06,                               // EtherType: ARP (0x0806)

    // ARP Header (28 bytes)
    0x00, 0x01,                               // Hardware type: Ethernet (1)
    0x08, 0x00,                               // Protocol type: IPv4 (0x0800)
    0x06,                                     // Hardware size: 6
    0x04,                                     // Protocol size: 4
    0x00, 0x01,                               // Opcode: request (1)

    // Sender MAC address (6 bytes)
    0x00, 0x0c, 0x29, 0xab, 0xcd, 0xef,
    // Sender IP address (4 bytes)
    0xc0, 0xa8, 0x00, 0x01,                   // 192.168.0.1

    // Target MAC address (6 bytes, unknown)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // Target IP address (4 bytes)
    0xc0, 0xa8, 0x00, 0x02                    // 192.168.0.2
  };
  tap_write(arp_request, sizeof(arp_request));
}