#include <linux/input.h>

#include <vector>

class UInput {
private:
	int fd; // Handle for uinput file ops

	int open(const char *uinput_path);
	void openAll();
	void setup(const char *dev_name, std::vector<__u16> keys);
	void create();

	void destroy();

	// TODO Add something like
	// onUInputEvent(

public:
	UInput(const char *dev_name, std::vector<__u16> keys);
	virtual ~UInput();

	void send_event(__u16 type, __u16 code, __s32 value) const;
	void sync() const;
};
