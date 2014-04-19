#include <cstdint>
#include <iostream>
#include <libcec/cectypes.h>

namespace HDMI
{
    class physical_address
    {
        public:

        physical_address(uint16_t address=0) : address(address) {};
        physical_address(int a, int b=0, int c=0, int d=0) :
            address((a & 15)<<12|(b & 15)<<8|(c & 15)<<4|(d & 15)) {};

        operator uint16_t() const { return address; };
        int operator [] (int x) const { return (address>>((3 & ~x)*4)) & 15; };

        void set(const int (&val)[4])
        {
            address = (val[0] & 15)<<12|(val[1] & 15)<<8|(val[2] & 15)<<4|(val[3] & 15);
        }

        protected:

        uint16_t address;
    };

    class address
    {
        public:

        address() : physical(0), logical(CEC::CECDEVICE_UNKNOWN), port(0) {};

        physical_address physical;
        CEC::cec_logical_address logical;
        uint8_t port;
    };

    std::ostream& operator<<(std::ostream &out, const HDMI::physical_address & address);
    std::istream& operator>>(std::istream &in, HDMI::physical_address & address);

    std::ostream& operator<<(std::ostream &out, const HDMI::address & address);
    std::istream& operator>>(std::istream &in, HDMI::address & address);
};


