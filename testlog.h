#include <stdint.h>

struct TestStatus {
    uint16_t state;
    char     id[8];	// battery ID, e.g. "2023-04"
    uint16_t vstart;	// initial battery voltage
    uint16_t vbat;	// current battery voltage
    uint16_t vend;	// end-test voltage
    uint32_t time;	// test runtime, seconds
};

#define LOG_NONE     0  // empty slot, log ring end marker
#define LOG_START    1  // set up to start test
#define LOG_RUNNING  2  // test in progress
#define LOG_HALTED   3  // test stopped by user
#define LOG_FINISHED 4  // test ended (battery <= 10.5V)
#define LOG_ERASE 0xffff // unused (erased) EEPROM


class _TestLog
{
public:
    // Initialize the logger, set the first and loast slots.
    void begin();

    // Return the number of slots in the EEPROM.
    int num_slots();

    // Return the number of active entries in the EEPROM.
    int num_used();

    // Get the nth log entry.
    // 0 is the oldest entry in the log.
    // (testNum = num_used() - 1) is the newest entry.
    TestStatus& get(int testNum, TestStatus& entry);

    // Create a new entry at the end of the log.
    // Clear the following entry and update the
    // _last index to mark the new end of the ring.
    // If the ring is full, adjust _first as well.
    TestStatus& add(TestStatus& entry);

    // Update (overwrite) the last entry in the log.
    TestStatus& update(TestStatus& entry);

protected:
    int _num_slots;   // number of slots available in EEPROM
    int _first;       // first used slot
    int _last;        // last used slot + 1

    // Find the first-used and first-unused entries in the ring.
    // These are helper functions for begin() and shouldn't be
    // needed by applications.
    int _findUsed(int slot);
    int _findUnused(int slot);

    // Get the specified log entry (by slot, not test number).
    TestStatus& _get(int slot, TestStatus& entry);

    // Store the specified log entry.
    TestStatus& _put(int slot, TestStatus& entry);

    // Clear the specified log entry.
    void _clear(int slot);

    // Test if log slot is in use.
    bool _isActive(TestStatus& entry);
};

extern _TestLog TestLog;  // presumed singleton instance

