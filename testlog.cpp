#include <stdint.h>
#include <Arduino.h>
#include <EEPROM.h>
#include "testlog.h"

_TestLog TestLog;  // presumed singleton instance

// Find the first and last occupied slots in the ring.
void _TestLog::begin()
{
    _num_slots = (EEPROM.length() / sizeof (LogEntry));

    LogEntry entry;
    get(0, entry);
    if (isActive(entry)) {
	// Slot 0 is occupied, but it may not be the first-used.
	// Search for the marker after the last-used slot.
	_last = findUnused(1);
  Serial.print("_last = "); Serial.println(_last);
	// Continue searching to find the first-used slot.
	_first = findUsed(_last);
  Serial.print("_first = "); Serial.println(_first);
    } else {
	// Slot 0 is unoccupied.  Search for the first-used slot.
	_first = findUsed(1);
  Serial.print("_first = "); Serial.println(_first);
	// Continue searching to find the end marker.
	_last = findUnused(_first);
  Serial.print("_last = "); Serial.println(_last);
    }
}

int _TestLog::findUsed(int slot)
{
    // Search for the first active slot.
    while (slot < _num_slots) {
	LogEntry entry;
	get(slot, entry);
	if (isActive(entry)) {
	    return slot;
	}
	++slot;
    }
    // The rest of the log is empty.
    return 0;
}

int _TestLog::findUnused(int slot)
{
    // Search for the marker after the last-used slot.
    while (slot < _num_slots) {
	LogEntry entry;
	get(slot, entry);
	if (!isActive(entry)) {
	    return slot;
	}
	++slot;
    }
    // "Cannot happen": the marker is missing!
    return 0;
}

int _TestLog::num_slots()
{
    return _num_slots;
}

int _TestLog::first()
{
    return _first;
}

int _TestLog::last()
{
    return _last;
}

// Get the specified log entry.
LogEntry& _TestLog::get(int slot, LogEntry& entry)
{
    unsigned int addr = slot * sizeof (LogEntry);
    EEPROM.get(addr, entry);
#ifdef SERIAL_DEBUG      
    Serial.print("get: slot "); Serial.print(slot); Serial.print(" state "); Serial.println(entry.state);
#endif
    return entry;
}

// Store the specified log entry.  If writing to the
// _last slot, clear the following entry and update the
// _last index to mark the new end of the ring.
LogEntry& _TestLog::put(int slot, LogEntry& entry)
{
#ifdef SERIAL_DEBUG      
    Serial.print("put: slot "); Serial.print(slot); Serial.print(" state "); Serial.println(entry.state);
#endif
    unsigned int addr = slot * sizeof (LogEntry);
    EEPROM.put(addr, entry);

    if (slot == _last) {
	if (++_last == _num_slots)
	    _last = 0;
	// write an end marker
	clear(_last);
	// if the log is full, adjust the _first index
	if (_last == _first) {
	    if (++_first == _num_slots)
		_first = 0;
	}
    }
#ifdef SERIAL_DEBUG
    Serial.print("     first "); Serial.print(_first); Serial.print(" last "); Serial.println(_last);
#endif
    return entry;
}

// Clear a single entry as a marker for the end of the ring.
void _TestLog::clear(int slot)
{
#ifdef SERIAL_DEBUG
    Serial.print("clear: slot "); Serial.println(slot);
#endif
    LogEntry entry;
    memset(&entry, 0, sizeof entry);
    unsigned int addr = slot * sizeof (LogEntry);
    EEPROM.put(addr, entry);
}

// test entry state
bool _TestLog::isActive(LogEntry& entry)
{
    // Don't just test for LOG_NONE, since hw-erased EEPROM
    //   returns 0xFFFF rather than 0x0000.
    return (entry.state >= LOG_STARTED && entry.state <= LOG_FINISHED);
}

