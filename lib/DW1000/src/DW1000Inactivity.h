#ifndef _DW1000Inactivity_H_INCLUDED
#define _DW1000Inactivity_H_INCLUDED

#include <stdint.h>

#ifndef UWB_DEVICE_INACTIVITY_TIMEOUT_MS
#define UWB_DEVICE_INACTIVITY_TIMEOUT_MS 8000
#endif

#ifndef UWB_DEVICE_INACTIVITY_GRACE_PERIOD_MS
#define UWB_DEVICE_INACTIVITY_GRACE_PERIOD_MS 5000
#endif

inline uint32_t normalizedDw1000InactivityTimeMs(uint32_t configuredMs) {
	return configuredMs < 1000 ? 1000 : configuredMs;
}

inline uint32_t dw1000InactivityTimeMs() {
	return normalizedDw1000InactivityTimeMs(UWB_DEVICE_INACTIVITY_TIMEOUT_MS);
}

inline uint32_t dw1000InactivityGracePeriodMs() {
	return UWB_DEVICE_INACTIVITY_GRACE_PERIOD_MS;
}

#endif