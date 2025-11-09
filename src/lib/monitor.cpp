/*
 * CpET 140 Final Project — Monitoring / Reporting module
 *
 * Contributors:
 *  Joshua Literal
 *  Mary Grace Jusay
 *  Matthew Anton Guina
 *
 * Purpose
 * - Collects and exposes domain-specific expense and payroll metrics:
 *   per-project expense totals, payroll/outflow summaries, invoice/payment
 *   status, and aggregated snapshots for UI consumption.
 *
 * Boundaries
 * - No rendering and no direct platform file/DB I/O. Persistence or queries
 *   must go through the db adapter (db.h) or other narrow adapters declared
 *   in monitor.h. No system-level monitoring (CPU/memory) here.
 *
 * Notes
 * - Single-threaded by default; callers must synchronize if used from workers.
 * - Validate inputs and avoid partial updates on failure. Emit alerts/logs
 *   when thresholds are breached. Keep data structures simple and serializable
 *   to ease UI consumption and testing.
 *
 * Contributors: Joshua Literal
 */

#include "monitor.h"
