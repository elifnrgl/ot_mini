#include <cstdio>
#include <iostream>
#include <string>
#include "cli_dataset.h"

int main()
{
    ot_mini::Cli cli;
    std::string  line;

    std::printf("ot-mini: kendi mini OpenThread CLI'miz (dataset, ifconfig, thread, state, ipaddr, ipmaddr, "
                 "netdata, ping, udp, coap). Cikmak icin 'exit' yaz.\n");

    while (true)
    {
        std::printf("> ");
        std::fflush(stdout);

        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;

        cli.ProcessLine(line);
    }

    return 0;
}
