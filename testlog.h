#include <stdint.h>

struct LogEntry {
    uint16_t state;
    char     id[8];	// battery ID, e.g. "2023-04"
    uint16_t vstart;	// initial battery voltage
    uint16_t vend;
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
protected:
    int _num_slots;   // number of slots available in EEPROM
    int _first;       // first used slot
    int _last;        // last used slot + 1

public:
    // Initialize the logger, set the first and loast slots.
    void begin();

    // Find the first-used and first-unused entries in the ring.
    // These are helper functions for begin() and shouldn't be
    // needed by applications.
    int findUsed(int slot);
    int findUnused(int slot);

    // Return the number of slots in the EEPROM.
    int num_slots();

    // Return the first- and last-used+1 slot numbers.
    // Beware: last may be less than first if the ring
    // of entries has wrapped around the end of the EEPROM!
    int first();
    int last();

    // Get the specified log entry.
    LogEntry& get(int slot, LogEntry& entry);

    // Store the specified log entry.  If writing to the
    // "last+1" slot, clear the following entry and update the
    // "last" index to mark the new end of the ring.
    LogEntry& put(int slot, LogEntry& entry);

    // Clear the specified log entry.
    void clear(int index);

    // test entry state
    bool isActive(LogEntry& entry);
};

extern _TestLog TestLog;  // presumed singleton instance

