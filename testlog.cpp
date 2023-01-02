#include <stdint.h>
#include <Arduino.h>
#include <EEPROM.h>
#include "testlog.h"

_TestLog TestLog;  // presumed singleton instance

// Initialize the logger, set the first and loast slots.
void _TestLog::begin()
{
#if 0  // clear EEPROM for testing
    for (int addr = 0; addr < EEPROM.length(); addr++)
	EEPROM.update(addr, 0xFF);
#endif

#if 0  // preload EEPROM for testing
    TestStatus entry1 = { LOG_FINISHED, "TEST-01", 1320, 1050, 1050, 30*60 };
    EEPROM.put(0 * sizeof(TestStatus), entry1);

    TestStatus entry2 = { LOG_HALTED,   "TEST-02", 1220, 1180, 1050, 20*60 };
    EEPROM.put(1 * sizeof(TestStatus), entry2);

    TestStatus entry3 = { LOG_RUNNING,  "TEST-03", 1265, 1225, 1050, 5*60 };
    EEPROM.put(2 * sizeof(TestStatus), entry3);
#endif

    _num_slots = (EEPROM.length() / sizeof (TestStatus));

    TestStatus entry;
    _get(0, entry);
    if (_isActive(entry)) {
	// Slot 0 is occupied, but it may not be the first-used.
	// Search for the marker after the last-used slot.
	_last = _findUnused(1);
	// Continue searching to find the first-used slot.
	_first = _findUsed(_last);
    } else {
	// Slot 0 is unoccupied.  Search for the first-used slot.
	_first = _findUsed(1);
	// Continue searching to find the end marker.
	_last = _findUnused(_first);
    }
}

// Return the number of slots in the EEPROM.
int _TestLog::num_slots()
{
    return _num_slots;
}

// Return the number of active entries in the EEPROM.
int _TestLog::num_used()
{
    int used = _last - _first;
    if (used < 0)
	used += _num_slots;
    return used;
}

// Get the nth log entry.
TestStatus& _TestLog::get(int testNum, TestStatus& entry)
{
#if 0
    assert(testNum >= 0 && testNum < num_used());
#endif
    int slot = _first + testNum;
    if (slot >= _num_slots)
	slot -= _num_slots;
    unsigned int addr = slot * sizeof (TestStatus);
    EEPROM.get(addr, entry);
#ifdef SERIAL_DEBUG      
    Serial.print("get: slot "); Serial.print(slot); Serial.print(" state "); Serial.println(entry.state);
#endif
    return entry;
}

// Create a new entry at the end of the log.
// Clear the following entry and update the
// _last index to mark the new end of the ring.
// If the ring is full, adjust _first as well.
TestStatus& _TestLog::add(TestStatus& entry)
{
    _put(_last, entry);
    if (++_last == _num_slots)
	_last = 0;
    // write an end marker
    _clear(_last);
    // adjust _first if needed
    if (_last == _first) {
	if (++_first == _num_slots)
	    _first = 0;
    }
#ifdef SERIAL_DEBUG
    Serial.print("     first "); Serial.print(_first); Serial.print(" last "); Serial.println(_last);
#endif
    return entry;
}

// Update (overwrite) the last entry in the log.
TestStatus& _TestLog::update(TestStatus& entry)
{
    int slot = _last - 1;
    if (slot < 0)
	slot = _num_slots - 1;
    _put(slot, entry);
    return entry;
}

int _TestLog::_findUsed(int slot)
{
    // Search for the first active slot.
    while (slot < _num_slots) {
	TestStatus entry;
	_get(slot, entry);
	if (_isActive(entry)) {
	    return slot;
	}
	++slot;
    }
    // The rest of the log is empty.
    return 0;
}

int _TestLog::_findUnused(int slot)
{
    // Search for the marker after the last-used slot.
    while (slot < _num_slots) {
	TestStatus entry;
	_get(slot, entry);
	if (!_isActive(entry)) {
	    return slot;
	}
	++slot;
    }
    // "Cannot happen": the marker is missing!
    return 0;
}

// Get the nth log entry.
TestStatus& _TestLog::_get(int slot, TestStatus& entry)
{
    unsigned int addr = slot * sizeof (TestStatus);
    EEPROM.get(addr, entry);
#ifdef SERIAL_DEBUG      
    Serial.print("get: slot "); Serial.print(slot); Serial.print(" state "); Serial.println(entry.state);
#endif
    return entry;
}

// Store the specified log entry.
TestStatus& _TestLog::_put(int slot, TestStatus& entry)
{
#ifdef SERIAL_DEBUG      
    Serial.print("put: slot "); Serial.print(slot); Serial.print(" state "); Serial.println(entry.state);
#endif
    unsigned int addr = slot * sizeof (TestStatus);
    EEPROM.put(addr, entry);
    return entry;
}

// Clear a single entry as a marker for the end of the ring.
void _TestLog::_clear(int slot)
{
#ifdef SERIAL_DEBUG
    Serial.print("clear: slot "); Serial.println(slot);
#endif
    TestStatus entry;
    memset(&entry, 0, sizeof entry);
    unsigned int addr = slot * sizeof (TestStatus);
    EEPROM.put(addr, entry);
}

// Test if log slot is in use.
bool _TestLog::_isActive(TestStatus& entry)
{
    // Don't just test for LOG_NONE, since hw-erased EEPROM
    //   returns 0xFFFF rather than 0x0000.
    return (entry.state >= LOG_START && entry.state <= LOG_FINISHED);
}

