/*
 * CpET 140 Final Project — Inventory module
 *
 * Contributors: Joshua Literal
 *
 * Purpose
 * - Implements inventory models and helpers: lookups, stock adjustments,
 *   and simple validation, with interfaces declared in inventory.h.
 *
 * Boundaries
 * - No rendering; no direct DB access here. Use the db adapter from db.h
 *   when persistence is needed.
 *
 * Notes
 * - Single-threaded by default; validate quantities and prevent negative stock.
 */

#include "inventory.h"
#include <string>