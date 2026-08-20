/**
 * @file output_conversion.h
 * @brief header file for writing output files (.ob, .ent, .ext)
 */

#ifndef OUTPUT_CONVERSION_H
#define OUTPUT_CONVERSION_H

/* ---- Main Output Conversion Driver ---- */

/**
 * @brief create output files (.ob, .ent, .ext) for assembled file
 *
 * @param input_filename name of input file
 * @return int 0 if all files created ok, 1 if error
 */
int output_conversion(char *input_filename);

/* ---- Target File Generation Generators ---- */

/**
 * @brief write object file (.ob) containing code and data images
 *
 * @param filename name of file
 * @return int 0 if ok, 1 if file error
 */
int convert_to_object_file(char *filename);

/**
 * @brief write entry file (.ent) if entry labels exist
 *
 * @param input_filename name of file
 * @return int 0 if ok (or no entries), 1 if file error
 */
int convert_to_entry_file(char *input_filename);

/**
 * @brief write extern file (.ext) if external symbols used
 *
 * @param input_filename name of file
 * @return int 0 if ok (or no externs), 1 if file error
 */
int convert_to_extern_file(char *input_filename);

#endif
