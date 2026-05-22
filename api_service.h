#ifndef API_SERVICE_H
#define API_SERVICE_H

#include "sensor_service.h"

// Send a reading to the backend via HTTP POST.
// Returns the HTTP status code (201 = success), or -1 on connection error.
int sendReading(Reading reading);

#endif