#include "uinput.h"

#include <cstring>
#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <unistd.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;

// Various uinput files we try and open
static const char *uinput_filename[] = {"/dev/uinput", "/dev/input/uinput", "/dev/misc/uinput"};

static Logger logger = Logger::getInstance("uinput");

UInput::UInput(const char *dev_name, const std::vector< std::list<__u16> > & keys) : fd(-1) {
	openAll();
	setup(dev_name, keys);
	create();
}

UInput::~UInput() {
	destroy();
}

int UInput::open(const char *uinput_path) {
	this->fd = ::open(uinput_path, O_WRONLY | O_NONBLOCK);
	if(this->fd < 0) {
		return errno;
	}
	return 0;
}

/**
 * Try each uinput until one works
 */
void UInput::openAll() {
	int ret = ENOENT;
	for (int i = 0; i < 3; i++) {
		ret = open(uinput_filename[i]);

		// If all things worked, then bail
		if (ret == 0) {
			LOG4CPLUS_INFO(logger, "Opened " << uinput_filename[i]);
			break;
		}

		// If the device isn't found, try the next one
		if (ret == ENOENT)
			continue;

		if (ret == EACCES) {
			LOG4CPLUS_ERROR(logger, "Permission denied. Check you have permission to uinput.");
		} else {
			LOG4CPLUS_ERROR(logger, errno << " " << strerror(errno));
		}

		throw std::runtime_error("Failed to open uinput");
	}

	if (ret != 0) {
		LOG4CPLUS_ERROR(logger, "uinput was not found. Is the uinput module loaded?");
		throw std::runtime_error("Failed to open uinput");
	}
}

void UInput::setup(const char *dev_name, const std::vector< std::list<__u16> > & keys) {

	int ret;
	struct uinput_user_dev uidev;
	memset(&uidev, 0, sizeof(uidev));

	strncpy(uidev.name, dev_name, UINPUT_MAX_NAME_SIZE);
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 1;
	uidev.id.product = 1;
	uidev.id.version = 1;

	ret = write(this->fd, &uidev, sizeof(uidev));
	if (ret != sizeof(uidev)) {
		throw std::runtime_error("Failed to setup uinput");
		//cerr << "Failed to setup uinput: " << errno << " " << strerror(errno) << endl;
	}

	// We only want to send keypresses
	ret  = ioctl(this->fd, UI_SET_EVBIT, EV_KEY);

	// Add all the keys we might use
	for (std::vector< std::list<__u16> >::const_iterator i = keys.begin(); i != keys.end(); ++i) {
		const std::list<__u16> & kk = *i;
		for (std::list<__u16>::const_iterator k = kk.begin(); k != kk.end(); ++k) {
			__u16 ukey = *k;
			if (ukey != KEY_RESERVED)
				ret |= ioctl(this->fd, UI_SET_KEYBIT, ukey);
		}
	}

	if (ret) {
    	throw std::runtime_error("Failed to setup uinput");
		//cerr << "Failed to setup uinput" " << errno << " " << strerror(errno) << endl;
	}
}

void UInput::create() {
	int ret = ioctl(this->fd, UI_DEV_CREATE);
	if (ret) {
		throw std::runtime_error("Failed to create uinput");
	}

	LOG4CPLUS_INFO(logger, "Created uinput device");

	// This sleep is here, because (for some reason) you need to wait before
	// sending you first uinput event.
	sleep(1);
}

void UInput::send_event(__u16 type, __u16 code, __s32 value) const {
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));

	ev.type  = type;
	ev.code  = code;
	ev.value = value;

	int ret = write(this->fd, &ev, sizeof(ev));
	if (ret != sizeof(ev)) {
		throw std::runtime_error("Failed to send_event");
	}
}

void UInput::sync() const {
	send_event(EV_SYN, SYN_REPORT, 0);
}


void UInput::destroy() {
	ioctl(this->fd, UI_DEV_DESTROY);
	close(this->fd);

	this->fd = -1;
}
