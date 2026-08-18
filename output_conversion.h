/**
 * @file output_conversion.h
 * @brief Header file for output file generation phase (.ob, .ent, .ext).
 */

#ifndef OUTPUT_CONVERSION_H
#define OUTPUT_CONVERSION_H

/**
 * @brief Main output conversion driver for creating all target files.
 *
 * @param input_filename Base filename string.
 * @return int 0 on success, 1 on error.
 */
int output_conversion(char *input_filename);

/**
 * @brief Generates the object output file (.ob) containing code and data images.
 *
 * @param filename Base filename string.
 * @return int 0 on success, 1 on file creation error.
 */
int convert_to_object_file(char *filename);

/**
 * @brief Generates the entry output file (.ent) if entry labels exist.
 *
 * @param input_filename Base filename string.
 * @return int 0 on success (or if no entry labels exist), 1 on file creation error.
 */
int convert_to_entry_file(char *input_filename);

/**
 * @brief Generates the extern output file (.ext) if external references exist.
 *
 * @param input_filename Base filename string.
 * @return int 0 on success (or if no external references exist), 1 on file creation error.
 */
int convert_to_extern_file(char *input_filename);

#endif
