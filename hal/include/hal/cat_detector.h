#ifndef __CAT_DETECTOR_H_
#define __CAT_DETECTOR_H_

#include <stdbool.h>

/**
 * Initialize the cat detection system.
 */
void CatDetector_init(void);

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
