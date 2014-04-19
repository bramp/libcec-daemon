#include "hdmi.h"

#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>

namespace HDMI {

std::ostream& operator<<(std::ostream &out, const HDMI::physical_address & address)
{
    return out << address[0] << '.' << address[1] << '.'
               << address[2] << '.' << address[3];
}

std::istream& operator>>(std::istream &in, HDMI::physical_address & address)
{
    std::string s;

    in >> s;

    std::istringstream ss(s);
    int val[4] = { 0,0,0,0 };
    int len=0;

    do
    {
        ss >> val[len];
        if( ss.fail() || val[len] <0 || val[len]>15 )
        {
            in.setstate(std::ios::failbit);
            break;
        }
        switch( ss.peek() )
        {
            case EOF:
                break;
            case '.':
                ss.ignore();
                break;
            default:
                in.setstate(std::ios::failbit);
                break;
        }
    }
    while( !ss.eof() && (++len < 4) );

    if( len == 4 )
    {
        in.setstate(std::ios::failbit);
    }
    else if( ! in.fail() )
    {
        address.set(val);
    }

    return in;
}

std::istream& operator>>(std::istream &in, HDMI::address & address)
{
    char c = in.peek();
    if( c >= '0' && c <= '9' )
    {
        in >> address.physical;
    }
    else
    {
        std::string s;

        in >> s;

        if( s.find("tv") == 0 )
        {
            address.logical = CEC::CECDEVICE_TV;
            s.erase(0, 2);
        }
        else if( s.find("av") == 0 )
        {
            address.logical = CEC::CECDEVICE_AUDIOSYSTEM;
            s.erase(0, 2);
        }
        else
        {
            in.setstate(std::ios::failbit);
            return in;
        }

        if( s.empty() )
        {
            if( address.logical == CEC::CECDEVICE_TV )
            {
                /* auto detect port when using tv */
                address.port = 0;
            }
            else
            {
                /* need a specific port otherwise */
                in.setstate(std::ios::failbit);
            }
        }
        else
        {
            // look for port
            std::istringstream ss(s);
            char c;
            unsigned port;

            ss >> c >> port;

            if( ss.fail() || c != '.' || port < 1 || port > 15 )
                in.setstate(std::ios::failbit);

            address.port = port;
        }
    }
    return in; 
}

std::ostream& operator<<(std::ostream &out, const HDMI::address & address)
{
    switch( address.logical )
    {
        case CEC::CECDEVICE_TV:
            out << "tv";
            if( address.port != 0 )
            {
                out << '.' << address.port;
            }
            break;
        case CEC::CECDEVICE_AUDIOSYSTEM:
            out << "av";
            if( address.port != 0 )
            {
                out << '.' << address.port;
            }
            break;
        default:
            out << address.physical;
            break;
    }
}

}