/*
 * CpET 140 Final Project — POS module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Implements POS business logic: cart operations, totals (subtotal/tax/total),
 *   finalize/record sale, and reset/clear session state.
 *
 * Boundaries
 * - No rendering and no direct file/DB I/O. Payments/persistence go through
 *   narrow adapters declared in pos.h. UI calls into this module.
 *
 * Notes
 * - Single-threaded by default; callers must sync if used from workers.
 * - Validate inputs; avoid partial updates on failure.
 * - Keep data structures simple/serializable; see pos.h for API details.
 *
 * Contributors: Joshua Literal
 */

#include "pos.h"

