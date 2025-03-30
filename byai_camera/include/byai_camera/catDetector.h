#ifndef __CAT_DETECTOR_H_
#define __CAT_DETECTOR_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize the cat detection system.
 *
 * @param timeout_seconds number of seconds to wait until the camera will
 *                        turn off with not cat detected by the camera.
 */
void CatDetector_init(uint16_t timeout_seconds);

/**
 * Clean up the resources used in the CatDetector.
 */
void CatDetector_cleanup(void);

/**
 * True if a cat is identified by the detector.
 * False otherwise.
 */
bool CatDetector_detected(void);

#endif
