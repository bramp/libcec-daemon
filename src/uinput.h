#include <linux/input.h>

#include <vector>
#include <list>

#define EV_KEY_RELEASED 0
#define EV_KEY_PRESSED  1
#define EV_KEY_REPEAT   2

class UInput {
private:
	int fd; // Handle for uinput file ops

	int open(const char *uinput_path);
	void openAll();
	void setup(const char *dev_name, const std::vector< std::list<__u16> > & keys);
	void create();

	void destroy();

	// TODO Add something like
	// onUInputEvent(

public:
	UInput(const char *dev_name, const std::vector< std::list<__u16> > & keys);
	virtual ~UInput();

	void send_event(__u16 type, __u16 code, __s32 value) const;
	void sync() const;
};
