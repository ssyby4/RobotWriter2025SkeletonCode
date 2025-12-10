#ifndef Text_engine_h
#define Text_engine_h

/* Render all text contained in the given file using the
 * single-stroke font and G-code backend.
 * Returns 0 on success, non-zero on error.
 */

int render_text_file(const char *filepath, float height_mm);

#endif
