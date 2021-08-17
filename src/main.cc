#include <iostream>
#include <chrono>

#include "hyperfractal.h"
#include "gui.h"
#include "../lib/mpc.h"

int main (int argc, char *argv[]) {
    complex<long double> shorter = complex<long double>(0.350004947826582879738619574761, 0.422633999014268769788384497166);
    mpc_t longer;
    mpc_init2 (longer, 512);
    mpc_set_ld_ld (longer, 0.350004947826582879738619574761, 0.422633999014268769788384497166, MPC_RNDNN);


    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i<100; i++) {
        shorter = pow (shorter, shorter);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i<100; i++) {
        mpc_pow (longer, longer, longer, MPC_RNDNN);
    }
    auto t4 = std::chrono::high_resolution_clock::now();


    std::cout << "Normal complex at ultra-precision: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "Super long complex at ultra-precision: " << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
    std::cout << "Normal="; printf ("%.100Lf", shorter.real()); std::cout << std::endl;
    std::cout << "Long="; printf ("%.100Lf", longer->re); std::cout << std::endl;




    if (argc == 8) {
        hfractal_main hm;
        hm.resolution = stoi (argv[1]);
        hm.offset_x = stod (argv[2]);
        hm.offset_y = stod (argv[3]);
        hm.zoom = stod (argv[4]);
        hm.eq = string (argv[5]);
        hm.worker_threads = stoi (argv[6]);
        hm.eval_limit = stoi (argv[7]);
        hm.generateImage(true);
        return hm.write ("out.pgm");
    } else if (argc != 1) {
        std::cout << "Provide all the correct arguments please:" << std::endl;
        std::cout << "int resolution, long double offset_x, long double offset_y, long double zoom, string equation, int worker_threads, int eval_limit" << std::endl;
        return 1;
    } else {
        return gui_main ();
    }
}