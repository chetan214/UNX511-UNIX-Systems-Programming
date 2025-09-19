#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

using namespace std;

int main() {
    // Redirect stderr to Screen.log
    int log_fd = open("Screen.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd < 0) {
        cerr << "Error opening Screen.log: " << strerror(errno) << endl;
        return 1;
    }
    dup2(log_fd, STDERR_FILENO);
    close(log_fd);

    // Open /dev/fb0 as read-only and non-blocking
    int fb_fd = open("/dev/fb0", O_RDONLY | O_NONBLOCK);
    if (fb_fd < 0) {
        cerr << "Error opening /dev/fb0: " << strerror(errno) << endl;
        return 1;
    }

    int choice;
    do {
        cout << "\n====== Framebuffer Info Menu ======\n";
        cout << "1. Fixed Screen Info\n";
        cout << "2. Variable Screen Info\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (cin.fail()) {
            cerr << "Invalid input. Exiting.\n";
            break;
        }

        if (choice == 1) {
            struct fb_fix_screeninfo fix_info;
            int ret = ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix_info);
            if (ret < 0) {
                cerr << "FBIOGET_FSCREENINFO failed: " << strerror(errno) << endl;
            } else {
                cout << "\n--- Fixed Screen Info ---\n";
                cout << "Framebuffer ID: " << fix_info.id << endl;
                cout << "Memory Start: 0x" << hex << fix_info.smem_start << dec << endl;
                cout << "Memory Length: " << fix_info.smem_len << " bytes\n";
                cout << "Type: " << fix_info.type << endl;
                cout << "Visual: " << fix_info.visual << " (FB_VISUAL_*)\n";   // report this number
                cout << "Acceleration: " << fix_info.accel << " (FB_ACCEL_*)\n"; // report this number
                cout << "Capabilities: " << fix_info.capabilities << " (FB_CAP_*)\n"; // report this number
            }
        }
        else if (choice == 2) {
            struct fb_var_screeninfo var_info;
            int ret = ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_info);
            if (ret < 0) {
                cerr << "FBIOGET_VSCREENINFO failed: " << strerror(errno) << endl;
            } else {
                cout << "\n--- Variable Screen Info ---\n";
                cout << "Screen Resolution: " << var_info.xres << "x" << var_info.yres << endl;
                cout << "Virtual Resolution: " << var_info.xres_virtual << "x" << var_info.yres_virtual << endl;
                cout << "Bits Per Pixel: " << var_info.bits_per_pixel << endl;

                // Report these in comments:
                // X resolution = var_info.xres
                // Y resolution = var_info.yres
                // Bits per pixel = var_info.bits_per_pixel
            }
        }
        else if (choice == 0) {
            cout << "Exiting program.\n";
        }
        else {
            cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 0);

    close(fb_fd);
    return 0;
}
